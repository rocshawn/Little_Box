#include "Sidebar.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {
constexpr int kSidebarWidth = 76;

struct CategoryDef {
    ModuleCategory  category{ ModuleCategory::Tools };
    QString         emoji{};
    QString         label{};
};

const QList<CategoryDef> kCategories{
    { ModuleCategory::Tools,     "\xF0\x9F\x94\xA7", "工具" },   // 🔧
    { ModuleCategory::Games,     "\xF0\x9F\x8E\xAE", "游戏" },   // 🎮
    { ModuleCategory::Challenge, "\xE2\x9A\xA1",     "挑战" },   // ⚡
};

const char* const kSidebarStyleSheet =
    "QWidget#sidebar { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #06b6d4, stop:1 #0d9488); }"

    "QPushButton#categoryButton {"
    "  background:transparent; color:#134e4a; border:none;"
    "  font-size:14px; font-weight:800;"
    "  padding:14px 4px; border-radius:18px;"
    "}"
    "QPushButton#categoryButton:hover {"
    "  background:rgba(255, 255, 255, 0.15); color:#134e4a;"
    "}"
    "QPushButton#categoryButton[active=\"true\"] {"
    "  background:rgba(255, 255, 255, 0.35); color:#000000;"
    "}"

    "QLabel#logoBadge {"
    "  background:#ffffff;"
    "  color:#0d9488; border-radius:18px; font-size:18px; font-weight:900;"
    "}"

    "QPushButton#settingsButton {"
    "  background:transparent; color:rgba(255, 255, 255, 0.85); border:none;"
    "  font-size:18px; padding:10px; border-radius:16px;"
    "}"
    "QPushButton#settingsButton:hover {"
    "  background:rgba(255, 255, 255, 0.25);"
    "}";
} // namespace

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

ModuleCategory Sidebar::currentCategory() const {
    return currentCategory_;
}

void Sidebar::setupUi() {
    setObjectName("sidebar");
    setFixedWidth(kSidebarWidth);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 16, 8, 16);
    layout->setSpacing(6);

    // Logo badge
    auto* logoBadge = new QLabel("LB", this);
    logoBadge->setObjectName("logoBadge");
    logoBadge->setAlignment(Qt::AlignCenter);
    logoBadge->setFixedSize(40, 40);
    layout->addWidget(logoBadge, 0, Qt::AlignHCenter);
    layout->addSpacing(18);

    // Category buttons
    for (const auto& def : kCategories) {
        auto* button = new QPushButton(def.emoji + "\n" + def.label, this);
        button->setObjectName("categoryButton");
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);
        button->setFixedHeight(62);

        connect(button, &QPushButton::clicked, this, [this, cat = def.category]() {
            selectCategory(cat);
        });

        entries_.append({ def.category, button });
        layout->addWidget(button);
    }

    layout->addStretch();

    // Settings button placeholder
    auto* settingsButton = new QPushButton("\xE2\x9A\x99", this);  // ⚙
    settingsButton->setObjectName("settingsButton");
    settingsButton->setCursor(Qt::PointingHandCursor);
    settingsButton->setFlat(true);
    settingsButton->setFixedSize(40, 40);
    settingsButton->setToolTip("设置");
    layout->addWidget(settingsButton, 0, Qt::AlignHCenter);

    connect(settingsButton, &QPushButton::clicked, this, &Sidebar::settingsRequested);

    setStyleSheet(kSidebarStyleSheet);

    // Set initial active state
    selectCategory(ModuleCategory::Tools);
}

void Sidebar::selectCategory(const ModuleCategory category) {
    if (currentCategory_ == category) {
        // Still update visuals in case this is the initial call
        for (auto& entry : entries_) {
            entry.button->setProperty("active", entry.category == category);
            entry.button->style()->unpolish(entry.button);
            entry.button->style()->polish(entry.button);
        }
        return;
    }

    currentCategory_ = category;

    for (auto& entry : entries_) {
        entry.button->setProperty("active", entry.category == category);
        entry.button->style()->unpolish(entry.button);
        entry.button->style()->polish(entry.button);
    }

    emit categoryChanged(category);
}
