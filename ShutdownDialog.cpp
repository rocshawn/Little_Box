#include "ShutdownDialog.h"

#include "SystemCommandService.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>

#include <limits>

namespace {
constexpr int kDialogWidth = 500;
constexpr int kDialogHeight = 340;
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
    auto* descriptionLabel = new QLabel("选择目标日期时间后，系统会在该时刻自动关机。", contentCard);

    auto* inputLabel = new QLabel("目标时间（年 / 月 / 日 / 时 / 分 / 秒）", contentCard);
    auto* helperLabel = new QLabel("请填写六个时间字段。默认值为当前时间后 5 分钟。", contentCard);
    auto* inputLayout = new QVBoxLayout();
    auto* dateRowLayout = new QHBoxLayout();
    auto* timeRowLayout = new QHBoxLayout();
    auto* buttonLayout = new QHBoxLayout();

    const QDateTime defaultDateTime = QDateTime::currentDateTime().addSecs(5 * 60);

    const QString defaultMonth = QString("%1").arg(defaultDateTime.date().month(), 2, 10, QChar('0'));
    const QString defaultDay = QString("%1").arg(defaultDateTime.date().day(), 2, 10, QChar('0'));
    const QString defaultHour = QString("%1").arg(defaultDateTime.time().hour(), 2, 10, QChar('0'));
    const QString defaultMinute = QString("%1").arg(defaultDateTime.time().minute(), 2, 10, QChar('0'));
    const QString defaultSecond = QString("%1").arg(defaultDateTime.time().second(), 2, 10, QChar('0'));

    yearEdit_ = new QLineEdit(QString::number(defaultDateTime.date().year()), contentCard);
    monthEdit_ = new QLineEdit(defaultMonth, contentCard);
    dayEdit_ = new QLineEdit(defaultDay, contentCard);
    hourEdit_ = new QLineEdit(defaultHour, contentCard);
    minuteEdit_ = new QLineEdit(defaultMinute, contentCard);
    secondEdit_ = new QLineEdit(defaultSecond, contentCard);

    auto* yearUnitLabel = new QLabel("年", contentCard);
    auto* monthUnitLabel = new QLabel("月", contentCard);
    auto* dayUnitLabel = new QLabel("日", contentCard);
    auto* hourSeparatorLabel = new QLabel(":", contentCard);
    auto* minuteSeparatorLabel = new QLabel(":", contentCard);

    yearUnitLabel->setObjectName("unitLabel");
    monthUnitLabel->setObjectName("unitLabel");
    dayUnitLabel->setObjectName("unitLabel");
    hourSeparatorLabel->setObjectName("separatorLabel");
    minuteSeparatorLabel->setObjectName("separatorLabel");

    yearEdit_->setValidator(new QIntValidator(1970, 9999, yearEdit_));
    monthEdit_->setValidator(new QIntValidator(1, 12, monthEdit_));
    dayEdit_->setValidator(new QIntValidator(1, 31, dayEdit_));
    hourEdit_->setValidator(new QIntValidator(0, 23, hourEdit_));
    minuteEdit_->setValidator(new QIntValidator(0, 59, minuteEdit_));
    secondEdit_->setValidator(new QIntValidator(0, 59, secondEdit_));

    for (auto* edit : { yearEdit_, monthEdit_, dayEdit_, hourEdit_, minuteEdit_, secondEdit_ }) {
        edit->setAlignment(Qt::AlignCenter);
        edit->setClearButtonEnabled(false);
    }

    yearEdit_->setFixedWidth(84);
    monthEdit_->setFixedWidth(60);
    dayEdit_->setFixedWidth(60);
    hourEdit_->setFixedWidth(60);
    minuteEdit_->setFixedWidth(60);
    secondEdit_->setFixedWidth(60);

    yearEdit_->setMaxLength(4);
    monthEdit_->setMaxLength(2);
    dayEdit_->setMaxLength(2);
    hourEdit_->setMaxLength(2);
    minuteEdit_->setMaxLength(2);
    secondEdit_->setMaxLength(2);

