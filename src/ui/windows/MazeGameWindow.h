#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QWidget>

class MazeGameModel;
class QLabel;
class QPushButton;

class MazeBoard final : public QWidget {
    Q_OBJECT
public:
    explicit MazeBoard(MazeGameModel* model, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    MazeGameModel* model_{ nullptr };
};

class MazeGameWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MazeGameWindow(QWidget* parent = nullptr);
    ~MazeGameWindow() override = default;

private slots:
    void handleLevelCompleted();
    void updateLabels();

private:
    void setupUi();
    void setupConnections();

    MazeGameModel* model_{ nullptr };
    MazeBoard*     board_{ nullptr };
    QLabel*        levelLabel_{ nullptr };
    QLabel*        hintLabel_{ nullptr };
    QPushButton*   restartButton_{ nullptr };
    QPushButton*   closeButton_{ nullptr };
};
