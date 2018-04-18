#include "finder.h"
#include "ui_finder.h"
#include <QKeyEvent>
#include <QTimer>
#include "utils.h"

Finder::Finder(QWidget* parent) :
    QWidget(parent),
    replaceNext(false),
    findChanged(false),
    ui(new Ui::Finder)
{
    ui->setupUi(this);
    this->setupInterface();
}

Finder::~Finder()
{
    delete ui;
}

QString Finder::findText()
{
    QString text = ui->lnFind->text();

    if (ui->cbEscape->isChecked())
    {
        text = unespace(text);
    }

    return text;
}

QString Finder::replaceText()
{
    return ui->lnReplace->text();
}

bool Finder::selectToFind()
{
    return (replaceNext == false && ui->cbSelectToFind->isChecked());
}

bool Finder::regex()
{
    return ui->cbRegex->isChecked();
}

bool Finder::caseSensitive()
{
    return ui->cbCaseSensitive->isChecked();
}

bool Finder::wholeWord()
{
    return ui->cbWholeWord->isChecked();
}

bool Finder::wrap()
{
    return !ui->cbDontWrap->isChecked();
}

bool Finder::escape()
{
    return ui->cbEscape->isChecked();
}

void Finder::setFindText(const QString& text)
{
    ui->lnFind->setText(text);
}

void Finder::setSelectToFind(bool select)
{
    ui->cbSelectToFind->setChecked(select);
}

void Finder::selectFindText()
{
    ui->lnFind->selectAll();
}

void Finder::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        this->hide();
    }
}

void Finder::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    emit visibilityChanged(this->isVisible());
}

void Finder::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    emit visibilityChanged(this->isVisible());
}

void Finder::setupInterface()
{
    connect(ui->lnFind, &QLineEdit::returnPressed, this, &Finder::emitFindNotification);
    connect(ui->lnFind, &QLineEdit::textChanged, this, &Finder::updateFindChanged);
    connect(ui->btnFindPrev, &QToolButton::clicked, this, &Finder::emitFindPrevNotification);
    connect(ui->btnFindNext, &QToolButton::clicked, this, &Finder::emitFindNextNotification);
    connect(ui->lnReplace, &QLineEdit::returnPressed, this, &Finder::emitReplaceNotification);
    connect(ui->btnReplace, &QToolButton::clicked, this, &Finder::emitReplaceNotification);
    connect(ui->btnInSelection, &QToolButton::clicked, this, &Finder::emitReplaceInSelNotification);
    connect(ui->btnReplaceAll, &QToolButton::clicked, this, &Finder::emitReplaceAllNotification);

    this->setFocusProxy(ui->lnFind);
}

void Finder::emitFindNotification()
{
    replaceNext = false;
    findChanged = false;
    emit findNotification();
}

void Finder::emitFindPrevNotification()
{
    if (!replaceNext)
    {
        emit findPrevNotification();
    }
    else
    {
        this->emitReplacePrevNotification();
    }
}

void Finder::emitFindNextNotification()
{
    if (!replaceNext)
    {
        if (findChanged)
        {
            this->emitFindNotification();
        }
        else
        {
            emit findNextNotification();
        }
    }
    else
    {
        this->emitReplaceNextNotification();
    }
}

void Finder::emitReplaceNotification()
{
    replaceNext = true;
    findChanged = false;
    emit replaceNotification();
}

void Finder::emitReplacePrevNotification()
{
    emit replacePrevNotification();
}

void Finder::emitReplaceNextNotification()
{
    emit replaceNextNotification();
}

void Finder::emitReplaceInSelNotification()
{
    emit replaceInSelNotification();
}

void Finder::emitReplaceAllNotification()
{
    replaceNext = true;
    findChanged = false;
    emit replaceAllNotification();
}

void Finder::updateFindChanged()
{
    findChanged = true;
}
