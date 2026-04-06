#pragma once

#include <QDialog>
#include "../../core/ThemeManager.h"

class QLineEdit;
class QPushButton;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

private:
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveWebsite(const QString& url);
    void onThemeModeSelected(ThemeMode mode);
    void onToggleWebsiteVisibility();
    void onClearWebsite();
    QString buildStyleSheet(bool dark) const;

    QLineEdit* websiteEdit_{ nullptr };
    QPushButton* toggleShowBtn_{ nullptr };
    QPushButton* lightBtn_{ nullptr };
    QPushButton* darkBtn_{ nullptr };
    QPushButton* systemBtn_{ nullptr };
};

