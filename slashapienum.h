#ifndef SLASHAPIENUM_H
#define SLASHAPIENUM_H

#include "slashapiobject.h"

// Used for <enum>
class SlashAPIEnum : public SlashAPIObject
{
    Q_OBJECT

public:
    explicit SlashAPIEnum(QObject* parent = 0);
    virtual ~SlashAPIEnum();

    virtual QString helpText() const;

public:
    int value;
    bool replace;

    // If replace=true, autocomplete will replace the string with it's value
    // This allow we to use "fake" enums to help coding.
};

#endif // SLASHAPIENUM_H
