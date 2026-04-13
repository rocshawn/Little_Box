#pragma once

#include <QWidget>

class QPushButton;
class QLabel;

class SessionOverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit SessionOverlayWidget(QWidget* parent = nullptr);
    ~SessionOverlayWidget() override = default;

signals:
    void continueRequested();
    void restartRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void setupUi();

    QPushButton* continueBtn_{ nullptr };
    QPushButton* restartBtn_{ nullptr };
    QLabel* titleLabel_{ nullptr };
};
