#include "WatermarkWindow.h"
#include "../../core/ThemeManager.h"
#include "../../logic/WatermarkModel.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QSplitter>
#include <QPainter>

WatermarkWindow::WatermarkWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new WatermarkModel(this)) {
    
    setWindowFlags(Qt::Window);
    setupUi();
    applyTheme();

    connect(model_, &WatermarkModel::imageLoaded, this, &WatermarkWindow::updatePreview);
    connect(styleCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &WatermarkWindow::onStyleChanged);
    connect(watermarkInput_, &QLineEdit::textChanged, this, &WatermarkWindow::updatePreview);
    connect(opacitySpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WatermarkWindow::updatePreview);
    connect(fontSizeSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WatermarkWindow::updatePreview);
    connect(positionCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &WatermarkWindow::updatePreview);
    connect(angleSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WatermarkWindow::updatePreview);
    connect(spacingSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WatermarkWindow::updatePreview);
}

void WatermarkWindow::setupUi() {
    setWindowTitle("Pro 影像水印专家");
    resize(1200, 800);
    setMinimumSize(950, 700);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(0);

    auto* mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainSplitter->setHandleWidth(2);
    mainSplitter->setStyleSheet("QSplitter::handle { background: transparent; }");

    // ── Left Panel (Controls) ──────────────────────────────────────────
    auto* leftScroll = new QScrollArea();
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);
    leftScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroll->setFixedWidth(340);
    leftScroll->setObjectName("leftPanelScroll");

    auto* leftContent = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftContent);
    leftLayout->setContentsMargins(0, 0, 16, 0);
    leftLayout->setSpacing(20);

    auto* titleLabel = new QLabel("Watermark Pro");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: 900; color: #0d9488; letter-spacing: 1px; margin-bottom: 5px;");
    leftLayout->addWidget(titleLabel);

    selectBtn_ = new QPushButton("选择源图片 (Select Image)");
    selectBtn_->setCursor(Qt::PointingHandCursor);
    selectBtn_->setFixedHeight(52);
    selectBtn_->setObjectName("primaryBtn");
    leftLayout->addWidget(selectBtn_);

    // 1. Basic Settings Card
    auto* basicForm = new QWidget();
    auto* basicLayout = new QFormLayout(basicForm);
    basicLayout->setSpacing(14);
    
    styleCombo_ = new QComboBox();
    styleCombo_->addItem("📍 方位水印", (int)WatermarkStyle::Corner);
    styleCombo_->addItem("🔳 满屏平铺", (int)WatermarkStyle::Tiled);
    styleCombo_->setFixedHeight(38);
    basicLayout->addRow("展现样式", styleCombo_);

    watermarkInput_ = new QLineEdit();
    watermarkInput_->setPlaceholderText("在这里输入水印内容...");
    watermarkInput_->setText("Little Box Studio");
    watermarkInput_->setFixedHeight(38);
    basicLayout->addRow("文字内容", watermarkInput_);
    
    leftLayout->addWidget(createActionCard("基础设置", "⚙️", basicForm));

    // 2. Visual Style Card
    auto* visualForm = new QWidget();
    auto* visualLayout = new QFormLayout(visualForm);
    visualLayout->setSpacing(14);

    fontSizeSpin_ = new QSpinBox();
    fontSizeSpin_->setRange(8, 2000);
    fontSizeSpin_->setValue(72);
    fontSizeSpin_->setFixedHeight(38);
    visualLayout->addRow("字体大小", fontSizeSpin_);

    opacitySpin_ = new QDoubleSpinBox();
    opacitySpin_->setRange(0.0, 1.0);
    opacitySpin_->setSingleStep(0.05);
    opacitySpin_->setValue(0.4);
    opacitySpin_->setFixedHeight(38);
    visualLayout->addRow("透明色阶", opacitySpin_);

    leftLayout->addWidget(createActionCard("视觉风格", "🎨", visualForm));

    // 3. Arrangement Card (Dynamic)
    auto* arrangeForm = new QWidget();
    auto* arrangeLayout = new QVBoxLayout(arrangeForm);
    arrangeLayout->setContentsMargins(0, 0, 0, 0);
    arrangeLayout->setSpacing(14);

    cornerControls_ = new QWidget();
    auto* cornerForm = new QFormLayout(cornerControls_);
    cornerForm->setContentsMargins(0, 0, 0, 0);
    positionCombo_ = new QComboBox();
    positionCombo_->addItem("左上角", (int)CornerPosition::TopLeft);
    positionCombo_->addItem("右上角", (int)CornerPosition::TopRight);
    positionCombo_->addItem("左下角", (int)CornerPosition::BottomLeft);
    positionCombo_->addItem("右下角", (int)CornerPosition::BottomRight);
    positionCombo_->setCurrentIndex(3);
    positionCombo_->setFixedHeight(38);
    cornerForm->addRow("锚点位置", positionCombo_);
    arrangeLayout->addWidget(cornerControls_);

    tiledControls_ = new QWidget();
    auto* tiledForm = new QFormLayout(tiledControls_);
    tiledForm->setContentsMargins(0, 0, 0, 0);
    angleSpin_ = new QSpinBox();
    angleSpin_->setRange(-180, 180);
    angleSpin_->setValue(-45);
    angleSpin_->setFixedHeight(38);
    tiledForm->addRow("螺旋角度", angleSpin_);

    spacingSpin_ = new QSpinBox();
    spacingSpin_->setRange(0, 2000);
    spacingSpin_->setValue(150);
    spacingSpin_->setFixedHeight(38);
    tiledForm->addRow("平铺间距", spacingSpin_);
    arrangeLayout->addWidget(tiledControls_);
    tiledControls_->hide();

    leftLayout->addWidget(createActionCard("排版布局", "🧩", arrangeForm));

    leftLayout->addStretch();

    downloadBtn_ = new QPushButton("保存高清成品 (Export)");
    downloadBtn_->setCursor(Qt::PointingHandCursor);
    downloadBtn_->setFixedHeight(56);
    downloadBtn_->setObjectName("exportBtn");
    downloadBtn_->setEnabled(false);
    leftLayout->addWidget(downloadBtn_);

    leftScroll->setWidget(leftContent);
    mainSplitter->addWidget(leftScroll);

    // ── Right Panel (Preview) ──────────────────────────────────────────
    auto* rightContainer = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(16, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto* previewHeader = new QHBoxLayout();
    auto* previewTitle = new QLabel("实时画布 (Canvas Preview)");
    previewTitle->setStyleSheet("font-weight: 800; color: #64748b; font-size: 14px;");
    previewHeader->addWidget(previewTitle);
    previewHeader->addStretch();
    rightLayout->addLayout(previewHeader);

    auto* previewFrame = new QFrame();
    previewFrame->setObjectName("previewFrame");
    auto* previewFrameLayout = new QVBoxLayout(previewFrame);
    previewFrameLayout->setContentsMargins(1, 1, 1, 1);

    previewLabel_ = new QLabel("正在等待图像载入...");
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet("color: #94a3b8; font-size: 15px; background: transparent;");
    previewLabel_->setWordWrap(true);

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setAlignment(Qt::AlignCenter);
    scrollArea_->setWidget(previewLabel_);
    scrollArea_->setObjectName("previewScroll");
    
    // 给预览区增加一个微妙的网格背景
    scrollArea_->viewport()->setStyleSheet("background-image: url(:/res/checker.png); background-repeat: repeat;");

    previewFrameLayout->addWidget(scrollArea_);
    rightLayout->addWidget(previewFrame, 1);

    mainSplitter->addWidget(rightContainer);
    mainSplitter->setStretchFactor(1, 1); // 预览区占据剩余空间

    rootLayout->addWidget(mainSplitter);

    connect(selectBtn_, &QPushButton::clicked, this, &WatermarkWindow::onSelectImage);
    connect(downloadBtn_, &QPushButton::clicked, this, &WatermarkWindow::onDownloadImage);
}

