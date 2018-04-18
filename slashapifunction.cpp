#include "slashapifunction.h"
#include "slashapi.h"

SlashAPIFunction::SlashAPIFunction(QObject* parent) :
    SlashAPIObject(parent)
{
}

SlashAPIFunction::~SlashAPIFunction()
{
}

QString SlashAPIFunction::apiName() const
{
    return this->fullName() + "?" + QString::number(this->type) + "(" + this->parameters.join(", ") + ")\n" + this->description;
}

QString SlashAPIFunction::helpText() const
{
    return "function " + this->fullName() + "(" + this->parameters.join(", ") + ")\n\n" + this->description;
}

QStringList SlashAPIFunction::splitParameters(const QString& params)
{
    QStringList paramList = params.split(",");

    for (int i = 0; i < paramList.length(); ++i)
    {
        paramList[i] = paramList[i].trimmed();
    }

    return paramList;
}
