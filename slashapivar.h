#ifndef SLASHAPIVAR_H
#define SLASHAPIVAR_H

#include "slashapiobject.h"

// Used for <var>
class SlashAPIVar : public SlashAPIObject
{
    Q_OBJECT

public:
    explicit SlashAPIVar(QObject* parent = 0);
    virtual ~SlashAPIVar();
};

#endif // SLASHAPIVAR_H
