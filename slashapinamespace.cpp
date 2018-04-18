#include "slashapinamespace.h"
#include "slashapi.h"

SlashAPINamespace::SlashAPINamespace(QObject* parent) :
    SlashAPIObject(parent)
{
}

SlashAPINamespace::~SlashAPINamespace()
{
}

QString SlashAPINamespace::fullName() const
{
    SlashAPINamespace* top = this->parentNS;
    QString ns = this->name;

    while (top != 0)
    {
        if (!top->name.isEmpty())
        {
            ns = top->name + "." + ns;
        }

        top = top->parentNS;
    }

    return ns;
}

QList<SlashAPIObject*> SlashAPINamespace::getChildren() const
{
    QList<SlashAPIObject*> list;

    QObjectList objects = this->children();

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        SlashAPIObject* object = qobject_cast<SlashAPIObject*>(*it);

        if (object)
        {
            list.append(object);
        }
    }

    return list;
}
