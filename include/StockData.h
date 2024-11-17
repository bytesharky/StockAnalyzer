#pragma once
#include <string>
#include <vector>
#include <optional>

struct TickData {
    int index;         // 序号
    std::string time;  // 时间
    double price;      // 价格
    double change;     // 涨跌幅
    int volume;        // 成交量
    int amount;        // 成交金额
    std::string type;  // 类型（买盘/卖盘/中性盘）
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
    static bool queryStockData(const std::string& stockCode);
private:
    static std::vector<TickData> fetchRealtimeTick(const std::string& stockCode);
    static void analyzeData(const std::vector<TickData>& data);
};