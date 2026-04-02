#pragma once

#include "../../core/ModuleInfo.h"

#include <QMainWindow>

class QLabel;
class QWidget;
class ModuleCardGrid;
class Sidebar;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onCategoryChanged(ModuleCategory category);
    void onModuleClicked(const QString& id);
    void onSettingsRequested();
    void onThemeChanged(bool dark);
    void toggleFullscreen();

private:
    void setupUi();
    void setupConnections();
    void setupStatusBar();
    void updateWindowModeUi();
    void refreshModuleGrid(ModuleCategory category);
    void applyThemeStylesheet();

    Sidebar*        sidebar_{ nullptr };
    ModuleCardGrid* cardGrid_{ nullptr };
    QWidget*        contentArea_{ nullptr };
    QLabel*         headerTitleLabel_{ nullptr };
    QLabel*         headerSubtitleLabel_{ nullptr };

    ModuleCategory  currentCategory_{ ModuleCategory::Tools };
};
