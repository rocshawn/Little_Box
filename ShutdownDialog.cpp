#include "ShutdownDialog.h"

#include "SystemCommandService.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr int kDialogWidth = 500;
constexpr int kDialogHeight = 310;
constexpr int kMaxSeconds = 315360000; // 10 years
} // namespace

ShutdownDialog::ShutdownDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setupConnections();
}

void ShutdownDialog::setupUi() {
    setWindowTitle("定时关机");
    setFixedSize(kDialogWidth, kDialogHeight);
    setModal(true);

    auto* rootLayout = new QVBoxLayout(this);
    auto* contentCard = new QFrame(this);
    auto* mainLayout = new QVBoxLayout(contentCard);
    auto* titleLabel = new QLabel("定时关机", contentCard);
    auto* descriptionLabel = new QLabel("输入延迟秒数后，系统将在倒计时结束时执行关机。", contentCard);

    auto* inputLabel = new QLabel("延迟秒数", contentCard);
    auto* helperLabel = new QLabel("示例：输入 300 表示 5 分钟后关机。", contentCard);
    auto* inputLayout = new QVBoxLayout();
    auto* buttonLayout = new QHBoxLayout();

    secondsEdit_ = new QLineEdit(contentCard);
    secondsEdit_->setPlaceholderText("请输入正整数秒数，例如 300");
    secondsEdit_->setValidator(new QIntValidator(1, kMaxSeconds, secondsEdit_));
    secondsEdit_->setClearButtonEnabled(true);

    confirmButton_ = new QPushButton("创建计划", contentCard);
    cancelShutdownButton_ = new QPushButton("取消计划", contentCard);
    closeButton_ = new QPushButton("关闭", contentCard);

    contentCard->setObjectName("contentCard");
    titleLabel->setObjectName("titleLabel");
    descriptionLabel->setObjectName("descriptionLabel");
    inputLabel->setObjectName("inputLabel");
    helperLabel->setObjectName("helperLabel");
    confirmButton_->setObjectName("primaryButton");
    cancelShutdownButton_->setObjectName("secondaryButton");
    closeButton_->setObjectName("ghostButton");

    descriptionLabel->setWordWrap(true);
    helperLabel->setWordWrap(true);
    confirmButton_->setCursor(Qt::PointingHandCursor);
    cancelShutdownButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setCursor(Qt::PointingHandCursor);
    confirmButton_->setMinimumHeight(44);
    cancelShutdownButton_->setMinimumHeight(44);
    closeButton_->setMinimumHeight(44);
    confirmButton_->setDefault(true);

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(secondsEdit_);
    inputLayout->addWidget(helperLabel);
    inputLayout->setSpacing(8);

    buttonLayout->addWidget(confirmButton_);
    buttonLayout->addWidget(cancelShutdownButton_);
    buttonLayout->addWidget(closeButton_);
    buttonLayout->setSpacing(10);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(14);
    mainLayout->addLayout(inputLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(26, 26, 26, 26);

    rootLayout->addStretch();
    rootLayout->addWidget(contentCard);
    rootLayout->addStretch();
    rootLayout->setContentsMargins(18, 18, 18, 18);

    auto* shadowEffect = new QGraphicsDropShadowEffect(contentCard);
    shadowEffect->setBlurRadius(30);
    shadowEffect->setOffset(0, 14);
    shadowEffect->setColor(QColor(79, 70, 229, 45));
    contentCard->setGraphicsEffect(shadowEffect);

    setStyleSheet(
        "QDialog { background:#eef2ff; }"
        "QFrame#contentCard { background:white; border:1px solid #dbe4ff; border-radius:24px; }"
        "QLabel#titleLabel { color:#1e1b4b; font-size:24px; font-weight:800; }"
        "QLabel#descriptionLabel { color:#475569; font-size:14px; margin-top:8px; }"
        "QLabel#inputLabel { color:#312e81; font-size:13px; font-weight:700; margin-top:6px; }"
        "QLabel#helperLabel { color:#64748b; font-size:12px; margin-top:8px; }"
        "QLineEdit { border:1px solid #c7d2fe; border-radius:14px; padding:12px 14px; background:#f8faff; color:#0f172a; }"
        "QLineEdit:focus { border:1px solid #5b5cf0; background:white; }"
        "QPushButton { border-radius:14px; padding:10px 16px; font-size:13px; font-weight:700; }"
        "QPushButton#primaryButton { background:#5b5cf0; color:white; border:none; }"
        "QPushButton#primaryButton:hover { background:#4f46e5; }"
        "QPushButton#primaryButton:pressed { background:#4338ca; }"
        "QPushButton#secondaryButton { background:#eef2ff; color:#312e81; border:1px solid #c7d2fe; }"
        "QPushButton#secondaryButton:hover { background:#e0e7ff; }"
        "QPushButton#ghostButton { background:white; color:#475569; border:1px solid #dbe4ff; }"
        "QPushButton#ghostButton:hover { background:#f8fafc; }"
    );
}

void ShutdownDialog::setupConnections() {
    connect(confirmButton_, &QPushButton::clicked, this, &ShutdownDialog::handleConfirmClicked);
    connect(cancelShutdownButton_, &QPushButton::clicked, this, &ShutdownDialog::handleCancelShutdownClicked);
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(secondsEdit_, &QLineEdit::returnPressed, this, &ShutdownDialog::handleConfirmClicked);
}

void ShutdownDialog::handleConfirmClicked() {
    bool ok = false;
    const int seconds = secondsEdit_->text().toInt(&ok);
    if (!ok || seconds <= 0) {
        QMessageBox::warning(this, "输入无效", "请输入大于 0 的整数秒数。");
        secondsEdit_->setFocus();
        return;
    }

    const auto result = QMessageBox::question(
        this,
        "确认创建计划",
        QString("系统将在 %1 秒后关机，是否继续？").arg(seconds),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    if (!SystemCommandService::scheduleShutdown(seconds)) {
        QMessageBox::critical(this, "创建失败", "无法创建关机计划，请检查系统权限或命令执行状态。");
        return;
    }

    QMessageBox::information(this, "创建成功", "定时关机计划已创建。");
}

void ShutdownDialog::handleCancelShutdownClicked() {
    if (!SystemCommandService::cancelShutdown()) {
        QMessageBox::warning(this, "取消失败", "当前没有可取消的关机计划，或命令执行失败。");
        return;
    }

    QMessageBox::information(this, "已取消", "定时关机计划已取消。");
}
