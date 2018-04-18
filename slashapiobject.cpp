#include "slashapiobject.h"
#include "slashapinamespace.h"
#include "slashapi.h"

SlashAPIObject::SlashAPIObject(QObject* parent) :
    QObject(parent), parentNS(0), type(SlashAPI::Null)
{
}

SlashAPIObject::~SlashAPIObject()
{
}

QString SlashAPIObject::fullName() const
{
    QString ns;

    if (this->parentNS)
    {
        ns = this->parentNS->fullName();

        if (!ns.isEmpty())
        {
            ns.append(".");
        }
    }

    return ns.append(this->name);
}

QString SlashAPIObject::apiName() const
{
    return this->fullName() + "?" + QString::number(this->type);
}

QString SlashAPIObject::helpText() const
{
    return this->fullName() + "\n\n" + this->description;
}
