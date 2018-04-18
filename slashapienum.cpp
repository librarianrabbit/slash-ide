#include "slashapienum.h"
#include "slashapi.h"

SlashAPIEnum::SlashAPIEnum(QObject* parent) :
    SlashAPIObject(parent), value(0), replace(false)
{
}

SlashAPIEnum::~SlashAPIEnum()
{
}

QString SlashAPIEnum::helpText() const
{
    return SlashAPIObject::helpText() + "\nValue: " + QString::number(this->value);
}
