#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QColor>

enum class WatermarkStyle {
    Corner, // 角落水印
    Tiled   // 满屏水印
};

enum class CornerPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

struct WatermarkConfig {
    WatermarkStyle style{ WatermarkStyle::Corner };
    QString text;
    double opacity{ 0.5 };
    int fontSize{ 40 };
    QColor color{ Qt::white };
    
    // Corner specific
    CornerPosition position{ CornerPosition::BottomRight };
    
    // Tiled specific
    int angle{ 45 };
    int spacing{ 100 }; // 密度，间距越大密度越小
};

class WatermarkModel final : public QObject {
    Q_OBJECT

public:
    explicit WatermarkModel(QObject* parent = nullptr);
    ~WatermarkModel() override = default;

    bool loadImage(const QString& filePath);
    QImage originalImage() const { return originalImage_; }
    
    // 重构后的应用方法
    QImage applyWatermark(const WatermarkConfig& config);

    bool saveImage(const QImage& image, const QString& filePath);
    bool hasImage() const { return !originalImage_.isNull(); }

signals:
    void imageLoaded();

private:
    void drawCornerWatermark(QPainter& painter, const QImage& img, const WatermarkConfig& config);
    void drawTiledWatermark(QPainter& painter, const QImage& img, const WatermarkConfig& config);

    QImage originalImage_;
};
