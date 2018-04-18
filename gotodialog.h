#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

namespace Ui {
    class GotoDialog;
}

class GotoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GotoDialog(QWidget* parent = 0);
    ~GotoDialog();
    
    void setMaximumLine(int max);
    void setMaximumIndex(int max);

    int line();
    int index();

public slots:
    virtual int exec();

private:
    Ui::GotoDialog *ui;
};

#endif // GOTODIALOG_H
