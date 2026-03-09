#include "SystemCommandService.h"

#include <QProcess>
#include <QStringList>

namespace {
bool executeShutdownCommand(const QStringList& arguments) {
    const int exitCode = QProcess::execute("shutdown", arguments);
    return exitCode == 0;
}
} // namespace

bool SystemCommandService::scheduleShutdown(const int seconds) {
    return executeShutdownCommand({ "-s", "-t", QString::number(seconds) });
}

bool SystemCommandService::cancelShutdown() {
    return executeShutdownCommand({ "-a" });
}
