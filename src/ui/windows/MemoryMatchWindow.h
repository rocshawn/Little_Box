#pragma once

#include <QMainWindow>
#include <QVector>
#include <QStringList>
#include <QTimer>

class MemoryMatchWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MemoryMatchWindow(QWidget* parent = nullptr);
    ~MemoryMatchWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct Card {
        Card() = default;
        Card(const QString& e, bool f, bool m) : emoji(e), isFlipped(f), isMatched(m) {}

        QString emoji;
        bool isFlipped{ false };
        bool isMatched{ false };
    };

    void setupUi();
    void resetGame();
    void checkMatch();

    static constexpr int kRows = 4;
    static constexpr int kCols = 4;
    static constexpr int kCellSize = 80;

    QVector<Card> cards_;
    int firstFlippedIdx_{ -1 };
    int secondFlippedIdx_{ -1 };
    bool isProcessing_{ false };
    int matchesFound_{ 0 };
};
