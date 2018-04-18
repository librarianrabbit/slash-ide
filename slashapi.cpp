#include "slashapi.h"
#include "slashapinamespace.h"
#include "slashapifunction.h"
#include "slashapienum.h"
#include "slashapivar.h"
#include <QDomDocument>
#include <QFile>

SlashAPI::SlashAPI()
{
    this->clear();
}

SlashAPI::~SlashAPI()
{
}

bool SlashAPI::loadFile(const QString& fileName)
{
    QFile file(fileName);

    if (file.open(QFile::ReadOnly))
    {
        QString data(file.readAll());
        file.close();

        return this->loadData(data);
    }

    return false;
}

bool SlashAPI::loadData(const QString& data)
{
    QDomDocument root("transformice"); // <transformice>
    root.setContent(data);

    QDomNode transformice = root.firstChild();

    if (transformice.nodeName().toLower() != "transformice")
    {
        // TODO: Invalid document
        return false;
    }

    QDomNodeList childs = transformice.childNodes();

    for (int i = 0; i < childs.length(); ++i)
    {
        this->loadNode(childs.at(i), namespaces[""]);
    }

    return true;
}

QStringList SlashAPI::buildScintillaAPI()
{
    QStringList data;

    for (auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        SlashAPINamespace* ns = it.value();

        if (ns && !ns->name.isEmpty())
        {
            data.append(ns->apiName());
        }
    }

    for (auto it = events.begin(); it != events.end(); ++it)
    {
        SlashAPIFunction* ev = it.value();

        if (ev && !ev->name.isEmpty())
        {
            data.append(ev->apiName());
        }
    }

    for (auto it = functions.begin(); it != functions.end(); ++it)
    {
        SlashAPIFunction* fn = it.value();

        if (fn && !fn->name.isEmpty())
        {
            data.append(fn->apiName());
        }
    }

    for (auto it = enums.begin(); it != enums.end(); ++it)
    {
        SlashAPIEnum* en = it.value();

        if (en && !en->name.isEmpty())
        {
            data.append(en->apiName());
        }
    }

    for (auto it = vars.begin(); it != vars.end(); ++it)
    {
        SlashAPIVar* va = it.value();

        if (va && !va->name.isEmpty())
        {
            data.append(va->apiName());
        }
    }

    data.sort();

    return data;
}

void SlashAPI::clear()
{
    // If we already have a global namespace we free it
    // All API QObjects are children of it, so we free it all!
    if (namespaces.contains(""))
    {
        SlashAPINamespace* ns = namespaces[""];
        ns->deleteLater();
    }

    names.clear();
    namespaces.clear();
    events.clear();
    functions.clear();
    enums.clear();
    vars.clear();

    // Global (unnamed) namespace
    SlashAPINamespace* global = new SlashAPINamespace(this);
    global->name = "";
    global->description = "Global namespace";
    global->parentNS = 0;

    namespaces[""] = global;
}

SlashAPI::FieldType SlashAPI::typeOf(const QString& name) const
{
    return this->names.value(name, Null);
}

QString SlashAPI::getHelp(const QString& name) const
{
    FieldType type = this->typeOf(name);
    SlashAPIObject* object = 0;

    switch (type)
    {
        case Namespace:
            object = this->namespaces.value(name, 0);
            break;

        case Event:
            object = this->events.value(name, 0);
            break;

        case Function:
            object = this->functions.value(name, 0);
            break;

        case Enum:
            object = this->enums.value(name, 0);
            break;

        case Var:
            object = this->vars.value(name, 0);
            break;
    }

    if (object)
    {
        return object->helpText();
    }

    return name;
}

QStringList SlashAPI::getTree(const QString& name, FieldType filter, GlobalListArg get) const
{
    return this->getTree(namespaces.value(name, 0), filter, get);
}

QStringList SlashAPI::getTree(SlashAPINamespace* ns, FieldType filter, GlobalListArg get) const
{
    QStringList list;

    if (ns)
    {
        QList<SlashAPIObject*> apis = ns->getChildren();

        for (auto it = apis.begin(); it != apis.end(); ++it)
        {
            SlashAPIObject* api = *it;

            if (api && ((api->type & filter) == api->type))
            {
                switch (get)
                {
                    case Name:
                        list.append(api->name);
                        break;

                    case FullName:
                        list.append(api->fullName());
                        break;

                    case API:
                        list.append(api->apiName());
                        break;
                }
            }
        }
    }

    list.sort();

    return list;
}

const QHash<QString, SlashAPI::FieldType>& SlashAPI::getNames() const
{
    return this->names;
}

const QHash<QString, SlashAPINamespace*>& SlashAPI::getNamespaces() const
{
    return this->namespaces;
}

const QHash<QString, SlashAPIFunction*>& SlashAPI::getEvents() const
{
    return this->events;
}

const QHash<QString, SlashAPIFunction*>& SlashAPI::getFunctions() const
{
    return this->functions;
}

const QHash<QString, SlashAPIEnum*>& SlashAPI::getEnums() const
{
    return this->enums;
}

const QHash<QString, SlashAPIVar*>& SlashAPI::getVars() const
{
    return this->vars;
}

bool SlashAPI::loadNode(const QDomNode& node, SlashAPINamespace* current)
{
    // current is used to propagate namespace hierarchy

    if (node.nodeName().toLower() == "namespace") // <namespace name desc>childs</namespace>
    {
        return this->loadNamespace(node, current);
    }
    else if (node.nodeName().toLower() == "event") // <event name params>desc</event>
    {
        return this->loadEvent(node, current);
    }
    else if (node.nodeName().toLower() == "function") // <function name params>desc</function>
    {
        return this->loadFunction(node, current);
    }
    else if (node.nodeName().toLower() == "enum") // <enum name value replace>desc</enum>
    {
        return this->loadEnum(node, current);
    }
    else if (node.nodeName().toLower() == "var") // <var name>desc</var>
    {
        return this->loadVar(node, current);
    }
    else
    {
        // TODO: Invalid node
    }

    return false;
}

