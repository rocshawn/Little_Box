#include "SystemCommandService.h"

#include <QFileInfo>
#include <QProcess>
#include <QStringList>

namespace {
QString shutdownExecutablePath() {
    static const QString kShutdownExecutable = QStringLiteral("C:/Windows/System32/shutdown.exe");
    if (QFileInfo::exists(kShutdownExecutable)) {
        return kShutdownExecutable;
    }

    // Fallback for unusual environments where absolute path is unavailable.
    return QStringLiteral("shutdown");
}

bool executeShutdownCommand(const QStringList& arguments, QString* errorMessage) {
    QProcess process;
    process.start(shutdownExecutablePath(), arguments);

    if (!process.waitForStarted()) {
        if (errorMessage != nullptr) {
            *errorMessage = process.errorString();
        }
        return false;
    }

    if (!process.waitForFinished()) {
        if (errorMessage != nullptr) {
            *errorMessage = process.errorString();
        }
        return false;
    }

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    const QString stdError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    const QString stdOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    QString detail = stdError;
    if (detail.isEmpty()) {
        detail = stdOutput;
    }
    if (detail.isEmpty()) {
        detail = QString("命令退出码：%1").arg(process.exitCode());
    }

    if (errorMessage != nullptr) {
        *errorMessage = detail;
    }
    return false;
}
} // namespace

bool SystemCommandService::scheduleShutdown(const int seconds, QString* errorMessage) {
    return executeShutdownCommand({ "-s", "-t", QString::number(seconds) }, errorMessage);
}

bool SystemCommandService::cancelShutdown(QString* errorMessage) {
    return executeShutdownCommand({ "-a" }, errorMessage);
}
