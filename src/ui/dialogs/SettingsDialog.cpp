#include "SettingsDialog.h"

#include "../../core/ThemeManager.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QVBoxLayout>

namespace {
constexpr int kDialogWidth  = 500;
constexpr int kDialogHeight = 580;
constexpr auto kAppVersion  = "1.2.0";
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
    websiteEdit_->setMinimumHeight(38);

    auto* clearBtn = new QPushButton("清除", card);
    clearBtn->setObjectName("secondaryButton");
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setFixedHeight(38);
    clearBtn->setFixedWidth(60);

    websiteRow->addWidget(websiteEdit_);
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

    lightRadio_  = new QRadioButton("日间模式", card);
    darkRadio_   = new QRadioButton("夜间模式", card);
    systemRadio_ = new QRadioButton("跟随系统", card);

    for (auto* radio : { lightRadio_, darkRadio_, systemRadio_ }) {
        radio->setObjectName("themeRadio");
        radio->setCursor(Qt::PointingHandCursor);
        cardLayout->addWidget(radio);
        cardLayout->addSpacing(4);
    }

    cardLayout->addSpacing(14);
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
    shadow->setColor(dark ? QColor(0, 0, 0, 80) : QColor(251, 113, 133, 40));
    card->setGraphicsEffect(shadow);

    rootLayout->addWidget(card);

    // ── Apply theme-aware stylesheet ──────────────────────────────────────

    setStyleSheet(buildStyleSheet(dark));
}

void SettingsDialog::setupConnections() {
    connect(lightRadio_,  &QRadioButton::clicked, this, &SettingsDialog::onLightModeSelected);
    connect(darkRadio_,   &QRadioButton::clicked, this, &SettingsDialog::onDarkModeSelected);
    connect(systemRadio_, &QRadioButton::clicked, this, &SettingsDialog::onSystemModeSelected);

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
    switch (ThemeManager::instance().mode()) {
    case ThemeMode::Light:  lightRadio_->setChecked(true);  break;
    case ThemeMode::Dark:   darkRadio_->setChecked(true);   break;
    case ThemeMode::System: systemRadio_->setChecked(true); break;
    }
}

void SettingsDialog::saveWebsite(const QString& url) {
    QSettings settings;
    if (url.isEmpty()) {
        settings.remove("website/url");
    } else {
        settings.setValue("website/url", url);
    }
}

void SettingsDialog::onLightModeSelected() {
    ThemeManager::instance().setMode(ThemeMode::Light);
}

void SettingsDialog::onDarkModeSelected() {
    ThemeManager::instance().setMode(ThemeMode::Dark);
}

void SettingsDialog::onSystemModeSelected() {
    ThemeManager::instance().setMode(ThemeMode::System);
}

void SettingsDialog::onClearWebsite() {
    websiteEdit_->clear();
    saveWebsite({});
}

QString SettingsDialog::buildStyleSheet(const bool dark) const {
    if (dark) {
        return
            "QDialog { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
            "QFrame#settingsCard { background:#3a1c36; border:2px solid #5e2c56; border-radius:28px; }"
            "QLabel#dialogTitle { color:#fff0f2; font-size:22px; font-weight:800; }"
            "QLabel#sectionTitle { color:#fbcfe8; font-size:13px; font-weight:700; }"
            "QLabel#hintLabel { color:#f472b6; font-size:12px; }"
            "QLabel#versionAppName { color:#fbcfe8; font-size:14px; font-weight:600; }"
            "QLabel#versionBadge { color:#fda4af; background:#4c0519; border:1px solid #f43f5e;"
            "  border-radius:12px; padding:3px 10px; font-size:12px; font-weight:700; }"
            "QLineEdit#websiteEdit { background:#1a0b1c; border:1px solid #5e2c56; border-radius:14px;"
            "  padding:8px 12px; color:#fff0f2; font-size:13px; }"
            "QLineEdit#websiteEdit:focus { border-color:#f43f5e; }"
            "QRadioButton#themeRadio { color:#fbcfe8; font-size:14px; }"
            "QRadioButton#themeRadio::indicator { width:18px; height:18px; }"
            "QFrame { color:#5e2c56; }"  // separators
            "QPushButton#primaryButton { background:#f43f5e; color:white; border:none;"
            "  border-radius:20px; padding:10px 16px; font-size:13px; font-weight:700; }"
            "QPushButton#primaryButton:hover { background:#e11d48; }"
            "QPushButton#secondaryButton { background:#3a1c36; color:#fbcfe8; border:2px solid #5e2c56;"
            "  border-radius:14px; padding:6px 10px; font-size:13px; font-weight:700; }"
            "QPushButton#secondaryButton:hover { background:#4c0519; border-color:#83386d; }";
    }

    return
        "QDialog { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }"
        "QFrame#settingsCard { background:white; border:2px solid #ffe4e6; border-radius:28px; }"
        "QLabel#dialogTitle { color:#4c0519; font-size:22px; font-weight:800; }"
        "QLabel#sectionTitle { color:#fb7185; font-size:13px; font-weight:700; }"
        "QLabel#hintLabel { color:#fda4af; font-size:12px; }"
        "QLabel#versionAppName { color:#4c0519; font-size:14px; font-weight:600; }"
        "QLabel#versionBadge { color:#f43f5e; background:#fff1f2; border:1px solid #fda4af;"
        "  border-radius:12px; padding:3px 10px; font-size:12px; font-weight:700; }"
        "QLineEdit#websiteEdit { background:#fffaff; border:2px solid #ffe4e6; border-radius:14px;"
        "  padding:8px 12px; color:#4c0519; font-size:13px; }"
        "QLineEdit#websiteEdit:focus { border-color:#fb7185; background:white; }"
        "QRadioButton#themeRadio { color:#4c0519; font-size:14px; }"
        "QRadioButton#themeRadio::indicator { width:18px; height:18px; }"
        "QFrame { color:#ffe4e6; }"  // separators
        "QPushButton#primaryButton { background:#fb7185; color:white; border:none;"
        "  border-radius:20px; padding:10px 16px; font-size:13px; font-weight:700; }"
        "QPushButton#primaryButton:hover { background:#f43f5e; }"
        "QPushButton#secondaryButton { background:#fff1f2; color:#f43f5e; border:2px solid #fda4af;"
        "  border-radius:14px; padding:6px 10px; font-size:13px; font-weight:700; }"
        "QPushButton#secondaryButton:hover { background:#ffe4e6; }";
}
