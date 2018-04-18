#ifndef MODIFYLINESDIALOG_H
#define MODIFYLINESDIALOG_H

#include <QDialog>

namespace Ui {
    class ModifyLinesDialog;
}

class ModifyLinesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ModifyLinesDialog(QWidget *parent = 0);
    ~ModifyLinesDialog();
    
    QString prefix();
    QString append();

private:
    Ui::ModifyLinesDialog *ui;
};

#endif // MODIFYLINESDIALOG_H
