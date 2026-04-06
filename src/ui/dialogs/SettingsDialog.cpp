#include "SettingsDialog.h"

#include "../../core/ThemeManager.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QSettings>
#include <QVBoxLayout>

namespace {
constexpr int kDialogWidth  = 500;
constexpr int kDialogHeight = 580;
constexpr auto kAppVersion  = "1.3.0";
constexpr auto kAppName     = "Little Box 工具箱";
} // namespace


// ── Helpers ──────────────────────────────────────────────────────────────────

static QFrame* makeSeparator(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    return line;
}

static QLabel* makeSectionTitle(const QString& icon, const QString& text, QWidget* parent) {
    auto* label = new QLabel(icon + "  " + text, parent);
    label->setObjectName("sectionTitle");
    return label;
}


// ── SettingsDialog ────────────────────────────────────────────────────────────

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setupConnections();
    loadSettings();
}

void SettingsDialog::setupUi() {
    setWindowTitle("设置");
    setFixedSize(kDialogWidth, kDialogHeight);
    setModal(true);

    const bool dark = ThemeManager::instance().isDark();

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 18, 18, 18);

    auto* card = new QFrame(this);
    card->setObjectName("settingsCard");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(0);

    // ── Title ──────────────────────────────────────────────────────────────

    auto* titleLabel = new QLabel("\xE2\x9A\x99  设置", card);  // ⚙
    titleLabel->setObjectName("dialogTitle");

    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(makeSeparator(card));
    cardLayout->addSpacing(18);

    // ── Section 1: Website ────────────────────────────────────────────────

    cardLayout->addWidget(makeSectionTitle("\xF0\x9F\x8C\x90", "网站链接", card));  // 🌐
    cardLayout->addSpacing(10);

    auto* websiteRow = new QHBoxLayout();
    websiteEdit_ = new QLineEdit(card);
    websiteEdit_->setObjectName("websiteEdit");
    websiteEdit_->setPlaceholderText("https://example.com");
    websiteEdit_->setEchoMode(QLineEdit::Password);
    websiteEdit_->setMinimumHeight(44);

    toggleShowBtn_ = new QPushButton("\xF0\x9F\x91\x81", card); // 👁
    toggleShowBtn_->setObjectName("secondaryButton");
    toggleShowBtn_->setFixedSize(44, 44);
    toggleShowBtn_->setCursor(Qt::PointingHandCursor);
    toggleShowBtn_->setToolTip("显示/隐藏网址");

    auto* clearBtn = new QPushButton("清除", card);
    clearBtn->setObjectName("secondaryButton");
    clearBtn->setFixedHeight(44);
    clearBtn->setCursor(Qt::PointingHandCursor);

    websiteRow->addWidget(websiteEdit_);
    websiteRow->addSpacing(8);
    websiteRow->addWidget(toggleShowBtn_);
    websiteRow->addSpacing(8);
    websiteRow->addWidget(clearBtn);

    auto* websiteHint = new QLabel("更改后将应用于\"打开网站\"功能", card);
    websiteHint->setObjectName("hintLabel");

    cardLayout->addLayout(websiteRow);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(websiteHint);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(makeSeparator(card));
    cardLayout->addSpacing(18);

    // ── Section 2: Theme ──────────────────────────────────────────────────

    cardLayout->addWidget(makeSectionTitle("\xF0\x9F\x8E\xA8", "界面主题", card));  // 🎨
    cardLayout->addSpacing(10);

    auto* themeBox = new QHBoxLayout();
    themeBox->setSpacing(12);

    lightBtn_  = new QPushButton("日间模式", card);
    darkBtn_   = new QPushButton("夜间模式", card);
    systemBtn_ = new QPushButton("跟随系统", card);

    for (auto* btn : { lightBtn_, darkBtn_, systemBtn_ }) {
        btn->setObjectName("themeBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(48);
        themeBox->addWidget(btn);
    }
    
    cardLayout->addLayout(themeBox);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(makeSeparator(card));
    cardLayout->addSpacing(18);

    // ── Section 3: Version ────────────────────────────────────────────────

    cardLayout->addWidget(makeSectionTitle("\xE2\x84\xB9", "关于", card));  // ℹ
    cardLayout->addSpacing(10);

    auto* versionRow = new QHBoxLayout();
    auto* appNameLabel = new QLabel(kAppName, card);
    appNameLabel->setObjectName("versionAppName");
    auto* versionLabel = new QLabel(QString("版本 %1").arg(kAppVersion), card);
    versionLabel->setObjectName("versionBadge");

    versionRow->addWidget(appNameLabel);
    versionRow->addStretch();
    versionRow->addWidget(versionLabel);
    cardLayout->addLayout(versionRow);

    cardLayout->addStretch();
    cardLayout->addSpacing(18);
    cardLayout->addWidget(makeSeparator(card));
    cardLayout->addSpacing(16);

    // ── Close button ──────────────────────────────────────────────────────

    auto* closeBtn = new QPushButton("关闭", card);
    closeBtn->setObjectName("primaryButton");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setMinimumHeight(42);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(clearBtn, &QPushButton::clicked, this, &SettingsDialog::onClearWebsite);

    cardLayout->addWidget(closeBtn);

    // ── Shadow ────────────────────────────────────────────────────────────

    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 12);
    shadow->setColor(dark ? QColor(0, 0, 0, 80) : QColor(13, 148, 136, 40)); // Teal shadow
    card->setGraphicsEffect(shadow);

    rootLayout->addWidget(card);

    // ── Apply theme-aware stylesheet ──────────────────────────────────────

    setStyleSheet(buildStyleSheet(dark));
}

