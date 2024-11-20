#ifndef RESULTFRAME_H
#define RESULTFRAME_H

#include <wx/wx.h>
#include <StockData.h>

class ResultWindow : public wxDialog {
private:
    void analyzeData(const std::string& stockCode, const std::vector<TickData>& data);

public:
    ResultWindow(wxWindow* parent, const wxString& title);
    int ResultWindow::ShowModal() override;
    void ShowResult(const std::string& stockCode, const std::vector<TickData>& data);
    int ShowModalResult(const std::string& stockCode, const std::vector<TickData>& data);
};

#endif // RESULTFRAME_H