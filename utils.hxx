#ifndef UTILS_HXX
#define UTILS_HXX

#include <QWidget>

template <class T>
uint disableWidgets(QWidget* parent, uint depth = 3, uint start = 0)
{
    uint count = 0;

    if (!parent || start > depth)
    {
        return count;
    }

    if (parent)
    {
        QObjectList list = parent->children();

        for (int i = 0; i < list.length(); ++i)
        {
            QObject* obj = list[i];
            QWidget* child = qobject_cast<T>(obj);

            if (child)
            {
                child->setDisabled(true);
                ++count;
            }
            else
            {
                count += disableWidgets<T>(static_cast<QWidget*>(obj), depth, (start + 1));
            }
        }
    }

    return count;
}

#endif // UTILS_HXX
