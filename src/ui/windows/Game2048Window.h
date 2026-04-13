#pragma once

#include <QMainWindow>
#include <QVector>

class Game2048Model;
class QGridLayout;
class QLabel;
class SessionOverlayWidget;

class Game2048Window final : public QMainWindow {
    Q_OBJECT

public:
    explicit Game2048Window(QWidget* parent = nullptr);
    ~Game2048Window() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateUi();
    void resetGame();
    void onGameOver();
    void onGameWon();
    void showOverlay();
    void hideOverlay();

private:
    void setupUi();
    QString colorForNumber(int num, bool darkTheme) const;

    Game2048Model* model_{ nullptr };
    SessionOverlayWidget* overlay_{ nullptr };

    QLabel* scoreLabel_{ nullptr };
    QLabel* statusLabel_{ nullptr };
    QGridLayout* gridLayout_{ nullptr };
    QVector<QLabel*> tiles_;
};
