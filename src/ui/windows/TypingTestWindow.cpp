#include "TypingTestWindow.h"

#include "../../core/ThemeManager.h"
#include "../../logic/TypingTestModel.h"
#include "../../services/StorageService.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

TypingTestWindow::TypingTestWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new TypingTestModel(this)) {
    
    setupUi();

    connect(model_, &TypingTestModel::updated, this, &TypingTestWindow::updateUi);
    connect(model_, &TypingTestModel::finished, this, &TypingTestWindow::onFinished);
    
    if (StorageService::instance().hasHistory("typing_test")) {
        model_->restoreHistory(StorageService::instance().loadHistory("typing_test"));
    }
    
    updateUi();
}

void TypingTestWindow::setupUi() {
    setWindowTitle("英文打字速度挑战 (MVC)");
    setMinimumSize(700, 500);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    auto* statsLayout = new QHBoxLayout();
    
    auto createStatCard = [dark](const QString& title, QLabel*& valueLabel) {
        auto* card = new QFrame();
        card->setStyleSheet(dark 
            ? "QFrame { background:#3a1c36; border-radius:16px; border:2px solid #5e2c56; }"
            : "QFrame { background:white; border-radius:16px; border:2px solid #ffe4e6; }");
        auto* layout = new QVBoxLayout(card);
        
        auto* titleLbl = new QLabel(title);
        titleLbl->setStyleSheet(dark ? "color:#fbcfe8; font-size:12px; font-weight:800;" : "color:#fb7185; font-size:12px; font-weight:800;");
        titleLbl->setAlignment(Qt::AlignCenter);
        
        valueLabel = new QLabel("0");
        valueLabel->setStyleSheet(dark ? "color:#fff0f2; font-size:28px; font-weight:900;" : "color:#4c0519; font-size:28px; font-weight:900;");
        valueLabel->setAlignment(Qt::AlignCenter);
        
        layout->addWidget(titleLbl);
        layout->addWidget(valueLabel);
        return card;
    };

    statsLayout->addWidget(createStatCard("WPM (词/分)", wpmLabel_));
    statsLayout->addWidget(createStatCard("准确率", accuracyLabel_));
    statsLayout->addWidget(createStatCard("时间 (秒)", timeLabel_));
    statsLayout->addWidget(createStatCard("历史最高 WPM", bestWpmLabel_));
    
    auto* optionsLayout = new QVBoxLayout();
    auto* diffLabel = new QLabel("难度选择:");
    diffLabel->setStyleSheet(dark ? "color:#fda4af; font-weight:800;" : "color:#f43f5e; font-weight:800;");
    
    difficultyCombo_ = new QComboBox();
    difficultyCombo_->addItems({"简单 (短)", "中等 (适中)", "困难 (长)"});
    difficultyCombo_->setCurrentIndex(static_cast<int>(model_->difficulty()));
    difficultyCombo_->setStyleSheet(dark
        ? "QComboBox { background:#3a1c36; color:#fbcfe8; border:2px solid #5e2c56; border-radius:10px; padding:5px; }"
        : "QComboBox { background:white; color:#4c0519; border:2px solid #ffe4e6; border-radius:10px; padding:5px; }");
    
    connect(difficultyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        model_->setDifficulty(static_cast<TypingTestModel::Difficulty>(index));
        resetTest();
    });

    optionsLayout->addWidget(diffLabel);
    optionsLayout->addWidget(difficultyCombo_);
    statsLayout->addLayout(optionsLayout);

    auto* resetBtn = new QPushButton("重新生成片段");
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setFixedSize(140, 60);
    resetBtn->setStyleSheet(dark 
        ? "QPushButton { background:#881337; color:white; border-radius:14px; padding:8px 16px; font-size:14px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#9f1239; }"
        : "QPushButton { background:#fb7185; color:white; border-radius:14px; padding:8px 16px; font-size:14px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#f43f5e; }");
    connect(resetBtn, &QPushButton::clicked, this, &TypingTestWindow::resetTest);
    statsLayout->addWidget(resetBtn);

    mainLayout->addLayout(statsLayout);

    textDisplay_ = new QTextBrowser(this);
    textDisplay_->setMinimumHeight(120);
    textDisplay_->setStyleSheet(dark 
        ? "QTextBrowser { background:#1a0b1c; color:#94a3b8; border:2px solid #5e2c56; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; line-height: 1.5; }"
        : "QTextBrowser { background:#fff1f2; color:#94a3b8; border:2px solid #fda4af; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; line-height: 1.5; }");
    
    mainLayout->addWidget(textDisplay_);

    inputEdit_ = new QTextEdit(this);
    inputEdit_->setPlaceholderText("在这里开始敲击键盘...");
    inputEdit_->setStyleSheet(dark 
        ? "QTextEdit { background:#3a1c36; color:#fbcfe8; border:2px solid #5e2c56; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; }"
          "QTextEdit:focus { border: 2px solid #fb7185; }"
        : "QTextEdit { background:white; color:#4c0519; border:2px solid #ffe4e6; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; }"
          "QTextEdit:focus { border: 2px solid #fb7185; }");
    
    mainLayout->addWidget(inputEdit_);
    
    connect(inputEdit_, &QTextEdit::textChanged, this, &TypingTestWindow::checkTyping);
}

