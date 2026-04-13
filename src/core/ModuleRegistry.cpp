#include "ModuleRegistry.h"

#include "../ui/dialogs/ShutdownDialog.h"
#include "../ui/windows/Game2048Window.h"
#include "../ui/windows/TypingTestWindow.h"
#include "../ui/windows/SnakeGameWindow.h"
#include "../ui/windows/MinesweeperWindow.h"
#include "../ui/windows/MemoryMatchWindow.h"
#include "../ui/windows/MazeGameWindow.h"
#include "../ui/windows/FlappyBirdWindow.h"
#include "../ui/windows/PomodoroWindow.h"
#include "../ui/windows/ReactionTestWindow.h"
#include "../ui/windows/WatermarkWindow.h"

#include <QDesktopServices>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

namespace {
void bringToFront(QWidget* window) {
    if (window == nullptr) {
        return;
    }

    if (window->isMinimized()) {
        window->showNormal();
    } else {
        window->show();
    }

    window->raise();
    window->activateWindow();
}
} // namespace

ModuleRegistry& ModuleRegistry::instance() {
    static ModuleRegistry registry;
    return registry;
}

void ModuleRegistry::registerModule(const ModuleInfo& info) {
    modules_.append(info);
}

QList<ModuleInfo> ModuleRegistry::modulesForCategory(const ModuleCategory category) const {
    QList<ModuleInfo> result;
    for (const auto& module : modules_) {
        if (module.category == category) {
            result.append(module);
        }
    }
    return result;
}

void ModuleRegistry::launch(const QString& id, QWidget* parent) {
    for (const auto& module : modules_) {
        if (module.id != id) {
            continue;
        }

        if (module.launchMode == LaunchMode::Window) {
            // Reuse existing window if still alive
            auto it = openWindows_.find(id);
            if (it != openWindows_.end() && !it.value().isNull()) {
                bringToFront(it.value().data());
                return;
            }
        }

        module.launcher(parent);
        return;
    }
}

void ModuleRegistry::registerBuiltinModules() {
    if (!modules_.isEmpty()) {
        return;
    }

    // ── Tools ──────────────────────────────────────────

    registerModule({
        /* id          */ "pomodoro",
        /* name        */ "番茄钟",
        /* description */ "25分钟专注时间，带有悬浮小窗功能",
        /* emoji       */ "\xE2\x8F\xB1",  // ⏱
        /* category    */ ModuleCategory::Tools,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new PomodoroWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["pomodoro"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "shutdown",
        /* name        */ "定时关机",
        /* description */ "设定到达时间，系统自动执行关机",
        /* emoji       */ "\xE2\x8F\xB0",  // ⏰
        /* category    */ ModuleCategory::Tools,
        /* launchMode  */ LaunchMode::Dialog,
        /* launcher    */ [](QWidget* parent) {
            ShutdownDialog dialog(parent);
            dialog.exec();
        }
    });

    registerModule({
        /* id          */ "website",
        /* name        */ "打开网站",
        /* description */ "快速打开已保存的网站地址",
        /* emoji       */ "\xF0\x9F\x8C\x90",  // 🌐
        /* category    */ ModuleCategory::Tools,
        /* launchMode  */ LaunchMode::Action,
        /* launcher    */ [](QWidget* parent) {
            QSettings settings;
            QString website = settings.value("website/url").toString();

            if (website.isEmpty()) {
                bool ok = false;
                const QString input = QInputDialog::getText(
                    parent,
                    "设置网站地址",
                    "请输入要打开的网站 URL（含 http:// 或 https://）：",
                    QLineEdit::Normal,
                    QString(),
                    &ok
                );

                if (!ok) {
                    return;
                }

                const QString trimmed = input.trimmed();
                if (trimmed.isEmpty()) {
                    return;
                }

                if (!(trimmed.startsWith("http://") || trimmed.startsWith("https://"))) {
                    QMessageBox::warning(
                        parent,
                        QStringLiteral("无效网址"),
                        QStringLiteral("网址需要以 http:// 或 https:// 开头。"),
                        QMessageBox::Ok
                    );
                    return;
                }

                settings.setValue("website/url", trimmed);
                website = trimmed;
            }

            const bool opened = QDesktopServices::openUrl(QUrl(website));
            if (!opened) {
                QMessageBox::warning(
                    parent,
                    QStringLiteral("打开失败"),
                    QStringLiteral("无法使用默认浏览器打开网页。请检查系统浏览器配置。"),
                    QMessageBox::Ok
                );
            }
        }
    });

    registerModule({
        /* id          */ "watermark",
        /* name        */ "图片水印",
        /* description */ "给图片添加自定义文字水印，支持高品质导出",
        /* emoji       */ "\xF0\x9F\x96\xBC",  // 🖼
        /* category    */ ModuleCategory::Tools,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new WatermarkWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["watermark"] = window;
            bringToFront(window);
        }
    });

    // ── Games ──────────────────────────────────────────

    registerModule({
        /* id          */ "2048",
        /* name        */ "2048",
        /* description */ "经典的数字合并滑块游戏，甜心马卡龙版",
        /* emoji       */ "\xF0\x9F\x8E\xB2",  // 🎲
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new Game2048Window(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["2048"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "snake",
        /* name        */ "贪吃蛇",
        /* description */ "果冻质感的英文经典贪吃蛇小游戏",
        /* emoji       */ "\xF0\x9F\x90\x8D",  // 🐍
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new SnakeGameWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["snake"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "minesweeper",
        /* name        */ "可爱扫雷",
        /* description */ "粉色立体风格的地雷阵挑战",
        /* emoji       */ "\xF0\x9F\x92\xA3",  // 💣
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new MinesweeperWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["minesweeper"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "memorymatch",
        /* name        */ "记忆翻牌",
        /* description */ "翻转卡片找到配对的可爱 Emoji",
        /* emoji       */ "\xF0\x9F\xA7\xA9",  // 🧩
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new MemoryMatchWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["memorymatch"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "maze",
        /* name        */ "迷宫闯关",
        /* description */ "方向键控制，穿越迷宫到达出口",
        /* emoji       */ "\xF0\x9F\x8C\x80",  // 🌀
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new MazeGameWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["maze"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "flappybird",
        /* name        */ "Flappy Bird",
        /* description */ "控制小鸟飞行，穿越水管挑战高分",
        /* emoji       */ "\xF0\x9F\x90\xA5",  // 🐥
        /* category    */ ModuleCategory::Games,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new FlappyBirdWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["flappybird"] = window;
            bringToFront(window);
        }
    });

    // ── Challenge ──────────────────────────────────────

    registerModule({
        /* id          */ "typingtest",
        /* name        */ "打字速度测验",
        /* description */ "名著节选英文打字训练，监测你的手速",
        /* emoji       */ "\xE2\x8C\xA8\xEF\xB8\x8F",  // ⌨️
        /* category    */ ModuleCategory::Challenge,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new TypingTestWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["typingtest"] = window;
            bringToFront(window);
        }
    });

    registerModule({
        /* id          */ "reaction",
        /* name        */ "反应速度测试",
        /* description */ "屏幕变绿后点击，测量你的反应速度",
        /* emoji       */ "\xE2\x9A\xA1",  // ⚡
        /* category    */ ModuleCategory::Challenge,
        /* launchMode  */ LaunchMode::Window,
        /* launcher    */ [this](QWidget* parent) {
            auto* window = new ReactionTestWindow(parent);
            window->setAttribute(Qt::WA_DeleteOnClose);
            openWindows_["reaction"] = window;
            bringToFront(window);
        }
    });
}
