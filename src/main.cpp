#pragma once
#include "Common.h"
#include "Config.h"
#include "LanguageLoader.h"
#include "MainWindow.h"
#include <locale.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class StockAnalyzerApp : public wxApp {
private:
    wxLocale locale;
public:
    bool OnInit() override {
        Config::getInstance().loadConfig();
        const int language = Config::getInstance().getLanguage();

        // 开启调试日志
        wxLog::AddTraceMask("i18n");

        // 设置自定义加载器
        auto* translations = new wxTranslations();
        wxTranslations::Set(translations);

        LanguageLoader* loader = new LanguageLoader();
        translations->SetLoader(loader);

        // 设置语言
        locale.Init(language);

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