    dateRowLayout->addWidget(yearEdit_);
    dateRowLayout->addWidget(yearUnitLabel);
    dateRowLayout->addSpacing(8);
    dateRowLayout->addWidget(monthEdit_);
    dateRowLayout->addWidget(monthUnitLabel);
    dateRowLayout->addSpacing(8);
    dateRowLayout->addWidget(dayEdit_);
    dateRowLayout->addWidget(dayUnitLabel);
    dateRowLayout->addStretch();
    dateRowLayout->setSpacing(6);
    dateRowLayout->setContentsMargins(0, 0, 0, 0);

    timeRowLayout->addWidget(hourEdit_);
    timeRowLayout->addWidget(hourSeparatorLabel);
    timeRowLayout->addWidget(minuteEdit_);
    timeRowLayout->addWidget(minuteSeparatorLabel);
    timeRowLayout->addWidget(secondEdit_);
    timeRowLayout->addStretch();
    timeRowLayout->setSpacing(8);
    timeRowLayout->setContentsMargins(0, 0, 0, 0);

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
    inputLayout->addLayout(dateRowLayout);
    inputLayout->addLayout(timeRowLayout);
    inputLayout->addWidget(helperLabel);
    inputLayout->setSpacing(10);

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
        "QLabel#unitLabel { color:#6366f1; font-size:12px; font-weight:700; }"
        "QLabel#separatorLabel { color:#475569; font-size:16px; font-weight:700; }"
        "QLabel#helperLabel { color:#64748b; font-size:12px; margin-top:8px; }"
        "QLineEdit { border:1px solid #c7d2fe; border-radius:12px; padding:10px 8px; background:#f8faff; color:#0f172a; }"
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
}

void ShutdownDialog::handleConfirmClicked() {
    const QDateTime now = QDateTime::currentDateTime();

    bool okYear = false;
    bool okMonth = false;
    bool okDay = false;
    bool okHour = false;
    bool okMinute = false;
    bool okSecond = false;

    const int year = yearEdit_->text().toInt(&okYear);
    const int month = monthEdit_->text().toInt(&okMonth);
    const int day = dayEdit_->text().toInt(&okDay);
    const int hour = hourEdit_->text().toInt(&okHour);
    const int minute = minuteEdit_->text().toInt(&okMinute);
    const int second = secondEdit_->text().toInt(&okSecond);

    if (!(okYear && okMonth && okDay && okHour && okMinute && okSecond)) {
        QMessageBox::warning(this, "输入无效", "请完整填写年月日时分秒。");
        return;
    }

    const QDate date(year, month, day);
    const QTime time(hour, minute, second);
    if (!date.isValid() || !time.isValid()) {
        QMessageBox::warning(this, "输入无效", "日期或时间不合法，请检查后重试。");
        return;
    }

    const QDateTime target(date, time);
    if (target <= now) {
        QMessageBox::warning(this, "输入无效", "请选择一个将来的日期时间。");
        yearEdit_->setFocus();
        return;
    }

    const qint64 secondsToShutdown = now.secsTo(target);
    if (secondsToShutdown > std::numeric_limits<int>::max()) {
        QMessageBox::warning(this, "输入无效", "选择的时间太远，请设置一个更近的关机时间。");
        yearEdit_->setFocus();
        return;
    }

    const auto result = QMessageBox::question(
        this,
        "确认创建计划",
        QString("系统将在 %1 自动关机（约 %2 秒后），是否继续？")
            .arg(target.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(secondsToShutdown),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;
    if (!SystemCommandService::scheduleShutdown(static_cast<int>(secondsToShutdown), &errorMessage)) {
        const QString detail = errorMessage.isEmpty() ? QStringLiteral("无额外错误信息。") : errorMessage;
        QMessageBox::critical(this, "创建失败", QString("无法创建关机计划，请检查系统权限或命令执行状态。\n\n详情：%1").arg(detail));
        return;
    }

    hasScheduledPlan_ = true;
    QMessageBox::information(this, "创建成功", QString("定时关机计划已创建：%1。").arg(target.toString("yyyy-MM-dd HH:mm:ss")));
}

void ShutdownDialog::handleCancelShutdownClicked() {
    QString errorMessage;
    const bool canceled = SystemCommandService::cancelShutdown(&errorMessage);
    if (!hasScheduledPlan_ && !canceled) {
        return;
    }

    hasScheduledPlan_ = false;
    QMessageBox::information(this, "已取消", "定时关机计划已取消。");
}
