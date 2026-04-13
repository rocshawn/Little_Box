#pragma once

#include <QMainWindow>
#include <QImage>
#include <QComboBox>

class WatermarkModel;
class QLabel;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class QScrollArea;
class QFrame;
struct WatermarkConfig;

class WatermarkWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit WatermarkWindow(QWidget* parent = nullptr);
    ~WatermarkWindow() override = default;

private slots:
    void onSelectImage();
    void onDownloadImage();
    void updatePreview();
    void onStyleChanged(int index);

private:
    void setupUi();
    void applyTheme();
    
    // 界面分组辅助方法
    QFrame* createActionCard(const QString& title, const QString& icon, QWidget* content);

    WatermarkModel* model_{ nullptr };
    QImage currentOutputImage_;

    // UI Elements
    QComboBox* styleCombo_{ nullptr };
    QLineEdit* watermarkInput_{ nullptr };
    QDoubleSpinBox* opacitySpin_{ nullptr };
    QSpinBox* fontSizeSpin_{ nullptr };
    
    // Style Specific Controls
    QWidget* cornerControls_{ nullptr };
    QComboBox* positionCombo_{ nullptr };
    
    QWidget* tiledControls_{ nullptr };
    QSpinBox* angleSpin_{ nullptr };
    QSpinBox* spacingSpin_{ nullptr };

    QLabel* previewLabel_{ nullptr };
    QPushButton* selectBtn_{ nullptr };
    QPushButton* downloadBtn_{ nullptr };
    QScrollArea* scrollArea_{ nullptr };
};
