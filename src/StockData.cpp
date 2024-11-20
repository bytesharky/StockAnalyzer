#pragma once
#include "Common.h"
#include "Config.h"
#include "StockData.h"
#include <algorithm>
#include <chrono>
#include <curl/curl.h>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <thread>
#include <tuple>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// 配置参数
const char* BASE_URL = "https://stock.gtimg.cn/data/index.php";
const int MAX_PAGE = 100;
const int SLEEP_TIME = 500; 

// 统一股票代码
std::string StockData::getStockSymbol(const std::string stockCode) {
    int pos = stockCode.find('.');
    std::string symbol = stockCode;
    if (pos > 0) {
        symbol = stockCode.substr(pos + 1);
        symbol += stockCode.substr(0, pos);
    }
    return symbol;
}

// 分析数据
wxString StockData::analyzeData(const std::vector<TickData>& data) {
    if (data.empty()) {
        wxMessageBox(_("No data available for analysis"), _("Information"), wxICON_INFORMATION);
        return "";
    }

    // 分离买入和卖出订单，忽略中性订单
    std::vector<TickData> buyOrders, sellOrders;
    for (const auto& tick : data) {
        if (tick.type == "Buy") {
            buyOrders.push_back(tick);
        }
        else if (tick.type == "Sell") {
            sellOrders.push_back(tick);
        }
    }

    // 数量分析函数
    auto analyzeVolume = [](const std::vector<TickData>& orders) {
        if (orders.empty()) return std::make_tuple(0.0, 0.0, 0.0, 0.0);

        double maxVolume = orders[0].volume;
        double minVolume = orders[0].volume;
        double sumVolume = 0.0;

        for (const auto& order : orders) {
            maxVolume = (std::max)(maxVolume, static_cast<double>(order.volume));
            minVolume = (std::min)(minVolume, static_cast<double>(order.volume));
            sumVolume += order.volume;
        }

        return std::make_tuple(sumVolume, maxVolume, minVolume, sumVolume / orders.size());
    };

    // 价格分析函数
    auto analyzePrices = [](const std::vector<TickData>& orders) {
        if (orders.empty()) return std::make_tuple(0.0, 0.0, 0.0);

        double maxPrice = orders[0].price;
        double minPrice = orders[0].price;
        double sumPrice = 0.0;

        for (const auto& order : orders) {
            maxPrice = (std::max)(maxPrice, order.price);
            minPrice = (std::min)(minPrice, order.price);
            sumPrice += order.price;
        }

        return std::make_tuple(sumPrice, maxPrice, minPrice);
    };

    // 交易金额分析函数
    auto analyzeAmount = [](const std::vector<TickData>& orders, const int sumVolume) {
        if (orders.empty()) return std::make_tuple(0.0, 0.0, 0.0, 0.0, 0.0);

        double onhand = orders[0].onehand();
        double maxAmount = orders[0].amount;
        double minAmount = orders[0].amount;
        double sumAmount = 0.0;

        for (const auto& order : orders) {
            double amount = order.amount;
            maxAmount = (std::max)(maxAmount, amount);
            minAmount = (std::min)(minAmount, amount);
            sumAmount += amount;
        }

        return std::make_tuple(sumAmount, maxAmount, minAmount, sumAmount / orders.size(), sumAmount / sumVolume / onhand);
    };

    // 分析买入和卖出订单
    auto [buySumVolume, buyMaxVolume, buyMinVolume, buyAvgVolume] = analyzeVolume(buyOrders);
    auto [buySumPrice, buyMaxPrice, buyMinPrice] = analyzePrices(buyOrders);
    auto [buySumAmount, buyMaxAmount, buyMinAmount, buyAvgAmount, buyAvgPrice] = analyzeAmount(buyOrders, buySumVolume);


    auto [sellSumVolume, sellMaxVolume, sellMinVolume, sellAvgVolume] = analyzeVolume(sellOrders);
    auto [sellSumPrice, sellMaxPrice, sellMinPrice] = analyzePrices(sellOrders);
    auto [sellSumAmount, sellMaxAmount, sellMinAmount, sellAvgAmount, sellAvgPrice] = analyzeAmount(sellOrders, sellSumVolume);

    // 获取最近的买入和卖出订单
    TickData lastBuy = buyOrders.empty() ? TickData{} : buyOrders.back();
    TickData lastSell = sellOrders.empty() ? TickData{} : sellOrders.back();

    // 生成表格
    auto makeTable = [=]() {
        // 设置输出精度为保留两位小数
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        // 人类友好
        const int language = Config::getInstance().getLanguage();
         const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(language);
        wxString localeName = languageInfo->GetLocaleName();
        wxString symbol = localeName.StartsWith("zh") ? _("E4") : _("kilo");
        int human = localeName.StartsWith("zh") ? 10000 : 1000;

        // 输出买订单分析结果
        ss << _("Buy Orders Analysis") << " (" << _("Count: ") << buyOrders.size() << ") \n";
        ss << _("| Analysis Item | Count | Max | Min | Avg |") << " \n";
        ss << _("| Prices |") << (buySumPrice / human) << symbol << " |" << buyMaxPrice << " |" << buyMinPrice << " | " << buyAvgPrice << " |\n";
        ss << _("| Volume |") << (buySumVolume / human) << symbol << " |" << buyMaxVolume << " | " << buyMinVolume << " | " << buyAvgVolume << " |\n";
        ss << _("| Amounts | ") << (buySumAmount / human) << symbol << " |" << buyMaxAmount << " | " << buyMinAmount << " | " << buyAvgAmount << " |\n";
        ss << "\n";

        // 输出卖订单分析结果
        ss << _("Sell Orders Analysis") << " (" << _("Count: ") << sellOrders.size() << ") \n";
        ss << _("| Analysis Item | Count | Max | Min | Avg |") << " \n";
        ss << _("| Prices |") << (sellSumPrice / human) << symbol << " | " << sellMaxPrice << " | " << sellMinPrice << " | " << sellAvgPrice << " |\n";
        ss << _("| Volume |") << (sellSumVolume / human) << symbol << " | " << sellMaxVolume << " | " << sellMinVolume << " | " << sellAvgVolume << " |\n";
        ss << _("| Amounts | ") << (sellSumAmount / human) << symbol << " | " << sellMaxAmount << " | " << sellMinAmount << " | " << sellAvgAmount << " |\n";
        ss << "\n";

        // 最近交易
        ss << _("Last Transaction") << " \n";
        ss << _("| Transaction | Price | Volume | Amounts |") << " \n";
        ss << _("| Buy Orders |") << lastBuy.price << " | " << static_cast<double>(lastBuy.volume) << " | " << static_cast<double>(lastBuy.amount) << " |\n";
        ss << _("| Sell Orders |") << lastSell.price << " | " << static_cast<double>(lastSell.volume) << " | " << static_cast<double>(lastSell.amount) << " |\n";

        return ss;
        };
    
    return formatTableData(makeTable());
}

