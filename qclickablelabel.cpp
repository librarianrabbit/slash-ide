#include "qclickablelabel.h"
#include <QMouseEvent>

QClickableLabel::QClickableLabel(QWidget *parent) : QLabel(parent)
{
}

QClickableLabel::~QClickableLabel()
{
}

void QClickableLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit doubleClicked();
    }
}
