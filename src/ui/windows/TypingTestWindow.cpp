#include "TypingTestWindow.h"

#include "../../core/ThemeManager.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QComboBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

TypingTestWindow::TypingTestWindow(QWidget* parent)
    : QMainWindow(parent),
      textDisplay_(nullptr),
      inputEdit_(nullptr),
      wpmLabel_(nullptr),
      accuracyLabel_(nullptr),
      bestWpmLabel_(nullptr),
      timeLabel_(nullptr),
      difficultyCombo_(nullptr),
      timer_(nullptr),
      targetText_(""),
      elapsedSeconds_(0),
      isRunning_(false),
      bestWpm_(0.0),
      currentDifficulty_(Difficulty::Medium) {
    
    timer_ = new QTimer(this);
    QSettings settings;
    bestWpm_ = settings.value("typing_test/best_wpm", 0.0).toDouble();

    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &TypingTestWindow::onTick);

    setupUi();
    resetTest();
}

void TypingTestWindow::setupUi() {
    setWindowTitle("英文打字速度挑战");
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

    // ── Header (Stats)
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

    // Best Record Card
    statsLayout->addWidget(createStatCard("历史最高 WPM", bestWpmLabel_));
    bestWpmLabel_->setText(QString::number(bestWpm_, 'f', 1));
    
    auto* optionsLayout = new QVBoxLayout();
    auto* diffLabel = new QLabel("难度选择:");
        diffLabel->setStyleSheet(dark ? "color:#fda4af; font-weight:800;" : "color:#f43f5e; font-weight:800;");
    
    difficultyCombo_ = new QComboBox();
    difficultyCombo_->addItems({"简单 (短)", "中等 (适中)", "困难 (长)"});
    difficultyCombo_->setCurrentIndex(1); // Default to Medium
    difficultyCombo_->setStyleSheet(dark
        ? "QComboBox { background:#3a1c36; color:#fbcfe8; border:2px solid #5e2c56; border-radius:10px; padding:5px; }"
        : "QComboBox { background:white; color:#4c0519; border:2px solid #ffe4e6; border-radius:10px; padding:5px; }");
    
    connect(difficultyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        currentDifficulty_ = static_cast<Difficulty>(index);
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

    // ── Text Display Area 
    textDisplay_ = new QTextBrowser(this);
    textDisplay_->setMinimumHeight(120);
    textDisplay_->setStyleSheet(dark 
        ? "QTextBrowser { background:#1a0b1c; color:#94a3b8; border:2px solid #5e2c56; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; line-height: 1.5; }"
        : "QTextBrowser { background:#fff1f2; color:#94a3b8; border:2px solid #fda4af; border-radius:24px; padding:20px; font-size:20px; font-family: Consolas, monospace; line-height: 1.5; }");
    
    mainLayout->addWidget(textDisplay_);

    // ── Input Area
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

QString TypingTestWindow::generateQuote() {
    QStringList filtered;
    for (const auto& quote : kQuotes_) {
        const int len = quote.length();
        if (currentDifficulty_ == Difficulty::Short && len < 60) filtered.append(quote);
        else if (currentDifficulty_ == Difficulty::Medium && len >= 60 && len < 150) filtered.append(quote);
        else if (currentDifficulty_ == Difficulty::Long && len >= 150) filtered.append(quote);
    }
    
    // Fallback if no quote matches current filter
    if (filtered.isEmpty()) filtered = kQuotes_;

    int idx = QRandomGenerator::global()->bounded(filtered.size());
    return filtered[idx];
}

void TypingTestWindow::resetTest() {
    timer_->stop();
    isRunning_ = false;
    elapsedSeconds_ = 0;
    
    targetText_ = generateQuote();
    textDisplay_->setText(targetText_);
    
    inputEdit_->blockSignals(true);
    inputEdit_->clear();
    inputEdit_->blockSignals(false);
    
    wpmLabel_->setText("0");
    accuracyLabel_->setText("100%");
    timeLabel_->setText("0");
    
    inputEdit_->setFocus();
}

void TypingTestWindow::checkTyping() {
    QString typed = inputEdit_->toPlainText();
    
    if (!isRunning_ && !typed.isEmpty()) {
        isRunning_ = true;
        timer_->start();
    }

    const bool dark = ThemeManager::instance().isDark();
    const QString correctColor = dark ? "#f472b6" : "#f43f5e"; // Pinkish highlight for correct
    const QString wrongColor = dark ? "#ef4444" : "#dc2626"; // Red for errors
    const QString pendingColor = dark ? "#64748b" : "#94a3b8"; // Gray for untyped
    
    QString html = "<div style='line-height:1.4; white-space:pre-wrap;'>";
    int correctCount = 0;

    for (int i = 0; i < targetText_.length(); ++i) {
        QChar targetChar = targetText_[i];
        
        if (i < typed.length()) {
            QChar typedChar = typed[i];
            if (typedChar == targetChar) { 
                correctCount++;
                html += QString("<span style='color:%1; font-weight:bold;'>%2</span>")
                            .arg(correctColor)
                            .arg(targetChar == ' ' ? " " : QString(targetChar));
            } else {
                html += QString("<span style='background-color:%1; color:white; border-radius:4px;'>%2</span>")
                            .arg(wrongColor)
                            .arg(targetChar == ' ' ? "_" : QString(targetChar));
            }
        } else {
            html += QString("<span style='color:%1;'>%2</span>")
                        .arg(pendingColor)
                        .arg(targetChar == ' ' ? " " : QString(targetChar));
        }
    }
    html += "</div>";

    // Update display
    textDisplay_->setHtml(html);

    // ── Auto-scroll logic: Center current line ──────────────────────────────
    QTextCursor cursor = textDisplay_->textCursor();
    cursor.setPosition(typed.length());
    textDisplay_->setTextCursor(cursor);
    
    const QRect cursorRect = textDisplay_->cursorRect(cursor);
    const int viewportHeight = textDisplay_->viewport()->height();
    const int currentScroll = textDisplay_->verticalScrollBar()->value();
    
    // Calculate new scroll position to keep cursor vertically centered
    const int targetScroll = currentScroll + (cursorRect.top() - viewportHeight / 2);
    textDisplay_->verticalScrollBar()->setValue(targetScroll);

    // Calculate accuracy
    int totalTyped = typed.length();
    if (totalTyped > 0) {
        int acc = (correctCount * 100) / totalTyped;
        accuracyLabel_->setText(QString("%1%").arg(acc));
    }

    // Stop if reached the end
    if (typed.length() >= targetText_.length()) {
        timer_->stop();
        isRunning_ = false;
        inputEdit_->setReadOnly(true);
        updateStats();
    }
}

void TypingTestWindow::onTick() {
    elapsedSeconds_++;
    timeLabel_->setText(QString::number(elapsedSeconds_));
    updateStats();
}

void TypingTestWindow::updateStats() {
    if (elapsedSeconds_ == 0) return;
    
    QString typed = inputEdit_->toPlainText();
    // Words per minute standard formula: (chars / 5) / (seconds / 60)
    int charsTyped = typed.length();
    double wpm = (charsTyped / 5.0) / (elapsedSeconds_ / 60.0);
    
    wpmLabel_->setText(QString::number(wpm, 'f', 1));

    // Update best record if test is finished or currently better
    if (wpm > bestWpm_) {
        bestWpm_ = wpm;
        bestWpmLabel_->setText(QString::number(bestWpm_, 'f', 1));
        
        QSettings settings;
        settings.setValue("typing_test/best_wpm", bestWpm_);
    }
}
