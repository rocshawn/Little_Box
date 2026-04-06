#include "MainWindow.h"

#include "../../core/ModuleRegistry.h"
#include "../../core/ThemeManager.h"
#include "../dialogs/SettingsDialog.h"
#include "../widgets/ModuleCardGrid.h"
#include "../widgets/Sidebar.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kMinimumWindowWidth  = 820;
constexpr int kMinimumWindowHeight = 560;
constexpr int kInitialWindowWidth  = 1020;
constexpr int kInitialWindowHeight = 700;
constexpr auto kVersionText = "版本 1.3.0";

QString categoryDescription(const ModuleCategory category) {
    switch (category) {
    case ModuleCategory::Tools:     return "系统实用功能，帮你高效完成日常任务";
    case ModuleCategory::Games:     return "轻松休闲的小游戏，随时随地玩一把";
    case ModuleCategory::Challenge: return "测量和锻炼你的能力，不断突破自我";
    }
    return {};
}

const char* const kLightContentStyleSheet =
    "QWidget#contentArea {"
    "  background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f0fdfa, stop:1 #ecfdf5);"
    "}"
    "QFrame#headerCard { background:white; border:2px solid #ccfbf1; border-radius:28px; }"
    "QLabel#headerLogoBadge {"
    "  background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #06b6d4, stop:1 #0d9488);"
    "  color:white; border-radius:26px; font-size:20px; font-weight:800;"
    "}"
    "QLabel#headerTitle  { color:#115e59; font-size:26px; font-weight:800; }"
    "QLabel#headerSubtitle { color:#0d9488; font-size:14px; font-weight:600; }"
    "QPushButton#ghostButton {"
    "  background:#ffffff; color:#0d9488; border:2px solid #ccfbf1;"
    "  border-radius:18px; padding:8px 16px; font-size:13px; font-weight:700;"
    "}"
    "QPushButton#ghostButton:hover { background:#f0fdfa; border-color:#5eead4; }";

const char* const kDarkContentStyleSheet =
    "QWidget#contentArea {"
    "  background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #020617, stop:1 #0f172a);"
    "}"
    "QFrame#headerCard { background:#0f172a; border:2px solid #1e293b; border-radius:28px; }"
    "QLabel#headerLogoBadge {"
    "  background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #06b6d4, stop:1 #0d9488);"
    "  color:white; border-radius:26px; font-size:20px; font-weight:800;"
    "}"
    "QLabel#headerTitle  { color:#f1f5f9; font-size:26px; font-weight:800; }"
    "QLabel#headerSubtitle { color:#94a3b8; font-size:14px; font-weight:600; }"
    "QPushButton#ghostButton {"
    "  background:#0f172a; color:#2dd4bf; border:2px solid #1e293b;"
    "  border-radius:18px; padding:8px 16px; font-size:13px; font-weight:700;"
    "}"
    "QPushButton#ghostButton:hover { background:#1e293b; border-color:#2dd4bf; }";

const char* const kLightStatusBarStyleSheet =
    "QStatusBar { background:white; color:#475569; border-top:1px solid #e2e8f0; }";

const char* const kDarkStatusBarStyleSheet =
    "QStatusBar { background:#0f172a; color:#64748b; border-top:1px solid #1e293b; }";
} // namespace