QFrame* WatermarkWindow::createActionCard(const QString& title, const QString& icon, QWidget* content) {
    auto* card = new QFrame();
    card->setObjectName("actionCard");
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 20);
    layout->setSpacing(15);

    auto* header = new QHBoxLayout();
    auto* iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet("font-size: 18px;");
    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: 800; color: #1e293b; font-size: 15px;");
    
    header->addWidget(iconLabel);
    header->addWidget(titleLabel);
    header->addStretch();
    
    layout->addLayout(header);
    layout->addWidget(content);

    // Shadow
    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 15));
    card->setGraphicsEffect(shadow);

    return card;
}

void WatermarkWindow::applyTheme() {
    const bool dark = ThemeManager::instance().isDark();
    
    QString baseStyle = dark ? 
        "QMainWindow { background: #020617; }"
        "QFrame#actionCard { background: #0f172a; border-radius: 20px; border: 1px solid #1e293b; }"
        "QFrame#previewFrame { background: #0f172a; border-radius: 24px; border: 1px solid #1e293b; }"
        "QLineEdit, QDoubleSpinBox, QSpinBox, QComboBox { background: #1e293b; color: #f1f5f9; border: 1px solid #334155; border-radius: 12px; padding: 0 12px; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 42px; border-left: 1px solid #334155; border-top-right-radius: 12px; border-bottom-right-radius: 12px; background: #1a253a; }"
        "QComboBox::drop-down:hover { background: #1e293b; }"
        "QComboBox::down-arrow { image: none; border-left: 9px solid transparent; border-right: 9px solid transparent; border-top: 9px solid #06b6d4; }"
        "QComboBox::down-arrow:hover { border-top-color: #2dd4bf; }"
        "QComboBox QAbstractItemView { background: #1e293b; color: #f1f5f9; border: 1px solid #334155; border-radius: 12px; selection-background-color: #0d9488; outline: none; }"
        "QLabel { color: #94a3b8; }"
        "QScrollArea#previewScroll { background: #020617; border: none; border-radius: 22px; }"
        :
        "QMainWindow { background: #f8fafc; }"
        "QFrame#actionCard { background: #ffffff; border-radius: 20px; border: 1px solid #e2e8f0; }"
        "QFrame#previewFrame { background: #ffffff; border-radius: 24px; border: 1px solid #e2e8f0; }"
        "QLineEdit, QDoubleSpinBox, QSpinBox, QComboBox { background: #f8fafc; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 12px; padding: 0 12px; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 42px; border-left: 1px solid #cbd5e1; border-top-right-radius: 12px; border-bottom-right-radius: 12px; background: #f1f5f9; }"
        "QComboBox::drop-down:hover { background: #e2e8f0; }"
        "QComboBox::down-arrow { image: none; border-left: 9px solid transparent; border-right: 9px solid transparent; border-top: 9px solid #0d9488; }"
        "QComboBox::down-arrow:hover { border-top-color: #0f766e; }"
        "QComboBox QAbstractItemView { background: white; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 12px; selection-background-color: #0d9488; selection-color: white; outline: none; }"
        "QLabel { color: #475569; }"
        "QScrollArea#previewScroll { background: #f1f5f9; border: none; border-radius: 22px; }";

    QString btnStyle = "QPushButton#primaryBtn { background: #f1f5f9; color: #0d9488; border: 2px dashed #cbd5e1; border-radius: 16px; font-weight: 800; }"
                       "QPushButton#primaryBtn:hover { background: #f0fdfa; border-color: #0d9488; }"
                       "QPushButton#exportBtn { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #06b6d4, stop:1 #0d9488); color: white; border-radius: 18px; font-weight: 900; font-size: 15px; border: none; }"
                       "QPushButton#exportBtn:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0891b2, stop:1 #0f766e); }"
                       "QPushButton#exportBtn:disabled { background: #e2e8f0; color: #94a3b8; }";

    setStyleSheet(baseStyle + btnStyle);
}