// 格式化成表格
wxString StockData::formatTableData(std::stringstream ss) {
    std::vector<std::string> rows;
    std::string row;
    while (std::getline(ss, row)) {
        rows.push_back(row);
    }

    // 计算每列的最大宽度，只考虑包含分隔符的行
    std::vector<int> columnWidths;
    for (const auto& r : rows) {
        if (r.find('|') != std::string::npos) {
            // 去除行首尾的空格和|后再进行列宽计算
            std::string trimmedRow = r;
            while (trimmedRow.front() == ' ' || trimmedRow.front() == '|') {
                trimmedRow.erase(trimmedRow.begin());
            }
            while (trimmedRow.back() == ' ' || trimmedRow.back() == '|') {
                trimmedRow.erase(trimmedRow.end() - 1);
            }

            std::stringstream rowSs(trimmedRow);
            std::string cell;
            int colIndex = 0;
            while (std::getline(rowSs, cell, '|')) {
                if (columnWidths.size() <= colIndex) {
                    columnWidths.push_back(cell.length());
                }
                else {
                    columnWidths[colIndex] = (std::max)(columnWidths[colIndex], static_cast<int>(cell.length()));
                }
                colIndex++;
            }
        }
    }

    // 构建对齐后的表格字符串
    std::stringstream outputSs;
    for (const auto& r : rows) {
        if (r.find('|') != std::string::npos) {
            // 去除行首尾的空格和|后进行对齐处理
            std::string trimmedRow = r;
            while (trimmedRow.front() == ' ' || trimmedRow.front() == '|') {
                trimmedRow.erase(trimmedRow.begin());
            }
            while (trimmedRow.back() == ' ' || trimmedRow.back() == '|') {
                trimmedRow.erase(trimmedRow.end() - 1);
            }

            std::stringstream rowSs(trimmedRow);
            std::string cell;
            int colIndex = 0;
            while (std::getline(rowSs, cell, '|')) {
                outputSs << "| " << std::setw(columnWidths[colIndex]) << cell << " ";
                colIndex++;
            }
            outputSs << "|\n";
        }
        else {
            outputSs << r << "\n";
        }
    }

    // 去除末尾多余的换行符
    std::string outputStr = outputSs.str();
    if (!outputStr.empty() && outputStr.back() == '\n') {
        outputStr.pop_back();
    }

    // 再次检查列宽，确保每列至少有一个字符宽度（避免空列导致格式错乱）
    for (size_t i = 0; i < columnWidths.size(); ++i) {
        columnWidths[i] = (std::max)(1, columnWidths[i]);
    }

    // 重新构建输出字符串，根据调整后的列宽再次精确对齐
    std::stringstream finalOutputSs;
    for (const auto& r : rows) {
        if (r.find('|') != std::string::npos) {
            // 去除行首尾的空格和|后进行最终对齐处理
            std::string trimmedRow = r;
            while (trimmedRow.front() == ' ' || trimmedRow.front() == '|') {
                trimmedRow.erase(trimmedRow.begin());
            }
            while (trimmedRow.back() == ' ' || trimmedRow.back() == '|') {
                trimmedRow.erase(trimmedRow.end() - 1);
            }

            std::stringstream rowSs(trimmedRow);
            std::string cell;
            int colIndex = 0;
            while (std::getline(rowSs, cell, '|')) {
                finalOutputSs << "| " << std::setw(columnWidths[colIndex]) << cell << " ";
                colIndex++;
            }
            finalOutputSs << "|\n";
        }
        else {
            finalOutputSs << r << "\n";
        }
    }

    // 将std::string转换回wxString并返回
    return wxString(finalOutputSs.str());
}

