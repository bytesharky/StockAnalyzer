#pragma once
#include <string>
#include <vector>
#include <optional>
#include <wx/string.h>
#include <wx/window.h>

struct TickData {
    int index;              // 序号
    std::string time;       // 时间
    double price;           // 价格
    double change;          // 涨跌幅
    double volume;          // 成交量
    double amount;          // 成交金额
    std::string type;       // 类型（买盘/卖盘/中性盘）
    int onehand() const {   // 计算一手多少股
        return static_cast<int>(amount / volume / price + 0.5);
    }
};

struct StockAnalysis {
    std::optional<double> minPrice;
    std::optional<double> maxPrice;
    std::optional<double> avgPrice;
    std::optional<int> minVolume;
    std::optional<int> maxVolume;
    std::optional<double> avgVolume;
    int count;
};

class StockData {
public:
    static std::vector<int> getTimePages(const std::string& inputStr);
    static int timeStringToSeconds(const std::string& timeStr);
    static int findIndexForTime(const std::vector<int>& timePeriods, int givenSecond);
    static int findIndexForTime(const std::vector<int>& timePeriods, const std::string& givenTime);
    static std::vector<TickData> queryStockData(const std::string& stockCode, int stimesec = -1, int etimesec = -1);
    static wxString analyzeData(const std::vector<TickData>& data);
private:
    static std::string getResponseText(const std::string& response);
    static std::string getStockSymbol(const std::string stockCode);
    static std::string fetchPageData(const std::string& symbol, int page, const std::string& action = "data");
    static std::vector<TickData>  StockData::parseStockData(const std::string& response, int stimesec = -1, int etimesec = -1);
    static wxString formatTableData(std::stringstream ss);
};