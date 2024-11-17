#pragma once
#include <string>
#include <vector>
#include <optional>

struct TickData {
    int index;         // ���
    std::string time;  // ʱ��
    double price;      // �۸�
    double change;     // �ǵ���
    int volume;        // �ɽ���
    int amount;        // �ɽ����
    std::string type;  // ���ͣ�����/����/�����̣�
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