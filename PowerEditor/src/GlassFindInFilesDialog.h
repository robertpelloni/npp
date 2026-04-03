#ifndef GLASS_FIND_IN_FILES_DIALOG_H
#define GLASS_FIND_IN_FILES_DIALOG_H

#include "LiquidGlass.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMouseEvent>

class GlassFindInFilesDialog : public QDialog {
public:
    explicit GlassFindInFilesDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Find in Files");
        setModal(false);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);
        setFixedSize(540, 260);

        auto* grid = new QGridLayout(this);
        grid->setContentsMargins(20, 20, 20, 20);
        grid->setSpacing(10);

        // Title bar
        auto* titleBar = new QWidget(this);
        auto* titleLay = new QHBoxLayout(titleBar);
        titleLay->setContentsMargins(0,0,0,0);
        auto* titleLabel = new QLabel("🔍 Find in Files", titleBar);
        titleLabel->setStyleSheet("color: rgba(240,245,255,240); font-size:14px; font-weight:600;");
        auto* closeBtn = new QPushButton("✕", titleBar);
        closeBtn->setFixedSize(24, 24);
        closeBtn->setStyleSheet("QPushButton{background:rgba(255,60,60,160);color:white;border:none;border-radius:12px;font-size:12px;} QPushButton:hover{background:rgba(255,80,80,220);}");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
        titleLay->addWidget(titleLabel);
        titleLay->addStretch();
        titleLay->addWidget(closeBtn);
        grid->addWidget(titleBar, 0, 0, 1, 3);

        // Find What
        grid->addWidget(new QLabel("Find what:", this), 1, 0);
        m_findEdit = new QLineEdit(this);
        grid->addWidget(m_findEdit, 1, 1, 1, 2);

        // Filters
        grid->addWidget(new QLabel("Filters:", this), 2, 0);
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setText("*.*");
        grid->addWidget(m_filterEdit, 2, 1, 1, 2);

        // Directory
        grid->addWidget(new QLabel("Directory:", this), 3, 0);
        m_dirEdit = new QLineEdit(this);
        auto* dirLay = new QHBoxLayout;
        dirLay->setContentsMargins(0,0,0,0);
        dirLay->addWidget(m_dirEdit);
        auto* browseBtn = new QPushButton("...", this);
        browseBtn->setFixedSize(30, 24);
        connect(browseBtn, &QPushButton::clicked, this, [this](){
            QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", m_dirEdit->text());
            if (!dir.isEmpty()) m_dirEdit->setText(dir);
        });
        dirLay->addWidget(browseBtn);
        grid->addLayout(dirLay, 3, 1, 1, 2);

        // Options
        m_caseCheck  = new QCheckBox("Match case", this);
        m_wholeCheck = new QCheckBox("Whole word", this);
        m_subDirCheck = new QCheckBox("In all sub-folders", this);
        m_subDirCheck->setChecked(true);
        auto* optLay = new QHBoxLayout;
        optLay->addWidget(m_caseCheck);
        optLay->addWidget(m_wholeCheck);
        optLay->addWidget(m_subDirCheck);
        optLay->addStretch();
        grid->addLayout(optLay, 4, 0, 1, 3);

        // Buttons
        m_findAllBtn = new QPushButton("Find All", this);
        auto* btnLay = new QHBoxLayout;
        btnLay->addStretch();
        btnLay->addWidget(m_findAllBtn);
        grid->addLayout(btnLay, 5, 0, 1, 3);

        connect(m_findAllBtn, &QPushButton::clicked, this, [this](){
            if (m_findCb) m_findCb();
        });

        installEventFilter(this);
    }

    QString findText() const { return m_findEdit->text(); }
    QString filterText() const { return m_filterEdit->text(); }
    QString directory() const { return m_dirEdit->text(); }
    bool matchCase() const { return m_caseCheck->isChecked(); }
    bool wholeWord() const { return m_wholeCheck->isChecked(); }
    bool searchSubDirs() const { return m_subDirCheck->isChecked(); }

    void setFindCallback(std::function<void()> cb) { m_findCb = cb; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 14.0, true);
    }
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) move(e->globalPosition().toPoint() - m_dragPos);
        e->accept();
    }

private:
    QLineEdit* m_findEdit;
    QLineEdit* m_filterEdit;
    QLineEdit* m_dirEdit;
    QCheckBox* m_caseCheck;
    QCheckBox* m_wholeCheck;
    QCheckBox* m_subDirCheck;
    QPushButton* m_findAllBtn;
    QPoint m_dragPos;
    std::function<void()> m_findCb;
};

#endif // GLASS_FIND_IN_FILES_DIALOG_H
