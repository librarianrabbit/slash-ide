#ifndef ENCLOSESELECTIONDIALOG_H
#define ENCLOSESELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class EncloseSelectionDialog;
}

class EncloseSelectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EncloseSelectionDialog(QWidget* parent = 0);
    ~EncloseSelectionDialog();

    QString before();
    QString after();
    
private:
    Ui::EncloseSelectionDialog* ui;
};

#endif // ENCLOSESELECTIONDIALOG_H
