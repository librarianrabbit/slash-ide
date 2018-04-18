#include "luaeditor.h"
#include <QKeyEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <QWheelEvent>
#include <Qsci/qsciapis.h>
#include "lualexer.h"
#include "slashapienum.h"
#include <QListWidget>
#include <QScrollBar>
#include "slashapi.h"
#include <QApplication>

LuaEditor::LuaEditor(SlashAPI& slash, QWidget* parent):
    QsciScintilla(parent), api(slash)
{
    this->setupEditor();

    // TODO: Move to a better place
    this->loadAPI();
}

LuaEditor::~LuaEditor()
{
}

QString LuaEditor::fileName() const
{
    return this->editorFileName;
}

void LuaEditor::setOverwriteMode(bool overwrite)
{
    emit overwriteModeChanged(overwrite);
    QsciScintilla::setOverwriteMode(overwrite);
}

QString LuaEditor::eolString()
{
    QString line;
    EolMode mode = this->eolMode();

    if (mode == EolWindows || mode == EolMac)
    {
        line += "\r";
    }

    if (mode == EolWindows || mode == EolUnix)
    {
        line += "\n";
    }

    return line;
}

int LuaEditor::getCurrentLine()
{
    int line = -1;
    int index = -1;

    this->getCursorPosition(&line, &index);

    return line;
}

bool LuaEditor::close()
{
    return QsciScintilla::close();
}

void LuaEditor::deleteLine(int line)
{
    if (line < 0 || line > this->lines())
    {
        return;
    }

    int length = this->lineLength(line);
    int pos = this->positionFromLineIndex(line, 0);

    this->deleteRange(pos, length);
}

void LuaEditor::deleteRange(int pos, int length)
{
    this->SendScintilla(this->SCI_DELETERANGE, pos, length);
}

void LuaEditor::setFileName(const QString& fileName)
{
    this->editorFileName = fileName;
}

void LuaEditor::setLexerFont(const QFont& font)
{
    if (this->lexer)
    {
        editorFont = font;
        this->lexer->setFont(editorFont);
    }
}

void LuaEditor::resetZoom()
{
    this->SendScintilla(this->SCI_SETZOOM, 0);
}

void LuaEditor::setUtf8(bool cp)
{
    QsciScintilla::setUtf8(cp);
    emit encodingChanged();
}

bool LuaEditor::loadAPI()
{
    // TODO: Optmize for performance, cache and modifications (CRC)
    QStringList list = api.buildScintillaAPI();
    QsciAPIs* sciapi = new QsciAPIs(this->lexer);

    for (auto it = list.begin(); it != list.end(); it++)
    {
        sciapi->add(*it);
    }

    sciapi->prepare();
    this->lexer->setAPIs(sciapi);

    return true;
}

void LuaEditor::setupLexer()
{
    this->lexer = new LuaLexer(this);

    // TODO: Setting
    this->lexer->loadKeywordFile("keywords.xml");

    // TODO: Setting
    this->lexer->setFoldCompact(true);

    // TODO: Setting
    editorFont = QFont("Consolas", 10); // Notepad2's default font

    this->lexer->setFont(editorFont);

    // TODO: Setting
    this->setAutoCompletionSource(QsciScintilla::AcsAPIs); // Check later
    this->setAutoCompletionCaseSensitivity(false);
    this->setAutoCompletionFillupsEnabled(true);

    this->setLexer(this->lexer);
}

void LuaEditor::setupEditor()
{
    this->setupLexer();

    // TODO: Setting
    this->setTabWidth(4);
    this->setIndentationWidth(0); // My settings on Notepad2

    // TODO: Setting
    marginFont = QFont("Consolas", 8);
    this->setMarginsFont(marginFont);
    // FIXME: Margin width when zooming

    // TODO: Setting
    this->setMarginLineNumbers(0, true);

    // TODO: Setting
    this->setMarginsForegroundColor(QColor(Qt::red));
    this->setMarginsBackgroundColor(this->palette().background().color());

    // TODO: Setting
    this->setIndentationGuides(true);

    // TODO: Setting
    this->setAutoIndent(true);

    // TODO: Setting
    this->setBraceMatching(this->SloppyBraceMatch);

    // TODO: Setting
    this->setEdgeColumn(80);

    // TODO: Setting
    this->setEdgeMode(LuaEditor::EdgeLine);

    // TODO: Setting
    this->setWrapVisualFlags(LuaEditor::WrapFlagByBorder);

    // TODO: Only if @Setting:show_numbers
    this->updateLineNumbers();

    // TODO: Setting
    this->setUtf8(true);

    this->registerImage(SlashAPI::Namespace, QImage(":/auto/green.png"));
    this->registerImage(SlashAPI::Event, QImage(":/auto/orange.png"));
    this->registerImage(SlashAPI::Function, QImage(":/auto/red.png"));
    this->registerImage(SlashAPI::Enum, QImage(":/auto/blue.png"));
    this->registerImage(SlashAPI::Var, QImage(":/auto/blue.png"));

    // TODO: Only if @Setting:show_numbers
    // Update margin width to show line numbers when number of lines is changed
    connect(this, &LuaEditor::linesChanged, this, &LuaEditor::updateLineNumbers);
    connect(this, &LuaEditor::userListActivated, this, &LuaEditor::userListSelected);
    connect(this, &LuaEditor::SCN_CHARADDED, this, &LuaEditor::charAdded);
    connect(this, &LuaEditor::SCN_AUTOCSELECTION, this, &LuaEditor::autoListSelected);
}

