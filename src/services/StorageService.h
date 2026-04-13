#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

class StorageService {
public:
    static StorageService& instance();

    // Prevent copying and assignment
    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;

    // Save and load entire session state for a module module_id
    void saveSession(const QString& moduleId, const QJsonObject& sessionData);
    QJsonObject loadSession(const QString& moduleId) const;
    void clearSession(const QString& moduleId);
    bool hasSession(const QString& moduleId) const;

    // Save and load history data for a module (e.g. high scores)
    void saveHistory(const QString& moduleId, const QJsonObject& historyData);
    bool hasHistory(const QString& moduleId) const;
    QJsonObject loadHistory(const QString& moduleId) const;

private:
    StorageService();
    ~StorageService() = default;

    void loadFromFile();
    void saveToFile() const;
    
    QString getFilePath() const;

    QJsonObject rootData_;
};
