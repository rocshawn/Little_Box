#include "ReactionTestWidget.h"

#include "../../logic/ReactionTestModel.h"

#include <QBrush>
#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QStringList>

#include <algorithm>

namespace {
constexpr int kChartTopFloorMs = 300;
} // namespace

ReactionTestWidget::ReactionTestWidget(QWidget* parent)
    : QWidget(parent),
      model_(new ReactionTestModel(this)) {
    setMinimumHeight(320);
    setCursor(Qt::PointingHandCursor);

    connect(model_, &ReactionTestModel::updated, this, &ReactionTestWidget::updateUi);
    connect(model_, &ReactionTestModel::stateChanged, this, &ReactionTestWidget::updateUi);
}

void ReactionTestWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    model_->handleInteraction();
    event->accept();
}

void ReactionTestWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QColor background = model_->state() == ReactionTestModel::State::Measuring ? QColor(34, 197, 94) : QColor(220, 38, 38);
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
    painter.setPen(Qt::white);
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

void ReactionTestWidget::updateUi() {
    update();
}

QString ReactionTestWidget::buildMainText() const {
    const int finishedRounds = static_cast<int>(model_->roundTimesMs().size());
    const int currentRound = std::min(finishedRounds + 1, model_->totalRounds());

    if (model_->state() == ReactionTestModel::State::ReadyToStart) {
        if (finishedRounds == 0) {
            return "点击任意地方开始测试，屏幕变绿后点击";
        }
        return QString("第 %1 轮：%2 ms，点击开始第 %3 轮")
            .arg(finishedRounds)
            .arg(model_->roundTimesMs().last())
            .arg(currentRound);
    }

    if (model_->state() == ReactionTestModel::State::WaitingGreen) {
        if (model_->clickedTooEarly()) {
            return QString("太快了，请等待屏幕变绿后再点击（第 %1 轮）").arg(currentRound);
        }
        return QString("第 %1 轮准备中，屏幕变绿后点击").arg(currentRound);
    }

    if (model_->state() == ReactionTestModel::State::Measuring) {
        return QString("第 %1 轮开始，立即点击").arg(currentRound);
    }

    return QString("五轮完成，平均 %1 ms。点击重新开始").arg(model_->averageMs());
}

QString ReactionTestWidget::buildProgressText() const {
    QString text = QString("进度：%1/%2").arg(static_cast<int>(model_->roundTimesMs().size())).arg(model_->totalRounds());

    if (model_->bestAverageMs() > 0) {
        text += QString("   本地最佳平均：%1 ms").arg(model_->bestAverageMs());
    }

    if (model_->state() == ReactionTestModel::State::Finished && model_->isNewRecord()) {
        text += "   新纪录";
    }

    return text;
}

QString ReactionTestWidget::buildRoundHistoryText() const {
    if (model_->roundTimesMs().isEmpty()) {
        return "本次成绩：暂无";
    }

    QStringList pieces;
    pieces.reserve(static_cast<int>(model_->roundTimesMs().size()));

    for (int i = 0; i < model_->roundTimesMs().size(); ++i) {
        pieces.push_back(QString("R%1 %2").arg(i + 1).arg(model_->roundTimesMs().at(i)));
    }

    return "本次成绩：" + pieces.join(" ms  |  ") + " ms";
}

void ReactionTestWidget::drawChart(QPainter& painter, const QRect& panelRect) const {
    if (panelRect.width() < 80 || panelRect.height() < 80) return;

    const QRect plotRect = panelRect.adjusted(34, 8, -12, -28);
    if (plotRect.width() <= 0 || plotRect.height() <= 0) return;

    painter.setPen(QPen(QColor(148, 163, 184), 1));
    painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight());
    painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());

    QFont axisFont = painter.font();
    axisFont.setPointSize(9);
    axisFont.setBold(false);
    painter.setFont(axisFont);
    painter.setPen(QColor(71, 85, 105));

    const int total = model_->totalRounds();
    for (int round = 1; round <= total; ++round) {
        const double ratio = total == 1 ? 0.0 : static_cast<double>(round - 1) / static_cast<double>(total - 1);
        const int x = plotRect.left() + qRound(ratio * plotRect.width());
        painter.drawLine(QPoint(x, plotRect.bottom()), QPoint(x, plotRect.bottom() + 4));
        painter.drawText(QRect(x - 8, plotRect.bottom() + 6, 16, 16), Qt::AlignCenter, QString::number(round));
    }

    if (model_->roundTimesMs().isEmpty()) {
        painter.drawText(plotRect, Qt::AlignCenter, "完成至少 1 轮后显示折线图");
        return;
    }

    const int maxRound = *std::max_element(model_->roundTimesMs().cbegin(), model_->roundTimesMs().cend());
    const int yMax = std::max(kChartTopFloorMs, maxRound + 50);
    painter.drawText(QRect(plotRect.left() - 30, plotRect.top() - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(yMax));
    painter.drawText(QRect(plotRect.left() - 30, plotRect.bottom() - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter, "0");

    QVector<QPoint> points;
    points.reserve(static_cast<int>(model_->roundTimesMs().size()));
    for (int i = 0; i < model_->roundTimesMs().size(); ++i) {
        const double ratioX = total == 1 ? 0.0 : static_cast<double>(i) / static_cast<double>(total - 1);
        const int x = plotRect.left() + qRound(ratioX * plotRect.width());
        const double ratioY = static_cast<double>(model_->roundTimesMs().at(i)) / static_cast<double>(yMax);
        const int y = plotRect.bottom() - qRound(ratioY * plotRect.height());
        points.push_back(QPoint(x, y));
    }

    painter.setPen(QPen(QColor(37, 99, 235), 2));
    painter.drawPolyline(points.constData(), static_cast<int>(points.size()));

    painter.setBrush(QColor(37, 99, 235));
    for (int i = 0; i < points.size(); ++i) {
        const auto& pt = points.at(i);
        painter.drawEllipse(pt, 4, 4);
        painter.drawText(QRect(pt.x() - 18, pt.y() - 22, 36, 16),
             Qt::AlignCenter,
             QString::number(model_->roundTimesMs().at(i)));
    }
}
