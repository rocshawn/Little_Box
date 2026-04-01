#pragma once

#include <QMainWindow>

class QLabel;
class QPushButton;
class ReactionTestWidget;

class ReactionTestWindow final : public QMainWindow {
public:
    explicit ReactionTestWindow(QWidget* parent = nullptr);
    ~ReactionTestWindow() override = default;

private:
    void setupUi();

    QLabel* titleLabel_{ nullptr };
    QLabel* subtitleLabel_{ nullptr };
    ReactionTestWidget* reactionTestWidget_{ nullptr };
    QPushButton* closeButton_{ nullptr };
};
