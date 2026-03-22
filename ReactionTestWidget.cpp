#include "ReactionTestWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QtGlobal>

#include <algorithm>
#include <numeric>

namespace {
constexpr int kDelayMinMs = 2000;
constexpr int kDelayMaxMs = 5000;
constexpr int kChartTopFloorMs = 300;
} // namespace

ReactionTestWidget::ReactionTestWidget(QWidget* parent)
    : QWidget(parent),
      waitTimer_(new QTimer(this)) {
    setMinimumHeight(320);
    setCursor(Qt::PointingHandCursor);

    waitTimer_->setSingleShot(true);
    connect(waitTimer_, &QTimer::timeout, this, &ReactionTestWidget::enterMeasuringState);

    const QSettings settings;
    const int savedBest = settings.value("reaction_test/best_average_ms", -1).toInt();
    if (savedBest > 0) {
        bestAverageMs_ = savedBest;
    }
}

void ReactionTestWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    switch (state_) {
    case State::ReadyToStart:
        clickedTooEarly_ = false;
        startRoundCountdown();
        break;
    case State::WaitingGreen:
        clickedTooEarly_ = true;
        startRoundCountdown();
        break;
    case State::Measuring:
        completeCurrentRound();
        break;
    case State::Finished:
        resetSession();
        startRoundCountdown();
        break;
    }

    event->accept();
}

void ReactionTestWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QColor background = state_ == State::Measuring ? QColor(34, 197, 94) : QColor(220, 38, 38);
    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 20, 20);

    const QRect contentRect = rect().adjusted(16, 16, -16, -16);
    const QRect headlineRect(contentRect.left(), contentRect.top(), contentRect.width(), 70);
    const QRect progressRect(contentRect.left(), headlineRect.bottom() + 6, contentRect.width(), 24);
    const QRect historyRect(contentRect.left(), progressRect.bottom() + 2, contentRect.width(), 24);
    const QRect chartPanelRect(contentRect.left(), historyRect.bottom() + 12, contentRect.width(), contentRect.bottom() - historyRect.bottom() - 12);

    QFont headlineFont = painter.font();
    headlineFont.setPointSize(15);
    headlineFont.setBold(true);
    painter.setFont(headlineFont);
    painter.setPen(Qt::white);
    painter.drawText(headlineRect, Qt::AlignCenter | Qt::TextWordWrap, buildMainText());

    QFont infoFont = painter.font();
    infoFont.setPointSize(10);
    infoFont.setBold(false);
    painter.setFont(infoFont);
    painter.drawText(progressRect, Qt::AlignCenter, buildProgressText());
    painter.drawText(historyRect, Qt::AlignCenter, buildRoundHistoryText());

    painter.setBrush(QColor(255, 255, 255, 235));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(chartPanelRect, 14, 14);

    QFont chartTitleFont = painter.font();
    chartTitleFont.setPointSize(10);
    chartTitleFont.setBold(true);
    painter.setFont(chartTitleFont);
    painter.setPen(QColor(51, 65, 85));
    painter.drawText(chartPanelRect.adjusted(12, 10, -12, -10), Qt::AlignLeft | Qt::AlignTop, "已完成轮次折线图（ms）");

    const QRect chartRect = chartPanelRect.adjusted(14, 34, -14, -12);
    drawChart(painter, chartRect);
}

void ReactionTestWidget::startRoundCountdown() {
    state_ = State::WaitingGreen;
    waitTimer_->stop();

    const int delayMs = QRandomGenerator::global()->bounded(kDelayMinMs, kDelayMaxMs + 1);
    waitTimer_->start(delayMs);

    update();
}

void ReactionTestWidget::enterMeasuringState() {
    state_ = State::Measuring;
    clickedTooEarly_ = false;
    reactionTimer_.restart();
    update();
}

void ReactionTestWidget::completeCurrentRound() {
    if (!reactionTimer_.isValid()) {
        return;
    }

    roundTimesMs_.append(static_cast<int>(reactionTimer_.elapsed()));

    if (roundTimesMs_.size() >= kTotalRounds) {
        finalizeSession();
    } else {
        state_ = State::ReadyToStart;
    }

    update();
}

void ReactionTestWidget::finalizeSession() {
    waitTimer_->stop();
    state_ = State::Finished;
    isNewRecord_ = false;

    if (roundTimesMs_.isEmpty()) {
        averageMs_ = -1;
        return;
    }

    const int total = std::accumulate(roundTimesMs_.cbegin(), roundTimesMs_.cend(), 0);
    averageMs_ = qRound(static_cast<double>(total) / static_cast<double>(roundTimesMs_.size()));

    if (averageMs_ > 0 && (bestAverageMs_ < 0 || averageMs_ < bestAverageMs_)) {
        bestAverageMs_ = averageMs_;
        isNewRecord_ = true;

        QSettings settings;
        settings.setValue("reaction_test/best_average_ms", bestAverageMs_);
    }
}

