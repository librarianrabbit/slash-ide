#ifndef LUALEXER_H
#define LUALEXER_H

#include <Qsci/qscilexerlua.h>

class QDomNode;

class LuaLexer : public QsciLexerLua
{
    Q_OBJECT

public:
    explicit LuaLexer(QObject* parent = 0);
    virtual ~LuaLexer();

    virtual QStringList autoCompletionWordSeparators() const;

    virtual const char* wordCharacters() const;
    virtual const char* keywords(int set) const;

    virtual bool caseSensitive() const;

    bool loadKeywordFile(const QString& fileName);
    bool loadKeywordData(const QString& data);

private:
    bool loadNode(const QDomNode& node, int setId);
    bool loadSet(const QDomNode& setNode, int setId);
    bool loadWord(const QDomNode& wordNode, int setId);

    QMap<int, QSharedPointer<char>> sets;
    QMap<int, QStringList> wordSets;
};

#endif // LUALEXER_H
