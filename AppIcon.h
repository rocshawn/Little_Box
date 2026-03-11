#pragma once

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>
#include <QRectF>

inline QIcon createAppIcon() {
    QIcon icon;
    const QList<int> sizes{ 16, 24, 32, 48, 64, 128, 256 };

    for (const int size : sizes) {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        const QRectF rect(1.0, 1.0, size - 2.0, size - 2.0);
        QLinearGradient background(0.0, 0.0, size, size);
        background.setColorAt(0.0, QColor("#6366F1"));
        background.setColorAt(1.0, QColor("#7C3AED"));

        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawRoundedRect(rect, size * 0.24, size * 0.24);

        painter.setBrush(QColor(255, 255, 255, 38));
        painter.drawEllipse(QRectF(size * 0.16, size * 0.10, size * 0.52, size * 0.34));

        QFont font(QStringLiteral("Segoe UI"));
        font.setBold(true);
        font.setPixelSize(size >= 128 ? 88 : (size >= 64 ? 42 : (size >= 32 ? 20 : 11)));
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignCenter, size >= 24 ? QStringLiteral("LB") : QStringLiteral("L"));

        icon.addPixmap(pixmap);
    }

    return icon;
}
