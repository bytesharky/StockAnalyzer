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


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// 配置参数
const char* BASE_URL = "https://stock.gtimg.cn/data/index.php";
const int MAX_PAGE = 100;
const int SLEEP_TIME = 500; 

// 用于动态字符串处理的内存块结构
struct MemoryBlock {
    char* data;
    size_t size;

    MemoryBlock() {
        data = (char*)malloc(1);
        size = 0;
    }

    ~MemoryBlock() {
        if (data) {
            free(data);
        }
    }
};

// 用于写入接收到的数据的回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    MemoryBlock* mem = (MemoryBlock*)userp;

    char* ptr = (char*)realloc(mem->data, mem->size + total_size + 1);
    if (!ptr) {
        throw std::runtime_error(_("Failed to allocate memory"));
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->data[mem->size] = 0;

    return total_size;
}

bool StockData::queryStockData(const std::string& stockCode) {
    try {
        int pos = stockCode.find('.');
        std::string stockCode_ = stockCode;
        if (pos > 0) {
            stockCode_ = stockCode.substr(pos + 1);
            stockCode_ += stockCode.substr(0, pos);
        }
        auto data = fetchRealtimeTick(stockCode_);
        if (!data.empty()) {
            analyzeData(data);
            Config::getInstance().saveConfig(stockCode);
            return true;
        }else{
            wxMessageBox(_("No data available for analysis"), _("Information"), wxICON_INFORMATION);
            return false;
        }
    }
    catch (const std::exception& e) {
        wxMessageBox(e.what(), "Error", wxICON_ERROR);
    }
    return false;
}


void StockData::analyzeData(const std::vector<TickData>& data) {
    if (data.empty()) {
        wxMessageBox(_("No data available for analysis"), _("Information"), wxICON_INFORMATION);
        return;
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

        return std::make_tuple(maxPrice, minPrice, sumPrice / orders.size());
        };

    // 交易金额分析函数
    auto analyzeAmount = [](const std::vector<TickData>& orders) {
        if (orders.empty()) return std::make_tuple(0.0, 0.0, 0.0);

        double maxAmount = orders[0].price * orders[0].volume;
        double minAmount = orders[0].price * orders[0].volume;
        double sumAmount = 0.0;

        for (const auto& order : orders) {
            double amount = order.price * order.volume;
            maxAmount = (std::max)(maxAmount, amount);
            minAmount = (std::min)(minAmount, amount);
            sumAmount += amount;
        }

        return std::make_tuple(maxAmount, minAmount, sumAmount / orders.size());
        };

    // 分析买入和卖出订单
    auto [buyMaxPrice, buyMinPrice, buyAvgPrice] = analyzePrices(buyOrders);
    auto [buyMaxAmount, buyMinAmount, buyAvgAmount] = analyzeAmount(buyOrders);
    auto [sellMaxPrice, sellMinPrice, sellAvgPrice] = analyzePrices(sellOrders);
    auto [sellMaxAmount, sellMinAmount, sellAvgAmount] = analyzeAmount(sellOrders);

    // 获取最近的买入和卖出订单
    TickData lastBuy = buyOrders.empty() ? TickData{} : buyOrders.back();
    TickData lastSell = sellOrders.empty() ? TickData{} : sellOrders.back();

    // 使用wxString构造输出内容
    wxString output;
    output.Printf(
        _("Buy Orders Analysis (%d):\n"
        "Prices - Max: %.2f, Min: %.2f, Avg: %.2f\n"
        "Transaction Amounts - Max: %.2f, Min: %.2f, Avg: %.2f\n"
        "Most Recent - Price: %.2f, Volume: %d\n\n"
        "Sell Orders Analysis (%d):\n"
        "Prices - Max: %.2f, Min: %.2f, Avg: %.2f\n"
        "Transaction Amounts - Max: %.2f, Min: %.2f, Avg: %.2f\n"
        "Most Recent - Price: %.2f, Volume: %d"),
        buyOrders.size(), buyMaxPrice, buyMinPrice, buyAvgPrice,
        buyMaxAmount, buyMinAmount, buyAvgAmount,
        lastBuy.price, lastBuy.volume,
        sellOrders.size(), sellMaxPrice, sellMinPrice, sellAvgPrice,
        sellMaxAmount, sellMinAmount, sellAvgAmount,
        lastSell.price, lastSell.volume
    );

    wxMessageBox(output, _("Analysis Results"), wxICON_INFORMATION);
}


static std::string toLowerCase(const std::string& str) {
    std::string result = str; // 创建副本
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// 用于获取页面数据的函数
static std::string fetchPageData(const std::string& symbol, int page) {
    MemoryBlock chunk;
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error(_("Failed to initialize CURL"));
    }

    std::string lowerSymbol = toLowerCase(symbol);

    wxString url = wxString::Format("%s?appn=detail&action=data&c=%s&p=%d", BASE_URL, lowerSymbol, page);

    curl_easy_setopt(curl, CURLOPT_URL, url.ToStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
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

// 从响应中解析股票数据
static std::vector<TickData> parseStockData(const std::string& response) {
    std::vector<TickData> result;
    
    // 如果响应为空或不包含数据，直接返回空结果
    if (response.empty() || response.find('[') == std::string::npos) {
        return result;
    }

    size_t start = response.find('"');
    start++;

    size_t end = response.find('"', start);

    if (end == std::string::npos) {
        return result;
    }

    std::string data = response.substr(start, end - start);
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
            result.push_back(tick);  // 添加到结果列表
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
std::vector<TickData> StockData::fetchRealtimeTick(const std::string& stockCode) {
    std::vector<TickData> allData;
    const int page_count = 100; // 限制获取的页数

    for (int page = 1; page <= page_count; page++) {
        try {
            std::string response = fetchPageData(stockCode, page);
            auto pageData = parseStockData(response);

            // 如果没有数据，跳出循环
            if (pageData.empty()) {
                break;
            }

            allData.insert(allData.end(), pageData.begin(), pageData.end());

            // Sleep between requests
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        }
        catch (const std::exception& e) {
                wxMessageBox(wxString::Format(_("Error occurred while fetching data on page %d: %s"), page, e.what()),
                _("Error"), wxICON_ERROR);

            break;
        }
    }

    return allData;
}