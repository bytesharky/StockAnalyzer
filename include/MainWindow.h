#pragma once
#include <wx/wx.h>
#include <wx/combobox.h>

class MainWindow : public wxFrame {
public:
    MainWindow();
	void UpdateStockComboBox(const std::vector<std::string>& newHistory);

private:
    void CreateStockPanel();
    void OnGetData();
    void OnButton(wxCommandEvent& event);
    void OnEnter(wxCommandEvent& event);

    wxPanel* mainPanel_;
    wxBoxSizer* mainSizer_;
    wxComboBox* stockCombo_;
    wxButton* actionButton_;
    wxTextCtrl* stimeBox_;
    wxTextCtrl* etimeBox_;
};