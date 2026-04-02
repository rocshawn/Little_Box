#pragma once

#include <QDialog>

class QLineEdit;
class QRadioButton;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

private slots:
    void onLightModeSelected();
    void onDarkModeSelected();
    void onSystemModeSelected();
    void onClearWebsite();

private:
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveWebsite(const QString& url);
    QString buildStyleSheet(bool dark) const;

    QLineEdit* websiteEdit_{ nullptr };
    QRadioButton* lightRadio_{ nullptr };
    QRadioButton* darkRadio_{ nullptr };
    QRadioButton* systemRadio_{ nullptr };
};