void TypingTestWindow::updateUi() {
    wpmLabel_->setText(QString::number(model_->wpm(), 'f', 1));
    accuracyLabel_->setText(QString("%1%").arg(model_->accuracy()));
    timeLabel_->setText(QString::number(model_->elapsedSeconds()));
    bestWpmLabel_->setText(QString::number(model_->bestWpm(), 'f', 1));

    if (inputEdit_->toPlainText().isEmpty()) {
        textDisplay_->setText(model_->targetText());
    }
}

void TypingTestWindow::onFinished() {
    inputEdit_->setReadOnly(true);
}

void TypingTestWindow::resetTest() {
    model_->reset();
    inputEdit_->setReadOnly(false);
    inputEdit_->blockSignals(true);
    inputEdit_->clear();
    inputEdit_->blockSignals(false);
    inputEdit_->setFocus();
}

void TypingTestWindow::checkTyping() {
    const QString typed = inputEdit_->toPlainText();
    model_->updateInput(typed);

    const QString targetText = model_->targetText();
    const bool dark = ThemeManager::instance().isDark();
    const QString correctColor = dark ? "#f472b6" : "#f43f5e";
    const QString wrongColor = dark ? "#ef4444" : "#dc2626";
    const QString pendingColor = dark ? "#64748b" : "#94a3b8";
    
    QString html = "<div style='line-height:1.4; white-space:pre-wrap;'>";
    for (int i = 0; i < targetText.length(); ++i) {
        const QChar targetChar = targetText.at(i);
        if (i < typed.length()) {
            if (typed.at(i) == targetChar) { 
                html += QString("<span style='color:%1; font-weight:bold;'>%2</span>").arg(correctColor).arg(targetChar == ' ' ? " " : QString(targetChar));
            } else {
                html += QString("<span style='background-color:%1; color:white; border-radius:4px;'>%2</span>").arg(wrongColor).arg(targetChar == ' ' ? "_" : QString(targetChar));
            }
        } else {
            html += QString("<span style='color:%1;'>%2</span>").arg(pendingColor).arg(targetChar == ' ' ? " " : QString(targetChar));
        }
    }
    html += "</div>";

    textDisplay_->setHtml(html);

    QTextCursor cursor = textDisplay_->textCursor();
    cursor.setPosition(static_cast<int>(typed.length()));
    textDisplay_->setTextCursor(cursor);
    
    const QRect cursorRect = textDisplay_->cursorRect(cursor);
    const int viewportHeight = textDisplay_->viewport()->height();
    const int currentScroll = textDisplay_->verticalScrollBar()->value();
    const int targetScroll = currentScroll + (cursorRect.top() - viewportHeight / 2);
    textDisplay_->verticalScrollBar()->setValue(targetScroll);
}

void TypingTestWindow::closeEvent(QCloseEvent* event) {
    StorageService::instance().saveHistory("typing_test", model_->saveHistory());
    QMainWindow::closeEvent(event);
}
