#ifndef MY_TRANSLATIONS_LOADER_H
#define MY_TRANSLATIONS_LOADER_H

#include <wx/translation.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/filename.h>
#include <vector>


// 自定义的翻译加载器类
class LanguageLoader : public wxTranslationsLoader {
public:
    wxMsgCatalog* LoadCatalog(const wxString& domain, const wxString& lang) override;
    wxArrayString GetAvailableTranslations(const wxString& domain) const override;
};

#endif // MY_TRANSLATIONS_LOADER_H
