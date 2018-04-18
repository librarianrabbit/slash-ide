#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "slashapi.h"

namespace Ui
{
    class MainWindow;
}

class QLabel;
class QClickableLabel;
class LuaEditor;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

    virtual bool loadAPI();

    virtual QString getOpenFileName();
    virtual QString getSaveFileName();

public slots:
    // TODO: Create overloads for all (index = -1) functions.
    virtual bool newTab(const QString& fileName = QString());
    virtual bool closeTab(int index = -1);

    virtual void openFile();
    virtual bool reloadFile(int index = -1);
    virtual bool saveFile(int index = -1);
    virtual bool saveFileAs(int index = -1);
    virtual bool saveFileCopy(int index = -1);

    virtual bool closeFile(int index = -1);
    virtual bool closeAllFiles();

    virtual void undo();
    virtual void redo();

    virtual void cut();
    virtual void copy();
    virtual void copyAll();
    virtual void paste();
    virtual void clear();
    virtual void clearClipboard();
    virtual void selectAll();

    virtual void moveUp();
    virtual void moveDown();
    virtual void duplicateLine();
    virtual void cutLine();
    virtual void copyLine();
    virtual void deleteLine();
    virtual void splitLines();
    virtual void joinLines();

    virtual void indent();
    virtual void unindent();
    virtual void encloseSelection();
    virtual void duplicateSelection();
    virtual void padWithSpaces();
    virtual void stripTrailingBlanks();
    virtual void compressWhitespace();
    virtual void removeBlankLines();
    virtual void modifyLines();
    virtual void sortLines();

    virtual void uppercase();
    virtual void lowercase();
    virtual void invertCase();
    virtual void tabifySelection();
    virtual void untabifySelection();
    virtual void tabifyIndent();
    virtual void untabifyIndent();
    virtual void toggleEncoding();

    virtual void toggleLineComment();
    virtual void findMachingBrace();
    virtual void selectToMatchingBrace();

    virtual void customizeScheme();
    virtual void changeDefaultFont();
    virtual void wordWrap();
    virtual void longLineMarker();
    virtual void indentationGuides();
    virtual void showWhitespace();
    virtual void showLineEndings();
    virtual void visualBraceMatching();
    virtual void highlightCurrentLine();
    virtual void lineNumbers();
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void resetZoom();

    virtual void insertTabsAsSpaces();
    virtual void tabSettings();
    virtual void wordWrapSettings();
    virtual void longLineSettings();
    virtual void autoIndentText();
    virtual void autoCloseMarkupTag();
    virtual void escNone();
    virtual void escMinimize();
    virtual void escExit();
    virtual void saveSettingsOnExit();
    virtual void saveSettingsNow();

    virtual void showFinder();
    virtual void toggleFinder();
    virtual void ensureFinderVisible();

    virtual void findPrev();
    virtual void findNext();
    virtual bool replace();
    virtual void replacePrev();
    virtual void replaceNext();
    virtual void replaceInSel();
    virtual void replaceAll();

    virtual bool findOnEditor(bool forward = true);

    virtual void gotoLineIndex();

    virtual void showAbout();

    // TODO: Refactor
    virtual void updateStats();
    virtual void updateCursorStats(int line, int index);
    virtual void updateSelectionStats();
    virtual void updateBytesStats();
    virtual void updateEncoding();
    virtual void updateInputState(bool mode);
    virtual void updateFinderState(bool visible);

protected:
    virtual void setupInterface();
    virtual void setupEditor(LuaEditor* editor);
    virtual void setupStatusBar();
    virtual void closeEvent(QCloseEvent* event);

protected slots:
    virtual void updateTitle(int index = -1);

private:
    Ui::MainWindow* ui;
    // TODO: Make pointer and remove include to add forward declaration
    SlashAPI api;

    QLabel* sLines;
    QLabel* sCols;
    QLabel* sSel;
    QLabel* sBytes;
    QClickableLabel* sEncoding;
    QLabel* sInput;
    QToolButton* sFind;

    QIcon fileBlue;
    QIcon fileRed;

    // TODO: Setting
    const static int maxTitleLen = 30;
};

#endif // MAINWINDOW_H