void SettingsDialog::setupConnections() {
    connect(lightBtn_,  &QPushButton::clicked, this, [this]{ onThemeModeSelected(ThemeMode::Light); });
    connect(darkBtn_,   &QPushButton::clicked, this, [this]{ onThemeModeSelected(ThemeMode::Dark); });
    connect(systemBtn_, &QPushButton::clicked, this, [this]{ onThemeModeSelected(ThemeMode::System); });

    connect(toggleShowBtn_, &QPushButton::clicked, this, &SettingsDialog::onToggleWebsiteVisibility);

    // Save URL whenever it changes
    connect(websiteEdit_, &QLineEdit::textEdited, this, [this](const QString& text) {
        saveWebsite(text.trimmed());
    });
}

void SettingsDialog::loadSettings() {
    // Website
    QSettings settings;
    websiteEdit_->setText(settings.value("website/url").toString());

    // Theme mode
    const auto mode = ThemeManager::instance().mode();
    lightBtn_->setChecked(mode == ThemeMode::Light);
    darkBtn_->setChecked(mode == ThemeMode::Dark);
    systemBtn_->setChecked(mode == ThemeMode::System);
}

void SettingsDialog::saveWebsite(const QString& url) {
    QSettings settings;
    if (url.isEmpty()) {
        settings.remove("website/url");
    } else {
        settings.setValue("website/url", url);
    }
}

void SettingsDialog::onThemeModeSelected(ThemeMode mode) {
    ThemeManager::instance().setMode(mode);
    loadSettings(); // update checked states
    // Instant refresh for this dialog!
    setStyleSheet(buildStyleSheet(ThemeManager::instance().isDark()));
}

void SettingsDialog::onToggleWebsiteVisibility() {
    if (websiteEdit_->echoMode() == QLineEdit::Password) {
        websiteEdit_->setEchoMode(QLineEdit::Normal);
        toggleShowBtn_->setText("\xF0\x9F\x99\x88"); // 🙈
    } else {
        websiteEdit_->setEchoMode(QLineEdit::Password);
        toggleShowBtn_->setText("\xF0\x9F\x91\x81"); // 👁
    }
}

void SettingsDialog::onClearWebsite() {
    websiteEdit_->clear();
    saveWebsite({});
}

