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
        const QString hoverBg  = dark_ ? "#0f766e" : "#f0fdfa";
        const QString hoverBdr = dark_ ? "#2dd4bf" : "#99f6e4";
        setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:32px; }")
                      .arg(hoverBg, hoverBdr));
        QFrame::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override {
        const QString bg  = dark_ ? "#134e4a" : "white";
        const QString bdr = dark_ ? "transparent" : "transparent"; // borderless by default
        setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:32px; }")
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

        const QString cardBg  = dark ? "#134e4a" : "white";
        const QString cardBdr = "transparent"; // modern borderless look

        auto* card = new ClickableCard(mod.id, this, dark, gridContainer_);
        card->setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:32px; }")
                            .arg(cardBg, cardBdr));

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 18, 20, 18);
        cardLayout->setSpacing(8);

        // Emoji badge (colorful candy theme based on index modulo)
        const QStringList lightCandyBgs = { "#ccfbf1", "#ecfeff", "#f0fdfa", "#e0f2fe", "#f1f5f9" };
        const QStringList lightCandyTxs = { "#0d9488", "#0891b2", "#0f766e", "#0369a1", "#475569" };
        const QStringList darkCandyBgs  = { "#115e59", "#155e75", "#064e3b", "#0c4a6e", "#1e293b" };
        const QStringList darkCandyTxs  = { "#5eead4", "#67e8f9", "#6ee7b7", "#7dd3fc", "#94a3b8" };
        int colorIdx = i % 5;

        auto* emojiLabel = new QLabel(mod.emoji, card);
        emojiLabel->setAlignment(Qt::AlignCenter);
        emojiLabel->setFixedSize(48, 48); // slightly larger badge for cute look
        emojiLabel->setStyleSheet(QString("background:%1; color:%2; border-radius:24px; font-size:24px;")
            .arg(dark ? darkCandyBgs[colorIdx] : lightCandyBgs[colorIdx])
            .arg(dark ? darkCandyTxs[colorIdx] : lightCandyTxs[colorIdx]));

        // Name
        auto* nameLabel = new QLabel(mod.name, card);
        nameLabel->setObjectName("moduleName");
        
        // Description
        auto* descLabel = new QLabel(mod.description, card);
        descLabel->setObjectName("moduleDesc");
        descLabel->setWordWrap(true);

        card->setStyleSheet(card->styleSheet() + (dark 
            ? "QLabel#moduleName { color:#2dd4bf; font-size:18px; font-weight:800; }"
              "QLabel#moduleDesc { color:#94a3b8; font-size:13px; }"
            : "QLabel#moduleName { color:#115e59; font-size:18px; font-weight:800; }"
              "QLabel#moduleDesc { color:#475569; font-size:13px; }"));

        cardLayout->addWidget(emojiLabel);
        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();

        card->setMinimumHeight(kCardMinHeight);

        // Elegant soft shadow
        auto* shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(42);
        shadow->setOffset(0, 10);
        shadow->setColor(dark ? QColor(0, 0, 0, 100) : QColor(13, 148, 136, 32)); // Teal shadow
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
