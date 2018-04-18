#include "lualexer.h"
#include <QFile>
#include <QDomDocument>

LuaLexer::LuaLexer(QObject* parent) :
    QsciLexerLua(parent)
{
}

LuaLexer::~LuaLexer()
{
}

QStringList LuaLexer::autoCompletionWordSeparators() const
{
    QStringList wl;

    wl << ":";

    return wl;
}

const char* LuaLexer::wordCharacters() const
{
    return "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.";
}

const char* LuaLexer::keywords(int set) const
{
    if (sets.contains(set))
    {
        QWeakPointer<char> data = sets[set].toWeakRef();

        if (!data.isNull())
        {
            return static_cast<const char*>(data.data());
        }
    }

    if (set == 1) // Keywords.
    {
        return "and break do else elseif end false for function if in local nil not or repeat return then true until while";
    }

    if (set == 2) // Basic functions.
    {
        return "_G assert error ipairs next pairs pcall rawequal tonumber tostring type xpcall";
    }

    if (set == 3) // String, table and maths functions.
    {
        return "string.byte string.char string.dump string.find string.format string.gmatch string.gsub string.len "
               "string.lower string.match string.rep string.reverse string.sub string.upper "
               "table.concat table.foreach table.foreachi table.getn table.insert table.maxn table.remove table.sort "
               "math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp "
               "math.floor math.fmod math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf "
               "math.pi math.pow math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh";
    }

    if (set == 4) // Coroutine, I/O and system facilities.
    {
        return "os.date os.difftime os.time";
    }

    return 0;
}

bool LuaLexer::caseSensitive() const
{
    return true;
}

bool LuaLexer::loadKeywordFile(const QString& fileName)
{
    QFile file(fileName);

    if (file.open(QFile::ReadOnly))
    {
        QString data(file.readAll());
        file.close();

        return this->loadKeywordData(data);
    }

    return false;
}

bool LuaLexer::loadKeywordData(const QString& data)
{
    QDomDocument root("keywords"); // <keywords>
    root.setContent(data);

    QDomNode kws = root.firstChild();

    if (kws.nodeName().toLower() != "keywords")
    {
        // TODO: Invalid document
        return false;
    }

    QDomNodeList childs = kws.childNodes();

    for (int i = 0; i < childs.length(); ++i)
    {
        this->loadNode(childs.at(i), 0);
    }

    for (auto it = wordSets.begin(); it != wordSets.end(); ++it)
    {
        if (!sets.contains(it.key()))
        {
            QStringList& list = it.value();
            list.removeDuplicates();
            list.sort();

            QByteArray kw = list.join(" ").toLocal8Bit();
            char* ptr = new char[kw.length() + 1];
            memcpy_s(ptr, kw.length() + 1, kw.data(), kw.length() + 1);

            sets.insert(it.key(), QSharedPointer<char>(ptr));
        }
    }

    wordSets.clear();

    return true;
}

bool LuaLexer::loadNode(const QDomNode& node, int setId)
{
    if (node.nodeName().toLower() == "set")
    {
        return this->loadSet(node, setId);
    }
    else if (node.nodeName().toLower() == "w")
    {
        return this->loadWord(node, setId);
    }

    return false;
}

bool LuaLexer::loadSet(const QDomNode& setNode, int setId)
{
    Q_UNUSED(setId);

    if (setNode.nodeName().toLower() != "set")
    {
        // TODO: Not a set node
        return false;
    }

    int id = 0;

    if (setNode.attributes().contains("id"))
    {
        id = setNode.attributes().namedItem("id").nodeValue().toInt();
    }

    if (id <= 0)
    {
        // TODO: Id must be greater than 0
        return false;
    }

    QDomNodeList childs = setNode.childNodes();

    for (int i = 0; i < childs.length(); ++i)
    {
        this->loadNode(childs.at(i), id);
    }

    return true;
}

bool LuaLexer::loadWord(const QDomNode& wordNode, int setId)
{
    if (wordNode.nodeName().toLower() != "w")
    {
        // TODO: Not a word node
        return false;
    }

    if (wordNode.hasChildNodes())
    {
        const QDomNode& text = wordNode.firstChild();

        if (text.isText())
        {
            wordSets[setId].append(text.nodeValue());
            return true;
        }
    }

    return false;
}
