#ifndef SLASHAPINAMESPACE_H
#define SLASHAPINAMESPACE_H

#include "slashapiobject.h"
#include <QStringList>

// TODO: Add support to childs and search and items
// Used for <namespace>
class SlashAPINamespace : public SlashAPIObject
{
    Q_OBJECT

public:
    explicit SlashAPINamespace(QObject* parent = 0);
    virtual ~SlashAPINamespace();

    virtual QString fullName() const;

    virtual QList<SlashAPIObject*> getChildren() const;
};

#endif // SLASHAPINAMESPACE_H
