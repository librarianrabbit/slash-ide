#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include <Qsci/qsciscintilla.h>

class LuaLexer;
class QListWidget;
class QListWidgetItem;
class SlashAPI;

// TODO: Make global e local lexers
class LuaEditor : public QsciScintilla
{
    Q_OBJECT

public:
    explicit LuaEditor(SlashAPI& slash, QWidget* parent = 0);
    virtual ~LuaEditor();

    QString fileName() const;

    virtual void setOverwriteMode(bool overwrite);

    virtual QString eolString();
    virtual int getCurrentLine();

public slots:
    virtual bool close();

    virtual void deleteLine(int line);
    virtual void deleteRange(int pos, int length);

    virtual void setFileName(const QString& fileName);

    virtual void setLexerFont(const QFont& font);

    virtual void resetZoom();

    virtual void setUtf8(bool cp);

protected:
    virtual bool loadAPI();

    virtual void setupLexer();
    virtual void setupEditor();

    virtual bool event(QEvent* event);

    virtual QString symbolAtPoint(const QPoint& pos) const;

    virtual QListWidget* getQsciSciListBox() const;

protected slots:
    virtual void updateLineNumbers();
    virtual void userListSelected(int id, const QString& text);
    virtual void charAdded(int ch);
    virtual void autoListSelected(const char* text, int id);
    virtual void startUpdatingScibox();
    virtual void updateScibox();
    virtual void sciboxTooltip();

signals:
    void overwriteModeChanged(bool over) const;
    void encodingChanged() const;

private:
    // TODO: Make global?
    LuaLexer* lexer;

    // TODO: Make global
    QFont editorFont;

    // TODO: Make global
    QFont marginFont;

    SlashAPI& api;

    QString editorFileName;
};

#endif // LUAEDITOR_H
