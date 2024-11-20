#pragma once
#include "ResultWindow.h"
#include <StockData.h>

ResultWindow::ResultWindow(wxWindow* parent, const wxString& title)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    wxIcon appIcon("IDI_APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
    SetIcon(appIcon);
}

void ResultWindow::analyzeData(const std::string& stockCode, const std::vector<TickData>& data) {
    const wxString analyze = StockData::analyzeData(data);
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(mainSizer);

    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    wxStaticText* staticCode = new wxStaticText(panel, wxID_ANY, wxString::Format(_("Stock Code: %s"), stockCode));
    wxStaticText* staticStatus = new wxStaticText(panel, wxID_ANY, "");

    sizer->Add(staticCode, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);
    sizer->Add(staticStatus, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);
    mainSizer->Add(sizer, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

    wxStaticText* staticData = new wxStaticText(panel, wxID_ANY, analyze);
    staticData->SetFont(monoFont);
    mainSizer->Add(staticData, 1, wxEXPAND | wxALL, 20);


    // 自适应大小
    panel->Fit();
    Fit();
}

void ResultWindow::ShowResult(const std::string& stockCode, const std::vector<TickData>& data) {
    analyzeData(stockCode, data);
    MessageBeep(MB_OK);
    Show();
}

int ResultWindow::ShowModalResult(const std::string& stockCode, const std::vector<TickData>& data) {
    analyzeData(stockCode, data);
    MessageBeep(MB_OK);
    return ShowModal();
}

// 重写 ShowModal，临时移除最小化按钮
int ResultWindow::ShowModal() {
    long style = GetWindowStyle();
    SetWindowStyle(style & ~wxMINIMIZE_BOX);
    int result = wxDialog::ShowModal();
    SetWindowStyle(style);
    return result;
}