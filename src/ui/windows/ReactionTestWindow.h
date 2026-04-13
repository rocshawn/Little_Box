#pragma once

#include <QMainWindow>

class ReactionTestModel;
class QLabel;
class QPushButton;
class ReactionTestWidget;

class ReactionTestWindow final : public QMainWindow {
    Q_OBJECT // <--- 必须添加这一行，注意不需要分号

public:
    explicit ReactionTestWindow(QWidget* parent = nullptr);
    ~ReactionTestWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots: // 因为你使用了 slots，所以没有 Q_OBJECT 必报错
    void updateUi();
    void onSessionFinished(int avg);

private:
    void setupUi();

    ReactionTestModel* model_{ nullptr };
    QLabel* titleLabel_{ nullptr };
    QLabel* subtitleLabel_{ nullptr };
    ReactionTestWidget* reactionTestWidget_{ nullptr };
    QPushButton* closeButton_{ nullptr };
};