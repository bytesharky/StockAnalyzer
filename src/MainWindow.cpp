#pragma once
#include "Config.h"
#include "MainWindow.h"
#include "ResultWindow.h"
#include "StockData.h"
#include <wx/regex.h>
#include <thread>

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, _("Stock Data Analyzer"), wxDefaultPosition, wxSize(400, 150),
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    // 设置主窗体图标
    wxIcon appIcon("IDI_APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE); // 从资源文件中加载图标
    SetIcon(appIcon);

    // 创建主面板和布局管理器
    mainPanel_ = new wxPanel(this);
    mainSizer_ = new wxBoxSizer(wxVERTICAL);
    mainPanel_->SetSizer(mainSizer_);

    // 调用方法创建股票面板
    CreateStockPanel();
}

void MainWindow::CreateStockPanel() {


    const auto& history = Config::getInstance().getStockHistory();
    // 将 std::vector<std::string> 转换为 wxArrayString
    wxArrayString stockChoices;
    for (const auto& item : history) {
        stockChoices.Add(item);
    }

    // 设置ComboBox样式以处理回车键
    stockCombo_ = new wxComboBox(mainPanel_, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, stockChoices);
    stockCombo_->SetWindowStyle(stockCombo_->GetWindowStyle() | wxTE_PROCESS_ENTER);
    
    auto* rSizer1 = new wxBoxSizer(wxHORIZONTAL);
    rSizer1->Add(new wxStaticText(mainPanel_, wxID_ANY, _("Select Stock Code:"), wxDefaultPosition, wxSize(90, -1)), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP | wxRIGHT, 10);
    rSizer1->Add(stockCombo_, 1, wxLEFT | wxTOP | wxRIGHT, 10);

    stimeBox_ = new wxTextCtrl(mainPanel_, wxID_ANY, "", wxDefaultPosition, wxSize(1, 25), wxBORDER_SIMPLE);
    etimeBox_ = new wxTextCtrl(mainPanel_, wxID_ANY,"", wxDefaultPosition, wxSize(1, 25), wxBORDER_SIMPLE);
    actionButton_ = new wxButton(mainPanel_, wxID_ANY, _("Get Data"));
    stimeBox_->SetMaxSize(wxSize(-1, 25));
    etimeBox_->SetMaxSize(wxSize(-1, 25));
    stimeBox_->SetWindowStyle(stimeBox_->GetWindowStyle() | wxTE_PROCESS_ENTER);
    etimeBox_->SetWindowStyle(etimeBox_->GetWindowStyle() | wxTE_PROCESS_ENTER);

    auto* rSizer2 = new wxBoxSizer(wxHORIZONTAL);
    rSizer2->Add(new wxStaticText(mainPanel_, wxID_ANY, _("Set Time Span:"), wxDefaultPosition, wxSize(90,-1)), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP | wxRIGHT, 10);
    rSizer2->Add(stimeBox_, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
    rSizer2->Add(new wxStaticText(mainPanel_, wxID_ANY, "-"), 0, wxALIGN_CENTER_VERTICAL | wxTOP, 10);
    rSizer2->Add(etimeBox_, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
    rSizer2->Add(actionButton_, 0, wxTOP | wxRIGHT, 10);

    mainSizer_->Add(rSizer1, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
    mainSizer_->Add(rSizer2, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // 添加版权信息标签
    auto* copyrightLabel = new wxStaticText(mainPanel_, wxID_ANY, wxString::Format(_("Copyright %s 2024 ByteSharky All rights reserved."),wxT("\u00A9")));
    mainSizer_->Add(copyrightLabel, 0, wxALIGN_CENTER | wxTOP, 10);

    stockCombo_->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnEnter, this);
    stimeBox_->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnEnter, this);
    etimeBox_->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnEnter, this);
    actionButton_->Bind(wxEVT_BUTTON, &MainWindow::OnButton, this);
}

void MainWindow::UpdateStockComboBox(const std::vector<std::string>& newHistory) {
    if (!stockCombo_) return; 

    stockCombo_->Clear();

    // 用新的项填充组合框
    for (const auto& item : newHistory) {
        stockCombo_->AppendString(item);
    }

    // 如果新的历史记录不为空，将第一项设置为默认选中项
    if (!newHistory.empty()) {
        stockCombo_->SetSelection(0);
    }
}

// 按键事件
void MainWindow::OnEnter(wxCommandEvent& event){
    OnGetData();
}

// 鼠标点击按钮
void MainWindow::OnButton(wxCommandEvent& event) {
    OnGetData();
}

void MainWindow::OnGetData() {

    std::string stockCode = stockCombo_->GetValue().ToStdString();
    std::string stime = stimeBox_->GetValue().ToStdString();
    std::string etime = etimeBox_->GetValue().ToStdString();
    int stimesec = -1, etimesec = -1;
    wxRegEx regex("^([0-1]?[0-9]|2[0-3])(:[0-5][0-9]){0,2}$");
    wxRegEx regexhh("^([0-1]?[0-9]|2[0-3])$");


    // 股票代码不能为空
    if (stockCode.empty()) {
        wxMessageBox(_("Please enter a stock code"), _("Error"), wxICON_ERROR);
        stockCombo_->SetFocus();
        return;
    }

    // 开始时间格式不正确，请输入格式为HH:mm:ssHH:mm或HH的时间。
    if (stime != ""){
        if(!regex.Matches(stime)) {
            wxMessageBox(_("The start time format is incorrect.\nIt should be in the format of HH:mm:ss, HH:mm or HH."), _("Error"), wxICON_ERROR);
            stimeBox_->SetFocus();
            return;
        }
        stimesec = StockData::timeStringToSeconds(stime);
    }

    // 结束时间格式不正确，请输入格式为HH:mm或HH的时间。
    if (etime != ""){
        if (!regex.Matches(etime)) {
            wxMessageBox(_("The end time format is incorrect.\nIt should be in the format of HH:mm:ss, HH:mm or HH."), _("Error"), wxICON_ERROR);
            etimeBox_->SetFocus();
            return;
        }
        etimesec = StockData::timeStringToSeconds(etime);
    }

    //开始时间不能晚于结束时间
    if (stimesec > etimesec && etimesec > -1) {
        wxMessageBox(_("Start time cannot be later than end time."), _("Error"), wxICON_ERROR);
        etimeBox_->SetFocus();
        return;
    }

    actionButton_->Disable();
    actionButton_->SetLabel(_("Querying..."));
    
    // 使用 std::thread 启动异步任务
    std::thread([=]() {

        bool succeed = false;
        std::vector<TickData> data;
        try
        {
            data = StockData::queryStockData(stockCode, stimesec, etimesec);
            Config::getInstance().saveConfig(stockCode);
            succeed = true;
        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Error", wxICON_ERROR);
        }
        catch (const std::string& s) {
            wxMessageBox(s, _("Information"), wxICON_INFORMATION);
        }

        // 回到主线程更新 UI
        wxTheApp->CallAfter([=]() {
            actionButton_->Enable();
            actionButton_->SetLabel(_("Get Data"));
            if (succeed){
                UpdateStockComboBox(Config::getInstance().getStockHistory());
                ResultWindow* rWindow = new ResultWindow(this, wxString::Format(_("Stock Code: %s"), stockCode));
                rWindow->ShowResult(stockCode, data);
            }
        });

    }).detach(); // 分离线程
}
