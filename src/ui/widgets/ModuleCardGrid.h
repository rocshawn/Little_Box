#pragma once

#include "../../core/ModuleInfo.h"

#include <QList>
#include <QWidget>

class QVBoxLayout;

class ModuleCardGrid final : public QWidget {
    Q_OBJECT

public:
    explicit ModuleCardGrid(QWidget* parent = nullptr);

    void setModules(const QList<ModuleInfo>& modules);

signals:
    void moduleClicked(const QString& id);

private:
    void clearCards();

    QVBoxLayout* rootLayout_{ nullptr };
    QWidget* gridContainer_{ nullptr };
};
