#include "ThemeManager.h"

#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

namespace {
constexpr auto kThemeSettingsKey = "ui/themeMode";
} // namespace

ThemeManager::ThemeManager() {
    QSettings settings;
    const int saved = settings.value(kThemeSettingsKey, static_cast<int>(ThemeMode::Light)).toInt();
    mode_ = static_cast<ThemeMode>(saved);

    // React to system dark/light changes when mode is System
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this](Qt::ColorScheme) {
        if (mode_ == ThemeMode::System) {
            emit themeChanged(isDark());
        }
    });
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager mgr;
    return mgr;
}

ThemeMode ThemeManager::mode() const {
    return mode_;
}

void ThemeManager::setMode(const ThemeMode mode) {
    if (mode_ == mode) {
        return;
    }
    mode_ = mode;

    QSettings settings;
    settings.setValue(kThemeSettingsKey, static_cast<int>(mode_));

    emit themeChanged(isDark());
}

bool ThemeManager::isDark() const {
    switch (mode_) {
    case ThemeMode::Light:  return false;
    case ThemeMode::Dark:   return true;
    case ThemeMode::System: return systemIsDark();
    }
    return false;
}

bool ThemeManager::systemIsDark() const {
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}