void WatermarkWindow::onStyleChanged(int index) {
    WatermarkStyle style = (WatermarkStyle)styleCombo_->itemData(index).toInt();
    cornerControls_->setVisible(style == WatermarkStyle::Corner);
    tiledControls_->setVisible(style == WatermarkStyle::Tiled);
    updatePreview();
}

void WatermarkWindow::onSelectImage() {
    QString filePath = QFileDialog::getOpenFileName(this, "载入原始影像", "", "图像文件 (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!filePath.isEmpty()) {
        if (model_->loadImage(filePath)) {
            downloadBtn_->setEnabled(true);
        } else {
            QMessageBox::critical(this, "解析失败", "无法读取该文件，请检查文件权限或格式是否正确。");
        }
    }
}

void WatermarkWindow::onDownloadImage() {
    if (currentOutputImage_.isNull()) return;

    QString filePath = QFileDialog::getSaveFileName(this, "导出水印成品", "LB_Watermark_Result.png", "PNG 高清图 (*.png);;JPG 压缩图 (*.jpg)");
    if (!filePath.isEmpty()) {
        if (model_->saveImage(currentOutputImage_, filePath)) {
            QMessageBox::information(this, "导出成功", "照片已成功添加水印并保存至本地。");
        }
    }
}

void WatermarkWindow::updatePreview() {
    if (!model_->hasImage()) return;

    WatermarkConfig config;
    config.style = (WatermarkStyle)styleCombo_->currentData().toInt();
    config.text = watermarkInput_->text();
    config.opacity = opacitySpin_->value();
    config.fontSize = fontSizeSpin_->value();
    
    if (config.style == WatermarkStyle::Corner) {
        config.position = (CornerPosition)positionCombo_->currentData().toInt();
    } else {
        config.angle = angleSpin_->value();
        config.spacing = spacingSpin_->value();
    }

    currentOutputImage_ = model_->applyWatermark(config);

    if (!currentOutputImage_.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(currentOutputImage_);
        int w = scrollArea_->width() - 40;
        int h = scrollArea_->height() - 40;
        if (w > 0 && h > 0) {
            previewLabel_->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            previewLabel_->setText(""); 
        }
    }
}
