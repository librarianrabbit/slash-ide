#include "modifylinesdialog.h"
#include "ui_modifylinesdialog.h"

ModifyLinesDialog::ModifyLinesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyLinesDialog)
{
    ui->setupUi(this);
}

ModifyLinesDialog::~ModifyLinesDialog()
{
    delete ui;
}

QString ModifyLinesDialog::prefix()
{
    return ui->lnPrefix->text();
}

QString ModifyLinesDialog::append()
{
    return ui->lnAppend->text();
}