// 用于获取页面数据的函数
std::string StockData::fetchPageData(const std::string& symbol, int page, const std::string& action) {
    MemoryBlock chunk;
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error(_("Failed to initialize CURL"));
    }

    std::string lowerSymbol = toLowerCase(symbol);

    wxString url = wxString::Format("%s?appn=detail&action=%s&c=%s&p=%d", BASE_URL, action, lowerSymbol, page);

    curl_easy_setopt(curl, CURLOPT_URL, url.ToStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);


    // 只在非200状态码时抛出异常
    if (res != CURLE_OK) {
        throw std::runtime_error(wxString::Format(_("CURL request failed: %s"), curl_easy_strerror(res)).ToStdString());
    }

    if (http_code != 200) {
        throw std::runtime_error(wxString::Format(_("HTTP request failed with status code: %d"), http_code).ToStdString());
    }

    // 200状态码时，即使数据为空也返回空字符串
    return std::string(chunk.data, chunk.size);
}

// 提取响应中引号内的内容
std::string  StockData::getResponseText(const std::string& response) {
    // 如果响应为空或不包含数据，直接返回空结果
    if (response.empty() || response.find('[') == std::string::npos) {
        throw std::exception("Extraction failed.");
    }

    size_t start = response.find('"');
    start++;

    size_t end = response.find('"', start);

    if (end == std::string::npos) {
        throw std::exception("Extraction failed.");
    }

    return response.substr(start, end - start);
}

// 从响应中解析股票数据
std::vector<TickData>  StockData::parseStockData(const std::string& response, int stimesec, int etimesec) {
    std::vector<TickData> result;
    
    // 默认为一整天
    const std::string data = getResponseText(response);
    const int stimesec_ = (stimesec == -1) ? 0 : stimesec;
    const int etimesec_ = (etimesec == -1) ? 24 * 60 * 60 : etimesec;

    std::istringstream stream(data);
    std::string item;

    while (std::getline(stream, item, '|')) {
        if (item.empty()) continue;

        //// 输出分割记录的调试信息
        //wxMessageBox(wxString::Format("Processing Record: %s", item), "Info", wxICON_INFORMATION);

        std::istringstream item_stream(item);
        std::string index_str, time, price_str, change_str, volume_str, amount_str, type_str;

        // 逐字段解析
        if (!std::getline(item_stream, index_str, '/') ||
            !std::getline(item_stream, time, '/') ||
            !std::getline(item_stream, price_str, '/') ||
            !std::getline(item_stream, change_str, '/') ||
            !std::getline(item_stream, volume_str, '/') ||
            !std::getline(item_stream, amount_str, '/') ||
            !std::getline(item_stream, type_str, '/')) {
            wxMessageBox(_("Error: Missing fields in data"), _("Error"), wxICON_ERROR);
            continue;
        }

        //// 输出解析的字段信息
        //wxMessageBox(wxString::Format(
        //    "Parsed Fields:\nIndex: %s\nTime: %s\nPrice: %s\nChange: %s\nVolume: %s\nAmount: %s\nType: %s",
        //    index_str, time, price_str, change_str, volume_str, amount_str, type_str),
        //    "Info", wxICON_INFORMATION);

        // 尝试将字段转换为所需类型
        TickData tick;
        try {
            tick.index = std::stoi(index_str);          // 转换序号为 int
            tick.time = time;                           // 时间为字符串
            tick.price = std::stod(price_str);          // 转换价格为 double
            tick.change = std::stod(change_str);        // 转换涨跌幅为 double
            tick.volume = std::stoi(volume_str);        // 转换成交量为 int
            tick.amount = std::stoi(amount_str);        // 转换成交金额为 int
            tick.type = (type_str == "S") ? "Sell" :    // 类型判断
                (type_str == "B") ? "Buy" : "Neutral";

            int time_ = timeStringToSeconds(time);


            //wxMessageBox(wxString::Format(_("%s %d %d %d"), time, time_, stimesec, etimesec), _("Error"), wxICON_ERROR);

            if (time_ >= stimesec_ && time_ <= etimesec_) {
                result.push_back(tick);  // 添加到结果列表
            }
        }
        catch (const std::exception& e) {
            // 捕获转换异常，输出错误提示
            wxMessageBox(wxString::Format(_("Error parsing data: %s"), e.what()), _("Error"), wxICON_ERROR);
            continue;
        }
    }

    return result;
}

