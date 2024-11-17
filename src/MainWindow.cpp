#include "MainWindow.h"
#include "Config.h"
#include "StockData.h"
#include <thread>

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, _("Stock Data Analyzer"), wxDefaultPosition, wxSize(400, 100),
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
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    const auto& history = Config::getInstance().getStockHistory();
    // 将 std::vector<std::string> 转换为 wxArrayString
    wxArrayString stockChoices;
    for (const auto& item : history) {
        stockChoices.Add(item);
    }

    stockCombo_ = new wxComboBox(mainPanel_, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        stockChoices);

    actionButton_ = new wxButton(mainPanel_, wxID_ANY, _("Get Data"));

    sizer->Add(new wxStaticText(mainPanel_, wxID_ANY, _("Select Stock Code:")), 0, wxALL, 5);
    sizer->Add(stockCombo_, 1, wxALL, 5);
    sizer->Add(actionButton_, 0, wxALL, 5);

    mainSizer_->Add(sizer, 0, wxEXPAND | wxALL, 0);

    // 添加版权信息标签
    auto* copyrightLabel = new wxStaticText(mainPanel_, wxID_ANY, wxString::Format(_("Copyright %s 2024 ByteSharky All rights reserved."),_(wxT("\u00A9"))));
    mainSizer_->Add(copyrightLabel, 0, wxALIGN_CENTER | wxALL, 0);

    actionButton_->Bind(wxEVT_BUTTON, &MainWindow::OnGetData, this);
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

void MainWindow::OnGetData(wxCommandEvent& event) {
    std::string stockCode = stockCombo_->GetValue().ToStdString();
    if (stockCode.empty()) {
        wxMessageBox(_("Please enter a stock code"), _("Error"));
        return;
    }
    
    actionButton_->Disable();
    actionButton_->SetLabel(_("Querying..."));
    
    // 使用 std::thread 启动异步任务
    std::thread([this, stockCode]() {

        // 执行耗时任务
        StockData::queryStockData(stockCode);

        // 回到主线程更新 UI
        wxTheApp->CallAfter([this]() {
            actionButton_->Enable();
            actionButton_->SetLabel(_("Get Data"));
            UpdateStockComboBox(Config::getInstance().getStockHistory());
        });
    }).detach(); // 分离线程
}
