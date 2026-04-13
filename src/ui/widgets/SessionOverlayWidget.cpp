#include "SessionOverlayWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QEvent>

SessionOverlayWidget::SessionOverlayWidget(QWidget* parent)
    : QWidget(parent) {
    
    // Fill the parent by default
    if (parent) {
        resize(parent->size());
        parent->installEventFilter(this);
    }

    setupUi();
}

void SessionOverlayWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    titleLabel_ = new QLabel("Game Paused", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "color: white;"
        "font-size: 32px;"
        "font-weight: bold;"
        "margin-bottom: 20px;"
    );

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    continueBtn_ = new QPushButton("Continue", this);
    restartBtn_ = new QPushButton("Restart", this);

    QString btnStyle = 
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  padding: 10px 20px;"
        "  border-radius: 8px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #388E3C;"
        "}";

    QString restartBtnStyle = btnStyle;
    restartBtnStyle.replace("#4CAF50", "#F44336")
                   .replace("#45a049", "#E53935")
                   .replace("#388E3C", "#D32F2F");

    continueBtn_->setStyleSheet(btnStyle);
    restartBtn_->setStyleSheet(restartBtnStyle);
    
    continueBtn_->setCursor(Qt::PointingHandCursor);
    restartBtn_->setCursor(Qt::PointingHandCursor);

    connect(continueBtn_, &QPushButton::clicked, this, &SessionOverlayWidget::continueRequested);
    connect(restartBtn_, &QPushButton::clicked, this, &SessionOverlayWidget::restartRequested);

    btnLayout->addWidget(continueBtn_);
    btnLayout->addWidget(restartBtn_);

    mainLayout->addWidget(titleLabel_);
    mainLayout->addLayout(btnLayout);
}

void SessionOverlayWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    // Draw a semi-transparent dark background
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
}

bool SessionOverlayWidget::eventFilter(QObject* obj, QEvent* event) {
    // Keep overlay matched to parent size
    if (obj == parent() && event->type() == QEvent::Resize) {
        resize(parentWidget()->size());
    }
    return QWidget::eventFilter(obj, event);
}
