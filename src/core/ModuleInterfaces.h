#pragma once

#include <QJsonObject>

/**
 * @brief Interface for modules that support pause and resume functionality.
 */
class IPausableModule {
public:
    virtual ~IPausableModule() = default;

    /**
     * @brief Pauses the module's execution (e.g., stops timers, freezes logic).
     */
    virtual void pause() = 0;

    /**
     * @brief Resumes the module's execution.
     */
    virtual void resume() = 0;
};

/**
 * @brief Interface for modules that need to save and load state.
 * Supports both Session (in-progress game) and History (best scores).
 */
class IStatefulModule {
public:
    virtual ~IStatefulModule() = default;

    /**
     * @brief Whether the module supports saving mid-progress sessions.
     * Return false for modules that only need history tracking.
     */
    virtual bool supportsSession() const { return true; }

    /**
     * @brief Serialize the current session state.
     */
    virtual QJsonObject saveSession() const { return QJsonObject(); }

    /**
     * @brief Restore the session state from data.
     */
    virtual void restoreSession(const QJsonObject& data) {}

    /**
     * @brief Serialize the historical records (e.g., best score).
     */
    virtual QJsonObject saveHistory() const { return QJsonObject(); }

    /**
     * @brief Restore the historical records from data.
     */
    virtual void restoreHistory(const QJsonObject& data) {}
};
