#pragma once

#include "../../core/ModuleInfo.h"

#include <QList>
#include <QWidget>

class QPushButton;

class Sidebar final : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    ModuleCategory currentCategory() const;

signals:
    void categoryChanged(ModuleCategory category);
    void settingsRequested();

private:
    void setupUi();
    void selectCategory(ModuleCategory category);

    struct CategoryEntry {
        ModuleCategory  category{ ModuleCategory::Tools };
        QPushButton*    button{ nullptr };
    };

    QList<CategoryEntry> entries_;
    ModuleCategory currentCategory_{ ModuleCategory::Tools };
};
