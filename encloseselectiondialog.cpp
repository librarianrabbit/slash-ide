#include "encloseselectiondialog.h"
#include "ui_encloseselectiondialog.h"

EncloseSelectionDialog::EncloseSelectionDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::EncloseSelectionDialog)
{
    ui->setupUi(this);
}

EncloseSelectionDialog::~EncloseSelectionDialog()
{
    delete ui;
}

QString EncloseSelectionDialog::before()
{
    return ui->lnBefore->text();
}

QString EncloseSelectionDialog::after()
{
    return ui->lnAfter->text();
}