bool SlashAPI::loadNamespace(const QDomNode& namespaceNode, SlashAPINamespace* current)
{
    if (namespaceNode.nodeName().toLower() != "namespace")
    {
        // TODO: Not a namespace node
        return false;
    }

    QString name;

    if (namespaceNode.attributes().contains("name"))
    {
        name = namespaceNode.attributes().namedItem("name").nodeValue();
    }

    SlashAPINamespace* ns = 0;

    // If a namespace already exists we'll use it and ignore description and hierarchy
    if (namespaces.contains(name))
    {
        ns = namespaces[name];
    }
    else
    {
        ns = new SlashAPINamespace(current);
        ns->parentNS = current;
        ns->name = name;
    }

    if (namespaceNode.attributes().contains("desc"))
    {
        ns->description = namespaceNode.attributes().namedItem("desc").nodeValue();
    }

    QDomNodeList childs = namespaceNode.childNodes();

    for (int i = 0; i < childs.length(); ++i)
    {
        this->loadNode(childs.at(i), ns);
    }

    name = ns->fullName();

    names[name] = Namespace;
    ns->type = Namespace;
    namespaces[name] = ns;

    return true;
}

bool SlashAPI::loadEvent(const QDomNode& node, SlashAPINamespace* current)
{
    if (node.nodeName().toLower() != "event")
    {
        // TODO: Not an event node
        return false;
    }

    QString name;

    if (node.attributes().contains("name"))
    {
        name = node.attributes().namedItem("name").nodeValue();
    }

    if (!name.isEmpty())
    {
        SlashAPIFunction* event = new SlashAPIFunction(current);
        event->parentNS = current;
        event->name = name;

        if (node.attributes().contains("params"))
        {
            event->parameters = SlashAPIFunction::splitParameters(node.attributes().namedItem("params").nodeValue());
        }

        if (node.hasChildNodes())
        {
            const QDomNode& text = node.firstChild();

            if (text.isText())
            {
                event->description = text.nodeValue();
            }
        }

        QString name = event->fullName();

        names[name] = Event;
        event->type = Event;
        events[name] = event;

        return true;
    }

    return false;
}

bool SlashAPI::loadFunction(const QDomNode& node, SlashAPINamespace* current)
{
    if (node.nodeName().toLower() != "function")
    {
        // TODO: Not a function node
        return false;
    }

    QString name;

    if (node.attributes().contains("name"))
    {
        name = node.attributes().namedItem("name").nodeValue();
    }

    if (!name.isEmpty())
    {
        SlashAPIFunction* function = new SlashAPIFunction(current);
        function->parentNS = current;
        function->name = name;

        if (node.attributes().contains("params"))
        {
            // Parameters on xml are: Type param, Type param, Type param
            // splitParameters just split on ',' and trim every parameter
            function->parameters = SlashAPIFunction::splitParameters(node.attributes().namedItem("params").nodeValue());
        }

        if (node.hasChildNodes())
        {
            const QDomNode& text = node.firstChild();

            if (text.isText())
            {
                function->description = text.nodeValue();
            }
        }

        QString name = function->fullName();

        names[name] = Function;
        function->type = Function;
        functions[name] = function;

        return true;
    }

    return false;
}

bool SlashAPI::loadEnum(const QDomNode& node, SlashAPINamespace* current)
{
    if (node.nodeName().toLower() != "enum")
    {
        // TODO: Not an enum node
        return false;
    }

    QString name;

    if (node.attributes().contains("name"))
    {
        name = node.attributes().namedItem("name").nodeValue();
    }

    if (!name.isEmpty())
    {
        SlashAPIEnum* e = new SlashAPIEnum(current);
        e->parentNS = current;
        e->name = name;

        if (node.attributes().contains("value"))
        {
            e->value = node.attributes().namedItem("value").nodeValue().toInt();
        }

        if (node.attributes().contains("replace"))
        {
            // If replace=true, autocomplete will replace the string with it's value
            // This allow we to use "fake" enums to help coding
            e->replace = (node.attributes().namedItem("replace").nodeValue().toLower() == "true" ? true : false);
        }

        if (node.hasChildNodes())
        {
            const QDomNode& text = node.firstChild();

            if (text.isText())
            {
                e->description = text.nodeValue();
            }
        }

        QString name = e->fullName();

        names[name] = Enum;
        e->type = Enum;
        enums[name] = e;

        return true;
    }

    return false;
}

bool SlashAPI::loadVar(const QDomNode& node, SlashAPINamespace* current)
{
    if (node.nodeName().toLower() != "var")
    {
        // TODO: Not a var node
        return false;
    }

    QString name;

    if (node.attributes().contains("name"))
    {
        name = node.attributes().namedItem("name").nodeValue();
    }

    if (!name.isEmpty())
    {
        SlashAPIVar* var = new SlashAPIVar(current);
        var->parentNS = current;
        var->name = name;

        if (node.hasChildNodes())
        {
            const QDomNode& text = node.firstChild();

            if (text.isText())
            {
                var->description = text.nodeValue();
            }
        }

        QString name = var->fullName();

        names[name] = Var;
        var->type = Var;
        vars[name] = var;

        return true;
    }

    return false;
}
