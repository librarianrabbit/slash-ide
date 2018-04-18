#ifndef SLASHAPIOBJECT_H
#define SLASHAPIOBJECT_H

#include <QObject>

class SlashAPINamespace;

class SlashAPIObject : public QObject
{
    Q_OBJECT

public:
    explicit SlashAPIObject(QObject* parent = 0);
    virtual ~SlashAPIObject();

    virtual QString fullName() const;
    virtual QString apiName() const;
    virtual QString helpText() const;

public:
    QString name;
    QString description;

    int type;

    SlashAPINamespace* parentNS;
};

#endif // SLASHAPIOBJECT_H
