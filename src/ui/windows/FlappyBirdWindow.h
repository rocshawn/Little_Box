#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QWidget>

class FlappyBirdModel;
class QLabel;
class QPushButton;
class SessionOverlayWidget;

class FlappyBirdWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FlappyBirdWidget(FlappyBirdModel* model, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void advanceFrame();

    FlappyBirdModel* model_{ nullptr };
    QTimer           timer_;
};

class FlappyBirdWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit FlappyBirdWindow(QWidget* parent = nullptr);
    ~FlappyBirdWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateScorePanel();
    void startNewGame();
    void showOverlay();
    void hideOverlay();

private:
    void setupUi();

    FlappyBirdModel*  model_{ nullptr };
    SessionOverlayWidget* overlay_{ nullptr };
    FlappyBirdWidget* gameWidget_{ nullptr };
    QLabel*           scoreValueLabel_{ nullptr };
    QLabel*           bestScoreValueLabel_{ nullptr };
    QLabel*           stateLabel_{ nullptr };
    QPushButton*      restartButton_{ nullptr };
    QPushButton*      closeButton_{ nullptr };
};