QString SettingsDialog::buildStyleSheet(const bool dark) const {
    if (dark) {
        return
            "QDialog { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #020617, stop:1 #0f172a); }"
            "QFrame#settingsCard { background:#0f172a; border:2px solid #1e293b; border-radius:28px; }"
            "QLabel#dialogTitle { color:#f1f5f9; font-size:22px; font-weight:800; }"
            "QLabel#sectionTitle { color:#2dd4bf; font-size:13px; font-weight:700; }"
            "QLabel#hintLabel { color:#94a3b8; font-size:12px; }"
            "QLabel#versionAppName { color:#f1f5f9; font-size:14px; font-weight:600; }"
            "QLabel#versionBadge { color:#2dd4bf; background:#020617; border:1px solid #1e293b;"
            "  border-radius:12px; padding:3px 10px; font-size:12px; font-weight:700; }"
            "QLineEdit#websiteEdit { background:#020617; border:1px solid #1e293b; border-radius:14px;"
            "  padding:8px 12px; color:#f1f5f9; font-size:13px; }"
            "QLineEdit#websiteEdit:focus { border-color:#0d9488; }"
            "QPushButton#themeBtn { background:#020617; color:#94a3b8; border:2px solid #1e293b;"
            "  border-radius:14px; font-size:13px; font-weight:700; }"
            "QPushButton#themeBtn:checked { background:#0d9488; color:white; border-color:#2dd4bf; }"
            "QFrame { color:#1e293b; }"  // separators
            "QPushButton#primaryButton { background:#0d9488; color:white; border:none;"
            "  border-radius:20px; padding:10px 16px; font-size:13px; font-weight:700; }"
            "QPushButton#primaryButton:hover { background:#0f766e; }"
            "QPushButton#secondaryButton { background:#0f172a; color:#2dd4bf; border:2px solid #1e293b;"
            "  border-radius:14px; padding:6px 10px; font-size:13px; font-weight:700; }"
            "QPushButton#secondaryButton:hover { background:#1e293b; border-color:#2dd4bf; }";
    }

    return
        "QDialog { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f0fdfa, stop:1 #ecfdf5); }"
        "QFrame#settingsCard { background:white; border:2px solid #ccfbf1; border-radius:28px; }"
        "QLabel#dialogTitle { color:#115e59; font-size:22px; font-weight:800; }"
        "QLabel#sectionTitle { color:#0d9488; font-size:13px; font-weight:700; }"
        "QLabel#hintLabel { color:#5eead4; font-size:12px; }"
        "QLabel#versionAppName { color:#134e4a; font-size:14px; font-weight:600; }"
        "QLabel#versionBadge { color:#0d9488; background:#ccfbf1; border:1px solid #5eead4;"
        "  border-radius:12px; padding:3px 10px; font-size:12px; font-weight:700; }"
        "QLineEdit#websiteEdit { background:#f0fdfa; border:2px solid #ccfbf1; border-radius:14px;"
        "  padding:8px 12px; color:#115e59; font-size:13px; }"
        "QLineEdit#websiteEdit:focus { border-color:#0d9488; background:white; }"
        "QPushButton#themeBtn { background:white; color:#134e4a; border:2px solid #ccfbf1;"
        "  border-radius:14px; font-size:13px; font-weight:700; }"
        "QPushButton#themeBtn:checked { background:#0d9488; color:white; border-color:#0f766e; }"
        "QFrame { color:#ccfbf1; }"  // separators
        "QPushButton#primaryButton { background:#0d9488; color:white; border:none;"
        "  border-radius:20px; padding:10px 16px; font-size:13px; font-weight:700; }"
        "QPushButton#primaryButton:hover { background:#0f766e; }"
        "QPushButton#secondaryButton { background:#f0fdfa; color:#0d9488; border:2px solid #ccfbf1;"
        "  border-radius:14px; padding:6px 10px; font-size:13px; font-weight:700; }"
        "QPushButton#secondaryButton:hover { background:#ccfbf1; }";
}
