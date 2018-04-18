#ifndef QCLICKABLELABEL_H
#define QCLICKABLELABEL_H

#include <QLabel>

class QClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit QClickableLabel(QWidget* parent = 0);
    virtual ~QClickableLabel();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event);

signals:
    void doubleClicked() const;
};

#endif // QCLICKABLELABEL_H
