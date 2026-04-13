#include "WatermarkModel.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

WatermarkModel::WatermarkModel(QObject* parent)
    : QObject(parent) {
}

bool WatermarkModel::loadImage(const QString& filePath) {
    QImage img;
    if (img.load(filePath)) {
        originalImage_ = std::move(img);
        emit imageLoaded();
        return true;
    }
    return false;
}

QImage WatermarkModel::applyWatermark(const WatermarkConfig& config) {
    if (originalImage_.isNull()) {
        return QImage();
    }

    QImage result = originalImage_.copy();
    QPainter painter(&result);
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font("Microsoft YaHei", config.fontSize);
    font.setBold(true);
    painter.setFont(font);

    QColor markColor = config.color;
    markColor.setAlphaF(config.opacity);
    painter.setPen(markColor);

    if (config.style == WatermarkStyle::Corner) {
        drawCornerWatermark(painter, result, config);
    } else {
        drawTiledWatermark(painter, result, config);
    }
    
    painter.end();
    return result;
}

void WatermarkModel::drawCornerWatermark(QPainter& painter, const QImage& img, const WatermarkConfig& config) {
    QFontMetrics metrics(painter.font());
    int textWidth = metrics.horizontalAdvance(config.text);
    int textHeight = metrics.height();
    int margin = 30;

    int x = 0;
    int y = 0;

    switch (config.position) {
    case CornerPosition::TopLeft:
        x = margin;
        y = margin + textHeight;
        break;
    case CornerPosition::TopRight:
        x = img.width() - textWidth - margin;
        y = margin + textHeight;
        break;
    case CornerPosition::BottomLeft:
        x = margin;
        y = img.height() - margin;
        break;
    case CornerPosition::BottomRight:
        x = img.width() - textWidth - margin;
        y = img.height() - margin;
        break;
    }

    painter.drawText(x, y, config.text);
}

void WatermarkModel::drawTiledWatermark(QPainter& painter, const QImage& img, const WatermarkConfig& config) {
    QFontMetrics metrics(painter.font());
    int textWidth = metrics.horizontalAdvance(config.text);
    int textHeight = metrics.height();

    // 估算绘制范围，因为旋转后可能需要更大的区域
    int diag = std::sqrt(img.width() * img.width() + img.height() * img.height());
    
    // 平铺步长 = 文字大小 + 间距
    int stepX = textWidth + config.spacing;
    int stepY = textHeight + config.spacing;

    // 移动原点到图片中心进行旋转
    painter.save();
    painter.translate(img.width() / 2, img.height() / 2);
    painter.rotate(config.angle);
    
    // 以中心为原点，向四周平铺绘制
    for (int y = -diag; y < diag; y += stepY) {
        for (int x = -diag; x < diag; x += stepX) {
            painter.drawText(x, y, config.text);
        }
    }
    
    painter.restore();
}

bool WatermarkModel::saveImage(const QImage& image, const QString& filePath) {
    if (image.isNull()) {
        return false;
    }
    return image.save(filePath);
}
