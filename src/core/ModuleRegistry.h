#pragma once

#include "ModuleInfo.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class ModuleRegistry final {
public:
    static ModuleRegistry& instance();

    void registerModule(const ModuleInfo& info);
    void registerBuiltinModules();

    QList<ModuleInfo> modulesForCategory(ModuleCategory category) const;
    void launch(const QString& id, QWidget* parent);

private:
    ModuleRegistry() = default;
    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;

    QList<ModuleInfo> modules_;
    QMap<QString, QPointer<QWidget>> openWindows_;
};
