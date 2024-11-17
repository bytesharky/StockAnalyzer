#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Config {
public:
    static Config& getInstance();
    bool saveConfig(const std::string& stockCode = "");
    bool loadConfig();
	const std::string& getLanguage() const {return language_;};
    const std::vector<std::string>& getStockHistory() const { return stockHistory_; }

private:
    Config();
	std::string getProgramDir();
    std::string configFile_ = "stock_history.json";
	std::string language_ = "";
    std::vector<std::string> stockHistory_;
};