// 获取股票交易明细
std::vector<TickData> StockData::queryStockData(const std::string& stockCode, int stimesec, int etimesec) {

    // 根据时间获取分页数据
    std::vector<int> pages = StockData::getTimePages(stockCode);
    int sindex = (stimesec >= 0) ? StockData::findIndexForTime(pages, stimesec) : -1;
    int eindex = (etimesec >= 0) ? StockData::findIndexForTime(pages, etimesec) : -1;

    // 结束时间早于开盘时间
    if (etimesec >= 0 && eindex == -1) {
        throw std::string(_("No data available for analysis"));
    }
    // 开始时间大于收盘时间
    // 这里会得到有效的页码，在后续查询接口返回无数据
    const int page_start = (sindex >= 0) ? sindex : 0;
    const int page_end = (eindex >= 0)? eindex : 100;

    std::vector<TickData> allData;
    const std::string symbol = getStockSymbol(stockCode);

    for (int page = page_start; page <= page_end; page++) {
        try {
            std::string response = fetchPageData(symbol, page);
            // 如果没有数据，跳出循环
            if (response.empty()) {
                break;
            }
            auto pageData = parseStockData(response, stimesec, etimesec);
            allData.insert(allData.end(), pageData.begin(), pageData.end());
            // 防止频繁请求
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        }
        catch (const std::exception& e) {
                wxMessageBox(wxString::Format(_("Error occurred while fetching data on page %d: %s"), page, e.what()),
                _("Error"), wxICON_ERROR);
            break;
        }
    }

    if (!allData.empty()) {
        return allData;
    }
    else {
        throw std::string(_("No data available for analysis"));
    }
}

// 用于将时间字符串转换为从当天0点开始的秒数
int  StockData::timeStringToSeconds(const std::string& timeStr) {
    int hours = 0, minutes = 0, seconds = 0;
    std::istringstream iss(timeStr);

    char delimiter;
    iss >> hours >> delimiter >> minutes >> delimiter >> seconds;

    return hours * 3600 + minutes * 60 + seconds;
}

// 用于分割原始字符串并存储时间段信息，以秒数形式存储时间
std::vector<int>  StockData::getTimePages(const std::string& stockCode) {
    std::vector<int> timePeriods;
    std::string segment;

    const std::string symbol = getStockSymbol(stockCode);
    const std::string response = fetchPageData(symbol, 0, "");
    const std::string data = getResponseText(response);
    std::string stime, etime;
    std::istringstream iss(data);
    while (std::getline(iss, segment, '|')) {
        int second;
        std::istringstream subIss(segment);
        std::getline(subIss, stime, '~');
        std::getline(subIss, etime, '~');
        timePeriods.push_back(timeStringToSeconds(stime));
    }
    timePeriods.push_back(timeStringToSeconds(etime));
    timePeriods.push_back(24 * 60 * 60);
    return timePeriods;
}

// 函数用于判断给定时间是否在某个时间段内，并返回索引
int  StockData::findIndexForTime(const std::vector<int>& timeSeconds, const std::string& givenTime) {

    int givenSeconds = timeStringToSeconds(givenTime);

    return findIndexForTime(timeSeconds, givenSeconds);

}

int  StockData::findIndexForTime(const std::vector<int>& timeSeconds, int givenSecond) {

    for (size_t i = 0; i < timeSeconds.size() - 1; ++i) {
        const int& second = timeSeconds[i];
        const int& nextSecond = timeSeconds[i+1];

        if (givenSecond >= second){
            if (givenSecond <= nextSecond) {
                return static_cast<int>(i);
            }
        }
        else {
            break;
        }
    }

    return -1;
}
