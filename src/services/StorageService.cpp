#include "StorageService.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

StorageService& StorageService::instance() {
    static StorageService instance;
    return instance;
}

StorageService::StorageService() {
    loadFromFile();
}

QString StorageService::getFilePath() const {
    // Stores in the same directory as the executable
    return QCoreApplication::applicationDirPath() + "/userData.json";
}

void StorageService::loadFromFile() {
    QFile file(getFilePath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                rootData_ = doc.object();
            }
            file.close();
        } else {
            qWarning() << "Failed to open userData.json for reading.";
        }
    }
}

void StorageService::saveToFile() const {
    QFile file(getFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(rootData_);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        qWarning() << "Failed to open userData.json for writing.";
    }
}

void StorageService::saveSession(const QString& moduleId, const QJsonObject& sessionData) {
    QJsonObject sessionRoot = rootData_.value("session").toObject();
    sessionRoot[moduleId] = sessionData;
    rootData_["session"] = sessionRoot;
    saveToFile();
}

QJsonObject StorageService::loadSession(const QString& moduleId) const {
    QJsonObject sessionRoot = rootData_.value("session").toObject();
    return sessionRoot.value(moduleId).toObject();
}

void StorageService::clearSession(const QString& moduleId) {
    QJsonObject sessionRoot = rootData_.value("session").toObject();
    sessionRoot.remove(moduleId);
    rootData_["session"] = sessionRoot;
    saveToFile();
}

bool StorageService::hasSession(const QString& moduleId) const {
    QJsonObject sessionRoot = rootData_.value("session").toObject();
    return sessionRoot.contains(moduleId);
}

void StorageService::saveHistory(const QString& moduleId, const QJsonObject& historyData) {
    QJsonObject historyRoot = rootData_.value("history").toObject();
    historyRoot[moduleId] = historyData;
    rootData_["history"] = historyRoot;
    saveToFile();
}

bool StorageService::hasHistory(const QString& moduleId) const
{
    QJsonObject historyRoot = rootData_.value("history").toObject();
    return historyRoot.contains(moduleId);
    return false;
}

QJsonObject StorageService::loadHistory(const QString& moduleId) const {
    QJsonObject historyRoot = rootData_.value("history").toObject();
    return historyRoot.value(moduleId).toObject();
}
