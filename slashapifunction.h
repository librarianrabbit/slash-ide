#ifndef SLASHAPIFUNCTION_H
#define SLASHAPIFUNCTION_H

#include "slashapiobject.h"
#include <QStringList>

// Used for <event> and <function>
class SlashAPIFunction : public SlashAPIObject
{
    Q_OBJECT

public:
    explicit SlashAPIFunction(QObject* parent = 0);
    virtual ~SlashAPIFunction();

    virtual QString apiName() const;
    virtual QString helpText() const;

    // Parameters on xml are: Type param, Type param, Type param
    // splitParameters just split on ',' and trim every parameter
    static QStringList splitParameters(const QString& params);

public:
    QStringList parameters;
};

#endif // SLASHAPIFUNCTION_H
