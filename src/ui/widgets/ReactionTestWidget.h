#pragma once

#include <QString>
#include <QVector>
#include <QWidget>
#include <QRect>
#include <QPoint>

class ReactionTestModel;
class QPainter;
class QMouseEvent;
class QPaintEvent;

class ReactionTestWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ReactionTestWidget(QWidget* parent = nullptr);
    ~ReactionTestWidget() override = default;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateUi();

private:
    QString buildMainText() const;
    QString buildProgressText() const;
    QString buildRoundHistoryText() const;
    void drawChart(QPainter& painter, const QRect& panelRect) const;

    ReactionTestModel* model_{ nullptr };
};
