#include "Config.h"
#include <fstream>
#include <algorithm>
#include <wx/stdpaths.h>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    configFile_ = getProgramDir() + "/" + configFile_;
}

static std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// 获取程序所在目录路径的静态函数实现
std::string Config::getProgramDir() {
    wxStandardPaths& stdPaths = wxStandardPaths::Get();
    wxString exePath = stdPaths.GetExecutablePath();
    return std::string(wxPathOnly(exePath).mb_str());
}

bool Config::saveConfig(const std::string& stockCode) {

    if (!stockCode.empty()) {
        // 将股票代码转换为小写形式，以便进行不区分大小写的比较
        std::string lowerStockCode = toLowerCase(stockCode);

        // 在stockHistory_中查找元素，这里通过将历史记录中的代码也转换为小写形式来进行比较
        auto it = std::find_if(stockHistory_.begin(), stockHistory_.end(), [&lowerStockCode](const std::string& historyCode) {
            return toLowerCase(historyCode) == lowerStockCode;
            });

        if (it != stockHistory_.end()) {
            stockHistory_.erase(it);
        }

        // 将当前股票代码插入到stockHistory_的开头位置
        stockHistory_.insert(stockHistory_.begin(), stockCode);
    }

    nlohmann::json j;
    j["language"] = language_;
    j["stock_history"] = stockHistory_;

    std::ofstream file(configFile_);
    if (!file.is_open()) return false;
    file << j.dump(4);
    return true;
}

bool Config::loadConfig() {
    std::ifstream file(configFile_);
    if (!file.is_open()) return false;

    try {
        nlohmann::json j;
        file >> j;
        stockHistory_ = j["stock_history"].get<std::vector<std::string>>();
        language_ = toLowerCase(j["language"].get<std::string>());
    } catch (...) {
        return false;
    }
    return true;
}
