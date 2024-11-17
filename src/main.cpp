#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "MainWindow.h"
#include "Config.h"
#include <locale.h>
#include <windows.h>
#include "LanguageLoader.h"


class StockAnalyzerApp : public wxApp {

private:
    wxLocale locale;

public:
    bool OnInit() override {
        Config::getInstance().loadConfig();
        const std::string language = Config::getInstance().getLanguage();

        // 开启调试日志
        wxLog::AddTraceMask("i18n");

        // 设置自定义加载器
        auto* translations = new wxTranslations();
        wxTranslations::Set(translations);

        LanguageLoader* loader = new LanguageLoader();
        translations->SetLoader(loader);

        // 尝试初始化语言环境，优先配置文件中指定的语言，
        if (language.empty()) {
            locale.Init(wxLocale::GetSystemLanguage());
        } else {
            // 查找指定的语言信息
            const wxLanguageInfo* info = wxLocale::FindLanguageInfo(language);

            if (info != nullptr) {
                // 找到对应的语言信息，初始化语言环境
                locale.Init(info->Language);
            }
            else {
                // 如果没有找到，使用默认语言初始化语言环境
                wxLogError("Language info not found for %s", language);
                locale.Init(wxLocale::GetSystemLanguage());
            }
        }

        // 加载翻译文件
        if (!locale.AddCatalog("stock"))
        {
            wxLogError("Failed to load translations!");
        }

        wxMessageBox(
            _("Announcement\n\n"
                "The data is sourced from the network.\n"
                "The analysis results are only for reference.\n"
                "Regarding the accuracy and quality issues of the data,\n"
                "this tool shall not be held responsible in any way."),
            _("Announcement")
        );

        MainWindow* frame = new MainWindow();
        frame->Show(true);
        return true;
    }

};



int main(int argc, char** argv) {
    wxDISABLE_DEBUG_SUPPORT();

    wxEntryStart(argc, argv);
    wxApp::SetInstance(new StockAnalyzerApp());
    int exitCode = wxEntry(argc, argv);
    wxEntryCleanup();
    return exitCode;
}

wxIMPLEMENT_APP_NO_MAIN(StockAnalyzerApp);