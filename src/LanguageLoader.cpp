#pragma once
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/translation.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "LanguageLoader.h"

// 重写 LoadCatalog 函数
wxMsgCatalog* LanguageLoader::LoadCatalog(const wxString& domain, const wxString& lang) {

    // 取出语言代码
    std::string lang_ = lang;
    size_t pos = lang.find(".");
    if (pos != std::string::npos) {
        lang_ = lang.substr(0, pos);
    }

    // 合成翻译文件路径
    wxString exePath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
    wxString moFilePath = exePath + wxFileName::GetPathSeparator() + "locales" + wxFileName::GetPathSeparator() +
        domain + "_" + lang_ + ".mo";

    // 检查文件是否存在
    if (!wxFileExists(moFilePath)) {
        return nullptr;
    }

    // 加载翻译文件
    wxMsgCatalog* catalog = wxMsgCatalog::CreateFromFile(moFilePath, domain);
    if (!catalog) {
        wxLogError("Failed to load translation file: %s", moFilePath);
    }
    return catalog;
}

wxArrayString LanguageLoader::GetAvailableTranslations(const wxString& domain) const {
    wxArrayString availableLangs;

    wxString path = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxFileName::GetPathSeparator() + "locales"; // 翻译文件路径

    // 遍历 locales 目录下的文件
    wxDir dir(path);
    if (!dir.IsOpened()) {
        wxLogError("Failed to open translation directory: %s", path);
        return availableLangs;
    }

    wxString filename;
    bool cont = dir.GetFirst(&filename, wxString::Format("%s_*.mo", domain), wxDIR_FILES);
    while (cont) {
        // 提取语言代码
        wxFileName file(path + filename);
        wxString lang = file.GetName().AfterLast('_');
        availableLangs.Add(lang);
        cont = dir.GetNext(&filename);
    }
    return availableLangs;
}
