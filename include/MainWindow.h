#pragma once
#include <wx/wx.h>
#include <wx/combobox.h>

class MainWindow : public wxFrame {
public:
    MainWindow();
	void UpdateStockComboBox(const std::vector<std::string>& newHistory);

private:
    void CreateStockPanel();
    void OnGetData(wxCommandEvent& event);

    wxPanel* mainPanel_;
    wxComboBox* stockCombo_;
    wxButton* actionButton_;
    wxBoxSizer* mainSizer_;
};