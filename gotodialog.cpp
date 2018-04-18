#include "gotodialog.h"
#include "ui_gotodialog.h"
#include <limits>

GotoDialog::GotoDialog(QWidget* parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    ui(new Ui::GotoDialog)
{    
    ui->setupUi(this);

    ui->lnLine->setMinimum(1);
    ui->lnLine->setMaximum(INT_MAX);

    ui->lnIndex->setMinimum(1);
    ui->lnIndex->setMaximum(INT_MAX);
}

GotoDialog::~GotoDialog()
{
    delete ui;
}

void GotoDialog::setMaximumLine(int max)
{
    ui->lnLine->setMaximum(max);
}

void GotoDialog::setMaximumIndex(int max)
{
    ui->lnIndex->setMaximum(max);
}

int GotoDialog::line()
{
    return ui->lnLine->value();
}

int GotoDialog::index()
{
    return ui->lnIndex->value();
}

int GotoDialog::exec()
{
    ui->lnLine->setFocus();
    ui->lnLine->selectAll();

    return QDialog::exec();
}
