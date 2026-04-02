#pragma once

#include <QObject>

enum class ThemeMode {
    Light,   // 日间模式
    Dark,    // 夜间模式
    System   // 跟随系统
};

class ThemeManager final : public QObject {
    Q_OBJECT

public:
    static ThemeManager& instance();

    ThemeMode mode() const;
    void setMode(ThemeMode mode);
    bool isDark() const;

signals:
    void themeChanged(bool dark);

private:
    explicit ThemeManager();

    bool systemIsDark() const;

    ThemeMode mode_{ ThemeMode::Light };
};
