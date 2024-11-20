#pragma once
#include "Common.h"
#include "Config.h"
#include <algorithm>
#include <fstream>
#include <wx/stdpaths.h>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    configFile_ = getProgramDir() + "/" + configFile_;
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

const int Config::getLanguage() {
    // 尝试初始化语言环境，优先配置文件中指定的语言
    if (language_.empty()) {
        return wxLocale::GetSystemLanguage();
    }
    else {
        // 查找指定的语言信息
        const wxLanguageInfo* language = wxLocale::FindLanguageInfo(language_);

        // 找到对应的语言信息，初始化语言环境
        if (language != nullptr) {
            return language->Language;
        }
        // 如果没有找到，使用默认语言初始化语言环境
        else {
            return wxLocale::GetSystemLanguage();
        }
    }
}