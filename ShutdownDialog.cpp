#include "ShutdownDialog.h"

#include "SystemCommandService.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QTimer>
#include <QDateTime>
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

    auto* inputLabel = new QLabel("到达时间（年-月-日 时:分:秒）", contentCard);
    auto* helperLabel = new QLabel("请选择目标日期和时间，到达后会弹出取消关机对话框；30 秒内未取消将执行关机。", contentCard);
    auto* inputLayout = new QVBoxLayout();
    auto* buttonLayout = new QHBoxLayout();

    // Provide both a datetime editor and keep secondsEdit_ for backward compatibility (hidden)
    dateTimeEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(60), contentCard);
    dateTimeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    dateTimeEdit_->setCalendarPopup(true);
    secondsEdit_ = new QLineEdit(contentCard);
    secondsEdit_->setVisible(false);

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
    inputLayout->addWidget(dateTimeEdit_);
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
    connect(dateTimeEdit_, &QDateTimeEdit::editingFinished, this, &ShutdownDialog::handleConfirmClicked);
}

void ShutdownDialog::handleConfirmClicked() {
    const QDateTime target = dateTimeEdit_->dateTime();
    const QDateTime now = QDateTime::currentDateTime();
    if (target <= now) {
        QMessageBox::warning(
            this,
            QStringLiteral("输入无效"),
            QStringLiteral("请选择一个将来的日期时间。"),
            QMessageBox::Ok
        );
        dateTimeEdit_->setFocus();
        return;
    }

    const qint64 secs = now.secsTo(target);

    const auto result = QMessageBox::question(
        this,
        "确认创建计划",
        QString("系统将在 %1 秒后弹出确认对话框，30 秒内未取消将关机。是否继续？").arg(secs),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    // Start an internal timer to watch for the scheduled time
    if (watchTimer_ == nullptr) {
        watchTimer_ = new QTimer(this);
        connect(watchTimer_, &QTimer::timeout, this, &ShutdownDialog::handleWatchTimer);
    }

    scheduledTime_ = target;
    watchTimer_->start(1000); // check every second

    QMessageBox::information(
        this,
        QStringLiteral("创建成功"),
        QStringLiteral("定时关机计划已创建：%1。").arg(scheduledTime_.toString("yyyy-MM-dd HH:mm:ss")),
        QMessageBox::Ok
    );
}

void ShutdownDialog::handleCancelShutdownClicked() {
    QString errorMessage;
    if (!SystemCommandService::cancelShutdown(&errorMessage)) {
        const QString detail = errorMessage.isEmpty() ? QStringLiteral("无额外错误信息。") : errorMessage;
        QMessageBox::warning(
            this,
            QStringLiteral("取消失败"),
            QStringLiteral("当前没有可取消的关机计划，或命令执行失败。\n\n详情：%1").arg(detail),
            QMessageBox::Ok
        );
        return;
    }

    QMessageBox::information(
        this,
        QStringLiteral("已取消"),
        QStringLiteral("定时关机计划已取消。"),
        QMessageBox::Ok
    );
}

void ShutdownDialog::handleWatchTimer() {
    if (!watchTimer_) return;

    const QDateTime now = QDateTime::currentDateTime();
    if (now < scheduledTime_) {
        return; // not yet
    }

    // Stop watching while we handle trigger
    watchTimer_->stop();

    // Ask user to cancel within 30 seconds
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("关机确认");
    msgBox.setText(QString("系统即将关机。\n如需取消，请点击 取消 按钮。\n系统将在 %1 秒后关机。")
        .arg(30));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    // Start the 30-second system shutdown now (scheduled) and allow cancel
    QString errorMessage;
    if (!SystemCommandService::scheduleShutdown(30, &errorMessage)) {
        const QString detail = errorMessage.isEmpty() ? QStringLiteral("无额外错误信息。") : errorMessage;
        QMessageBox::critical(
            this,
            QStringLiteral("执行失败"),
            QStringLiteral("无法执行关机命令：%1").arg(detail),
            QMessageBox::Ok
        );
        return;
    }

    // Show modal dialog with Cancel option; if user cancels, abort system shutdown
    const int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel) {
        // User canceled via dialog, cancel system shutdown
        if (!SystemCommandService::cancelShutdown(&errorMessage)) {
            const QString detail = errorMessage.isEmpty() ? QStringLiteral("无额外错误信息。") : errorMessage;
            QMessageBox::warning(
                this,
                QStringLiteral("取消失败"),
                QStringLiteral("取消关机命令失败：%1").arg(detail),
                QMessageBox::Ok
            );
        } else {
            QMessageBox::information(
                this,
                QStringLiteral("已取消"),
                QStringLiteral("已取消关机。"),
                QMessageBox::Ok
            );
        }
    }
}
