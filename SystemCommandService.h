#pragma once

class SystemCommandService final {
public:
    static bool scheduleShutdown(int seconds);
    static bool cancelShutdown();
};
