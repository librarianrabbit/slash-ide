#ifndef SLASHAPI_H
#define SLASHAPI_H

#include <QObject>
#include <QHash>

class QDomNode;
class SlashAPINamespace;
class SlashAPIFunction;
class SlashAPIEnum;
class SlashAPIVar;

class SlashAPI : public QObject
{
    Q_OBJECT

public:
    SlashAPI();
    virtual ~SlashAPI();

    // Types of api.xml fields
    enum FieldType
    {
        Null = 0x00,
        Namespace = 0x01,
        Event = 0x02,
        Function = 0x04,
        Enum = 0x08,
        Var = 0x10,
        Any = 0xff
    };

    // Types for calling getGlobalList
    enum GlobalListArg
    {
        Name = 0,
        FullName = 1,
        API = 2
    };

    // Load api.xml from file - calls loadData to load from memory
    bool loadFile(const QString& fileName);
    bool loadData(const QString& data);

    // Build stringset to load into scintilla autocomplete list
    QStringList buildScintillaAPI();

    // Clear all fields and free memory
    void clear();

    FieldType typeOf(const QString& name) const;
    QString getHelp(const QString& name) const;

    QStringList getTree(const QString& name, FieldType filter, GlobalListArg get) const;
    QStringList getTree(SlashAPINamespace* ns, FieldType filter, GlobalListArg get) const;

    const QHash<QString, FieldType>& getNames() const;
    const QHash<QString, SlashAPINamespace*>& getNamespaces() const;
    const QHash<QString, SlashAPIFunction*>& getEvents() const;
    const QHash<QString, SlashAPIFunction*>& getFunctions() const;
    const QHash<QString, SlashAPIEnum*>& getEnums() const;
    const QHash<QString, SlashAPIVar*>& getVars() const;

private:
    bool loadNode(const QDomNode& node, SlashAPINamespace* current = 0);
    bool loadNamespace(const QDomNode& namespaceNode, SlashAPINamespace* current = 0);
    bool loadEvent(const QDomNode& node, SlashAPINamespace* current = 0);
    bool loadFunction(const QDomNode& node, SlashAPINamespace* current = 0);
    bool loadEnum(const QDomNode& node, SlashAPINamespace* current = 0);
    bool loadVar(const QDomNode& node, SlashAPINamespace* current = 0);

// Events are like functions.
// Events must have a fixed name from Tig's api and are callbacks.

private:
    QHash<QString, FieldType> names; // Global lookup table. I think I don't need it
    QHash<QString, SlashAPINamespace*> namespaces; // Namespace are tables with children
    QHash<QString, SlashAPIFunction*> events; // Events are callbacks
    QHash<QString, SlashAPIFunction*> functions; // Functions are api functions
    QHash<QString, SlashAPIEnum*> enums; // Enums are fixex-value api vars
    QHash<QString, SlashAPIVar*> vars; // Vars are unknown-value api vars (in general, read-only)
};

#endif // SLASHAPI_H