void ReactionTestWidget::resetSession() {
    waitTimer_->stop();
    reactionTimer_.invalidate();
    roundTimesMs_.clear();
    state_ = State::ReadyToStart;
    clickedTooEarly_ = false;
    averageMs_ = -1;
    isNewRecord_ = false;
    update();
}

QString ReactionTestWidget::buildMainText() const {
    const int finishedRounds = roundTimesMs_.size();
    const int currentRound = std::min(finishedRounds + 1, kTotalRounds);

    if (state_ == State::ReadyToStart) {
        if (finishedRounds == 0) {
            return "点击任意地方开始测试，屏幕变绿后点击";
        }

        return QString("第 %1 轮：%2 ms，点击开始第 %3 轮")
            .arg(finishedRounds)
            .arg(roundTimesMs_.last())
            .arg(currentRound);
    }

    if (state_ == State::WaitingGreen) {
        if (clickedTooEarly_) {
            return QString("太快了，请等待屏幕变绿后再点击（第 %1 轮）").arg(currentRound);
        }

        return QString("第 %1 轮准备中，屏幕变绿后点击").arg(currentRound);
    }

    if (state_ == State::Measuring) {
        return QString("第 %1 轮开始，立即点击").arg(currentRound);
    }

    return QString("五轮完成，平均 %1 ms。点击重新开始").arg(averageMs_);
}

QString ReactionTestWidget::buildProgressText() const {
    QString text = QString("进度：%1/%2").arg(roundTimesMs_.size()).arg(kTotalRounds);

    if (bestAverageMs_ > 0) {
        text += QString("   本地最佳平均：%1 ms").arg(bestAverageMs_);
    }

    if (state_ == State::Finished && isNewRecord_) {
        text += "   新纪录";
    }

    return text;
}

QString ReactionTestWidget::buildRoundHistoryText() const {
    if (roundTimesMs_.isEmpty()) {
        return "本次成绩：暂无";
    }

    QStringList pieces;
    pieces.reserve(roundTimesMs_.size());

    for (int i = 0; i < roundTimesMs_.size(); ++i) {
        pieces.push_back(QString("R%1 %2").arg(i + 1).arg(roundTimesMs_.at(i)));
    }

    return "本次成绩：" + pieces.join(" ms  |  ") + " ms";
}

void ReactionTestWidget::drawChart(QPainter& painter, const QRect& panelRect) const {
    if (panelRect.width() < 80 || panelRect.height() < 80) {
        return;
    }

    const QRect plotRect = panelRect.adjusted(34, 8, -12, -28);
    if (plotRect.width() <= 0 || plotRect.height() <= 0) {
        return;
    }

    painter.setPen(QPen(QColor(148, 163, 184), 1));
    painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight());
    painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());

    QFont axisFont = painter.font();
    axisFont.setPointSize(9);
    axisFont.setBold(false);
    painter.setFont(axisFont);
    painter.setPen(QColor(71, 85, 105));

    for (int round = 1; round <= kTotalRounds; ++round) {
        const double ratio = kTotalRounds == 1 ? 0.0 : static_cast<double>(round - 1) / static_cast<double>(kTotalRounds - 1);
        const int x = plotRect.left() + qRound(ratio * plotRect.width());
        painter.drawLine(QPoint(x, plotRect.bottom()), QPoint(x, plotRect.bottom() + 4));
        painter.drawText(QRect(x - 8, plotRect.bottom() + 6, 16, 16), Qt::AlignCenter, QString::number(round));
    }

    if (roundTimesMs_.isEmpty()) {
        painter.drawText(plotRect, Qt::AlignCenter, "完成至少 1 轮后显示折线图");
        return;
    }

    const int maxRound = *std::max_element(roundTimesMs_.cbegin(), roundTimesMs_.cend());
    const int yMax = std::max(kChartTopFloorMs, maxRound + 50);
    painter.drawText(QRect(plotRect.left() - 30, plotRect.top() - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(yMax));
    painter.drawText(QRect(plotRect.left() - 30, plotRect.bottom() - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter, "0");

    QVector<QPoint> points;
    points.reserve(roundTimesMs_.size());
    for (int i = 0; i < roundTimesMs_.size(); ++i) {
        const double ratioX = kTotalRounds == 1 ? 0.0 : static_cast<double>(i) / static_cast<double>(kTotalRounds - 1);
        const int x = plotRect.left() + qRound(ratioX * plotRect.width());
        const double ratioY = static_cast<double>(roundTimesMs_.at(i)) / static_cast<double>(yMax);
        const int y = plotRect.bottom() - qRound(ratioY * plotRect.height());
        points.push_back(QPoint(x, y));
    }

    painter.setPen(QPen(QColor(37, 99, 235), 2));
    painter.drawPolyline(points.constData(), points.size());

    painter.setBrush(QColor(37, 99, 235));
    for (int i = 0; i < points.size(); ++i) {
        painter.drawEllipse(points.at(i), 4, 4);
        painter.drawText(QRect(points.at(i).x() - 18, points.at(i).y() - 22, 36, 16),
            Qt::AlignCenter,
            QString::number(roundTimesMs_.at(i)));
    }
}