// TODO: Move to specific functions
bool LuaEditor::event(QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);

        if (e->key() == Qt::Key_Space)
        {
            // Ctrl + Space triggers autocomplete
            if ((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
            {
                this->autoCompleteFromAPIs();

                if (!this->isListActive())
                {
                    this->autoCompleteFromDocument();

                    if (!this->isListActive())
                    {
                        this->showUserList(1, api.getTree("", SlashAPI::Any, SlashAPI::API));

                        if (this->isListActive())
                        {
                            this->startUpdatingScibox();
                        }
                    }
                    else
                    {
                        this->startUpdatingScibox();
                    }
                }
                else
                {
                    this->startUpdatingScibox();
                }

                e->accept();
                return true;
            }
        }
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        {
            // NOTE: QScintilla2 bug. Date: 01/06/2013
            // Fix for focus out bug. A new line is inserted if user
            // clicked the autocomplete box.

            QWidget* widget = this->getQsciSciListBox();

            if (widget && !widget->isVisible())
            {
                e->accept();
                return true;
            }
        }
        else if (e->key() == Qt::Key_Insert)
        {
            bool result = QsciScintilla::event(event);
            emit overwriteModeChanged(this->overwriteMode());
            return result;
        }
    }
    else if (event->type() == QEvent::Wheel) // FIXME: Don't scroll scintilla when holding control
    {
        // Zoom in/out with mouse wheel
        QWheelEvent* e = static_cast<QWheelEvent*>(event);

        if ((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        {
            if (e->delta() > 0)
            {
                this->zoomIn();
            }
            else
            {
                this->zoomOut();
            }

            e->accept();
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(event);

        if (e->button() == Qt::MiddleButton)
        {
            this->resetZoom();

            e->accept();
            return true;
        }
    }
    else if (event->type() == QEvent::ToolTip)
    {
        // Show help for word on tooltip
        QHelpEvent* e = static_cast<QHelpEvent*>(event);

        QString word = this->symbolAtPoint(e->pos());

        if (!word.isEmpty())
        {
            word = api.getHelp(word);

            if (!word.isEmpty())
            {
                if (!QToolTip::isVisible() || QToolTip::text() != word)
                {
                    QToolTip::showText(e->globalPos(), api.getHelp(word), this);

                    e->accept();
                    return true;
                }
            }
        }
    }
    else if (event->type() == QEvent::FocusOut)
    {
        // NOTE: QScintilla2 bug. Date: 01/06/2013
        //
        // http://www.mail-archive.com/qscintilla@riverbankcomputing.com/msg00199.html
        //
        // I can reproduce it and I agree that the problem is an unexpected focus out
        // event. However I'm at a loss as how to fix it. The focus out event is being
        // sent to the editor window even before the mouse press event that triggers
        // it is being sent to the list.
        //
        // Phil
        //
        // Autocomplete box loses focus when editor receive FocusOut
        // Intercepting FocusOut event to fix this bug

        if (this->isListActive())
        {
            bool keep = false;

            QPoint pos = QCursor::pos();
            QWidget* widget = qApp->widgetAt(pos);

            while (keep == false)
            {
                if (widget)
                {
                    if (widget->metaObject()->className() == QString("QsciSciListBox"))
                    {
                        keep = true;
                        break;
                    }
                    else
                    {
                        widget = widget->parentWidget();
                        continue;
                    }
                }
                else
                {
                    break;
                }
            }

            if (keep)
            {
                event->accept();
                return true;
            }
            else
            {
                widget = this->getQsciSciListBox();

                if (widget)
                {
                    widget->close();
                    widget->deleteLater();
                }
            }
        }
    }

    return QsciScintilla::event(event);
}

// Get the word with namespace at point
QString LuaEditor::symbolAtPoint(const QPoint& pt) const
{
    long pos = this->SendScintilla(SCI_POSITIONFROMPOINTCLOSE, pt.x(), pt.y());

    if (pos < 0)
    {
        return QString();
    }

    long start = this->SendScintilla(SCI_WORDSTARTPOSITION, pos, (long)true);
    long end = this->SendScintilla(SCI_WORDENDPOSITION, pos, (long)true);

    char prev = this->SendScintilla(SCI_GETCHARAT, start - 1);

    // Until top namespace
    while (prev == '.')
    {
        start -= 2;
        start = this->SendScintilla(SCI_WORDSTARTPOSITION, start, (long)true);
        prev = this->SendScintilla(SCI_GETCHARAT, start - 1);
    }

    int len = end - start;

    if (len < 0)
    {
        return QString();
    }

    char* buf = new char[len + 1];
    this->SendScintilla(SCI_GETTEXTRANGE, start, end, buf);

    QString word;

    //convertTextS2Q
    if (this->isUtf8())
    {
        word = QString::fromUtf8(buf);
    }
    else
    {
        word = QString::fromLatin1(buf);
    }

    delete[] buf;

    return word;
}

QListWidget* LuaEditor::getQsciSciListBox() const
{
    const QObjectList& list = this->children();

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        QObject* child = *it;

        if (child->metaObject()->className() == QString("QsciSciListBox"))
        {
            return qobject_cast<QListWidget*>(child);
        }
    }

    return 0;
}

void LuaEditor::updateLineNumbers()
{
    // TODO: Only if @Setting:show_numbers
    QFontMetrics metrics(marginFont);
    this->setMarginWidth(0, metrics.width(QString::number(this->lines())) + metrics.width("__"));
}

// Used only for global namespace
void LuaEditor::userListSelected(int id, const QString& text)
{
    Q_UNUSED(id);

    int line = 0;
    int index = 0;

    this->getCursorPosition(&line, &index);

    int pos = this->positionFromLineIndex(line, index);

    long start = this->SendScintilla(SCI_WORDSTARTPOSITION, pos, (long)true);
    long end = this->SendScintilla(SCI_WORDENDPOSITION, pos, (long)true);

    int len = end - start;

    QString ins = text.mid(len);
    this->insert(ins);
    this->setCursorPosition(line, index + ins.length());
    QToolTip::hideText();
}

void LuaEditor::charAdded(int ch)
{
    // Calls API autocompletion if '.' is inserted
    if (ch == '.')
    {
        this->autoCompleteFromAPIs();

        if (this->isListActive())
        {
            this->startUpdatingScibox();
        }
    }

    if (this->isListActive())
    {
        this->autoCompleteFromAPIs();

        if (this->isListActive())
        {
            this->startUpdatingScibox();
        }
    }
}

void LuaEditor::autoListSelected(const char* text, int id)
{
    Q_UNUSED(id);

    QString word(text);

    if (api.typeOf(word) == SlashAPI::Enum)
    {
        SlashAPIEnum* e = api.getEnums().value(word, 0);

        if (e && e->replace)
        {
            int line = 0;
            int index = 0;

            this->getCursorPosition(&line, &index);
            int pos = this->positionFromLineIndex(line, index);

            long start = this->SendScintilla(SCI_WORDSTARTPOSITION, pos, (long)true);
            long end = this->SendScintilla(SCI_WORDENDPOSITION, pos, (long)true);
            char prev = this->SendScintilla(SCI_GETCHARAT, start - 1);

            // Until top namespace
            while (prev == '.')
            {
                start -= 2;
                start = this->SendScintilla(SCI_WORDSTARTPOSITION, start, (long)true);
                prev = this->SendScintilla(SCI_GETCHARAT, start - 1);
            }

            int len = end - start;

            if (len < 0)
            {
                return;
            }

            word = QString::number(e->value);

            this->SendScintilla(SCI_DELETERANGE, start, len);
            this->insertAt(word, line, index - len);
            this->setCursorPosition(line, index - len + word.length());

            this->cancelList();
        }
    }

    QToolTip::hideText();
}

void LuaEditor::startUpdatingScibox()
{
    this->updateScibox();
}

void LuaEditor::updateScibox()
{
    QListWidget* box = this->getQsciSciListBox();

    if (!box || !box->isVisible())
    {
        return;
    }

    int hint = box->sizeHintForColumn(0) + box->verticalScrollBar()->width() + 5;

    box->setMinimumWidth(hint);
    box->adjustSize();

    if (this->isListActive())
    {
        QTimer::singleShot(500, this, SLOT(updateScibox()));
        this->sciboxTooltip();
    }
}

void LuaEditor::sciboxTooltip()
{
    QListWidget* box = this->getQsciSciListBox();

    if (!box || !box->isVisible())
    {
        return;
    }

    QListWidgetItem* item = box->currentItem();

    QPoint pos = box->mapFromGlobal(QCursor::pos());

    if (!item || box->viewport()->rect().contains(pos))
    {
        item = box->itemAt(pos);
    }

    if (item)
    {
        QString word = item->text();

        if (!word.isEmpty())
        {
            word = api.getHelp(word);

            if (!word.isEmpty())
            {
                if (!QToolTip::isVisible() || QToolTip::text() != word)
                {
                    QToolTip::showText(box->geometry().adjusted(0, 0, 20, -(box->height() / 2)).bottomRight(), api.getHelp(word), this);
                }
            }
        }
    }
}
