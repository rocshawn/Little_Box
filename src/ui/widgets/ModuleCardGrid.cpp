#include "ModuleCardGrid.h"

#include "../../core/ThemeManager.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QLabel>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
constexpr int kGridColumns = 2;
constexpr int kCardMinHeight = 110;

const char* const kCardGridStyleSheet =
    "QScrollArea { background:transparent; border:none; }"
    "QWidget#scrollContent { background:transparent; }";
} // namespace


// ── ClickableCard ──────────────────────────────────────

class ClickableCard final : public QFrame {
public:
    ClickableCard(const QString& moduleId, ModuleCardGrid* grid, bool dark, QWidget* parent = nullptr)
        : QFrame(parent), moduleId_(moduleId), grid_(grid), dark_(dark) {
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            emit grid_->moduleClicked(moduleId_);
            event->accept();
            return;
        }
        QFrame::mousePressEvent(event);
    }

    void enterEvent(QEnterEvent* event) override {
        const QString hoverBg  = dark_ ? "#4c0519" : "#fff1f2";
        const QString hoverBdr = dark_ ? "#f43f5e" : "#fda4af";
        setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:28px; }")
                      .arg(hoverBg, hoverBdr));
        QFrame::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override {
        const QString bg  = dark_ ? "#3a1c36" : "white";
        const QString bdr = dark_ ? "#5e2c56" : "#ffe4e6";
        setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:28px; }")
                      .arg(bg, bdr));
        QFrame::leaveEvent(event);
    }

private:
    QString moduleId_;
    ModuleCardGrid* grid_;
    bool dark_;
};


// ── ModuleCardGrid ─────────────────────────────────────

ModuleCardGrid::ModuleCardGrid(QWidget* parent)
    : QWidget(parent) {
    rootLayout_ = new QVBoxLayout(this);
    rootLayout_->setContentsMargins(0, 0, 0, 0);

    setStyleSheet(kCardGridStyleSheet);
}

void ModuleCardGrid::clearCards() {
    if (gridContainer_ != nullptr) {
        gridContainer_->deleteLater();
        gridContainer_ = nullptr;
    }

    // Remove any existing scroll area
    while (rootLayout_->count() > 0) {
        auto* item = rootLayout_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ModuleCardGrid::setModules(const QList<ModuleInfo>& modules) {
    clearCards();

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    gridContainer_ = new QWidget();
    gridContainer_->setObjectName("scrollContent");
    auto* grid = new QGridLayout(gridContainer_);
    grid->setSpacing(16);
    grid->setContentsMargins(4, 4, 4, 4);

    for (int i = 0; i < modules.size(); ++i) {
        const auto& mod = modules[i];
        const bool dark = ThemeManager::instance().isDark();

        const QString cardBg  = dark ? "#3a1c36" : "white";
        const QString cardBdr = dark ? "#5e2c56" : "#ffe4e6";

        auto* card = new ClickableCard(mod.id, this, dark, gridContainer_);
        card->setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:28px; }")
                            .arg(cardBg, cardBdr));

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 18, 20, 18);
        cardLayout->setSpacing(8);

        // Emoji badge (colorful candy theme based on index modulo)
        const QStringList lightCandyBgs = { "#ffe4e6", "#fef08a", "#d9f99d", "#bfdbfe", "#e9d5ff" };
        const QStringList lightCandyTxs = { "#e11d48", "#854d0e", "#3f6212", "#1e3a8a", "#581c87" };
        const QStringList darkCandyBgs  = { "#4c0519", "#422006", "#14532d", "#172554", "#3b0764" };
        const QStringList darkCandyTxs  = { "#fda4af", "#fde047", "#86efac", "#93c5fd", "#d8b4fe" };
        int colorIdx = i % 5;

        auto* emojiLabel = new QLabel(mod.emoji, card);
        emojiLabel->setAlignment(Qt::AlignCenter);
        emojiLabel->setFixedSize(48, 48); // slightly larger badge for cute look
        emojiLabel->setStyleSheet(QString("background:%1; color:%2; border-radius:24px; font-size:24px;")
            .arg(dark ? darkCandyBgs[colorIdx] : lightCandyBgs[colorIdx])
            .arg(dark ? darkCandyTxs[colorIdx] : lightCandyTxs[colorIdx]));

        // Name
        auto* nameLabel = new QLabel(mod.name, card);
        nameLabel->setStyleSheet(dark
            ? "color:#fff0f2; font-size:16px; font-weight:800;"
            : "color:#4c0519; font-size:16px; font-weight:800;");

        // Description
        auto* descLabel = new QLabel(mod.description, card);
        descLabel->setStyleSheet(dark
            ? "color:#fbcfe8; font-size:13px;"
            : "color:#fb7185; font-size:13px;");
        descLabel->setWordWrap(true);

        cardLayout->addWidget(emojiLabel);
        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();

        card->setMinimumHeight(kCardMinHeight);

        // Subtle shadow
        auto* shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(24);
        shadow->setOffset(0, 6);
        shadow->setColor(QColor(99, 102, 241, 28));
        card->setGraphicsEffect(shadow);

        grid->addWidget(card, i / kGridColumns, i % kGridColumns);
    }

    // Fill remaining cells in last row so cards don't stretch
    const int remainder = modules.size() % kGridColumns;
    if (remainder != 0) {
        for (int col = remainder; col < kGridColumns; ++col) {
            auto* spacer = new QWidget(gridContainer_);
            spacer->setMinimumHeight(kCardMinHeight);
            grid->addWidget(spacer, (modules.size() - 1) / kGridColumns, col);
        }
    }

    grid->setRowStretch(grid->rowCount(), 1);

    scrollArea->setWidget(gridContainer_);
    rootLayout_->addWidget(scrollArea);
}
