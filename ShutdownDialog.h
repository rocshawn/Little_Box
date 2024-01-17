#pragma once
#ifndef SHUTDOWNDIALOG_H
#define SHUTDOWNDIALOG_H

#include <QDialog>

class QLineEdit;

class ShutdownDialog : public QDialog {
    Q_OBJECT

public:
    ShutdownDialog(QWidget* parent = nullptr);
    ~ShutdownDialog();

private:
    QLineEdit* timeEdit;

private slots:
    void onConfirmClicked();
    void reject();
};

#endif // SHUTDOWNDIALOG_H
