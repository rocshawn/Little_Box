#pragma once

#include <QString>

class SystemCommandService final {
public:
    static bool scheduleShutdown(int seconds, QString* errorMessage = nullptr);
    static bool cancelShutdown(QString* errorMessage = nullptr);
};
