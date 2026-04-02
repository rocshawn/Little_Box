#pragma once

#include <QString>

#include <functional>

class QWidget;

enum class ModuleCategory {
    Tools,      // 工具
    Games,      // 游戏
    Challenge   // 挑战
};

enum class LaunchMode {
    Dialog,     // 弹出模态对话框（如定时关机）
    Window,     // 打开独立子窗口（如游戏）
    Action      // 执行一个即时动作（如打开网站）
};

struct ModuleInfo {
    QString                                 id{};
    QString                                 name{};
    QString                                 description{};
    QString                                 emoji{};
    ModuleCategory                          category{ ModuleCategory::Tools };
    LaunchMode                              launchMode{ LaunchMode::Window };
    std::function<void(QWidget* parent)>    launcher{};
};