// ── Constructor ───────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    ModuleRegistry::instance().registerBuiltinModules();

    setupUi();
    setupConnections();
    setupStatusBar();
    applyThemeStylesheet();
    refreshModuleGrid(ModuleCategory::Tools);
    updateWindowModeUi();
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void MainWindow::setupUi() {
    setWindowTitle("Little Box 工具箱");
    setMinimumSize(kMinimumWindowWidth, kMinimumWindowHeight);
    resize(kInitialWindowWidth, kInitialWindowHeight);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Sidebar ──────────────────────────────────────────────────────────

    sidebar_ = new Sidebar(centralWidget);

    // ── Content area ─────────────────────────────────────────────────────

    contentArea_ = new QWidget(centralWidget);
    contentArea_->setObjectName("contentArea");
    auto* contentLayout = new QVBoxLayout(contentArea_);
    contentLayout->setContentsMargins(32, 28, 32, 12);
    contentLayout->setSpacing(0);

    // Header card
    auto* headerCard = new QFrame(contentArea_);
    headerCard->setObjectName("headerCard");
    auto* headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setContentsMargins(28, 24, 28, 24);
    headerLayout->setSpacing(6);

    auto* topRow = new QHBoxLayout();

    auto* logoBadge = new QLabel("LB", headerCard);
    logoBadge->setObjectName("headerLogoBadge");
    logoBadge->setAlignment(Qt::AlignCenter);
    logoBadge->setFixedSize(52, 52);

    auto* titleBlock = new QVBoxLayout();

    headerTitleLabel_ = new QLabel("Little Box 工具箱", headerCard);
    headerTitleLabel_->setObjectName("headerTitle");

    headerSubtitleLabel_ = new QLabel(categoryDescription(ModuleCategory::Tools), headerCard);
    headerSubtitleLabel_->setObjectName("headerSubtitle");
    headerSubtitleLabel_->setWordWrap(true);

    titleBlock->addWidget(headerTitleLabel_);
    titleBlock->addWidget(headerSubtitleLabel_);
    titleBlock->setSpacing(4);

    auto* fullscreenButton = new QPushButton("进入全屏", headerCard);
    fullscreenButton->setObjectName("ghostButton");
    fullscreenButton->setCursor(Qt::PointingHandCursor);
    fullscreenButton->setMinimumHeight(38);
    fullscreenButton->setMinimumWidth(100);

    topRow->addWidget(logoBadge, 0, Qt::AlignTop);
    topRow->addSpacing(14);
    topRow->addLayout(titleBlock, 1);
    topRow->addWidget(fullscreenButton, 0, Qt::AlignTop);

    headerLayout->addLayout(topRow);

    auto* headerShadow = new QGraphicsDropShadowEffect(headerCard);
    headerShadow->setBlurRadius(40);
    headerShadow->setOffset(0, 12);
    headerShadow->setColor(QColor(251, 113, 133, 40));
    headerCard->setGraphicsEffect(headerShadow);

    // Module card grid
    cardGrid_ = new ModuleCardGrid(contentArea_);

    contentLayout->addWidget(headerCard);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(cardGrid_, 1);

    // ── Assemble ─────────────────────────────────────────────────────────

    rootLayout->addWidget(sidebar_);
    rootLayout->addWidget(contentArea_, 1);

    setCentralWidget(centralWidget);

    connect(fullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);
}

void MainWindow::setupConnections() {
    connect(sidebar_, &Sidebar::categoryChanged,   this, &MainWindow::onCategoryChanged);
    connect(sidebar_, &Sidebar::settingsRequested, this, &MainWindow::onSettingsRequested);
    connect(cardGrid_, &ModuleCardGrid::moduleClicked, this, &MainWindow::onModuleClicked);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::onThemeChanged);

    auto* fullscreenShortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fullscreenShortcut, &QShortcut::activated, this, &MainWindow::toggleFullscreen);
}

void MainWindow::setupStatusBar() {
    auto* versionLabel = new QLabel(QString::fromUtf8(kVersionText), this);
    versionLabel->setObjectName("versionLabel");
    versionLabel->setStyleSheet("color:#fb7185; font-weight:700; padding-right:8px;");

    statusBar()->addPermanentWidget(versionLabel);
}

// ── Theme ──────────────────────────────────────────────────────────────────────

void MainWindow::applyThemeStylesheet() {
    const bool dark = ThemeManager::instance().isDark();
    contentArea_->setStyleSheet(dark ? kDarkContentStyleSheet : kLightContentStyleSheet);
    statusBar()->setStyleSheet(dark ? kDarkStatusBarStyleSheet : kLightStatusBarStyleSheet);
}

void MainWindow::onThemeChanged(const bool dark) {
    Q_UNUSED(dark);
    applyThemeStylesheet();
    // Rebuild cards so inline label colors update
    refreshModuleGrid(currentCategory_);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onSettingsRequested() {
    SettingsDialog dialog(this);
    dialog.exec();
    // Rebuild grid in case website URL changed (button label may differ elsewhere)
}

void MainWindow::onCategoryChanged(const ModuleCategory category) {
    refreshModuleGrid(category);
}

void MainWindow::refreshModuleGrid(const ModuleCategory category) {
    currentCategory_ = category;
    headerSubtitleLabel_->setText(categoryDescription(category));

    const auto modules = ModuleRegistry::instance().modulesForCategory(category);
    cardGrid_->setModules(modules);
}

void MainWindow::onModuleClicked(const QString& id) {
    ModuleRegistry::instance().launch(id, this);
}

void MainWindow::toggleFullscreen() {
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
    updateWindowModeUi();
}

void MainWindow::updateWindowModeUi() {
    const auto buttons = findChildren<QPushButton*>("ghostButton");
    for (auto* btn : buttons) {
        btn->setText(isFullScreen() ? "退出全屏" : "进入全屏");
    }

    statusBar()->showMessage(
        isFullScreen()
            ? "状态：全屏模式（按 F11 退出）"
            : "状态：窗口模式（支持自由缩放，按 F11 全屏）"
    );
}
