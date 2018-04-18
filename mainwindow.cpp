#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "luaeditor.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QLabel>
#include "utils.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QToolButton>
#include "gotodialog.h"
#include <QClipboard>
#include "encloseselectiondialog.h"
#include "modifylinesdialog.h"
#include <QFontDialog>
#include <QCheckBox>
#include "utils.hxx"
#include "qclickablelabel.h"

class QCheckBox;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sLines(0),
    sCols(0),
    sSel(0),
    sBytes(0),
    sInput(0),
    sFind(0),
    fileBlue(":/file/file_blue.png"),
    fileRed(":/file/file_red.png")
{    
    ui->setupUi(this);
    this->setupInterface();
    this->loadAPI();
}

MainWindow::~MainWindow()
{
    delete ui;

    qApp->closeAllWindows();
    qApp->quit();
}

bool MainWindow::loadAPI()
{
    // TODO: Check if file was changed
    // TODO: Handle other files
    // TODO: Handle $include files
    // TODO: Handle $import files
    // TODO: Handle $api files
    api.loadFile("api.xml");
    api.loadFile("lua.xml");

    return true;
}

QString MainWindow::getOpenFileName()
{
    return QFileDialog::getOpenFileName(this, QString(), QString(), "Lua scripts (*.lua);;Text files (*.txt);;All files (*.*)");
}

QString MainWindow::getSaveFileName()
{
    return QFileDialog::getSaveFileName(this, QString(), QString(), "Lua scripts (*.lua);;Text files (*.txt);;All files (*.*)");
}

bool MainWindow::newTab(const QString& fileName)
{
    // TODO: Check if file is already open
    // TODO: Check file size and alert
    // TODO: Setting

    bool result = false;

    int index = ui->tabWidget->currentIndex();
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (index != -1 && editor && editor->fileName().isEmpty() && editor->text().isEmpty() && !editor->isModified() && !fileName.isEmpty())
    {
        this->closeTab(index);
    }
    else
    {
        index = ui->tabWidget->count();
    }

    if (fileName.isEmpty())
    {
        editor = new LuaEditor(api, this);

        this->setupEditor(editor);

        ui->tabWidget->insertTab(index, editor, tr("Untitled"));

        editor->setModified(false);
        this->updateTitle(index);

        result = true;
    }
    else
    {
        QFile file(fileName);

        if (file.open(QFile::ReadOnly))
        {
            QString data = file.readAll();

            QFileInfo info(file);

            QString title = info.fileName();

            if (title.length() > maxTitleLen)
            {
                title = info.baseName();

                if (title.length() > maxTitleLen)
                {
                    title = title.left(maxTitleLen - 3) + "...";
                }
            }

            file.close();

            editor = new LuaEditor(api, this);
            editor->setFileName(info.absoluteFilePath());

            this->setupEditor(editor);

            ui->tabWidget->insertTab(index, editor, title);

            editor->setText(data);

            result = true;

            editor->setModified(false);
            this->updateTitle(index);
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open file: ") + "\n" + fileName);
        }
    }

    if (result)
    {
        ui->tabWidget->setCurrentIndex(index);
    }

    return result;
}

bool MainWindow::closeTab(int index)
{
    // TODO: Alert if file modified
    bool result = true;

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

    if (!editor)
    {
        index = ui->tabWidget->currentIndex();
        editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

        if (!editor)
        {
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
        }
    }

    if (editor)
    {
        if (editor->isModified())
        {
            QString msg = tr("Do you want to save this file before closing?") + "\n";

            if (!editor->fileName().isEmpty())
            {
                msg += "\n" + editor->fileName() + "\n\n";
            }
            else
            {
                ui->tabWidget->setCurrentIndex(index);
            }

            msg += tr("It was modified and not saved. You may lose the last changes.");

            int option = QMessageBox::question(this, tr("File is modified"), msg, (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));

            if (option == QMessageBox::Yes)
            {
                result = this->saveFile(index);
            }
            else if (option == QMessageBox::Cancel)
            {
                result = false;
            }
        }

        if (result)
        {
            editor->close();
            editor->deleteLater();
        }
    }

    if (result)
    {
        ui->tabWidget->removeTab(index);

        if (ui->tabWidget->count() == 0)
        {
            ui->finder->hide();
        }
    }

    return result;
}

void MainWindow::openFile()
{    
    QString fileName = this->getOpenFileName();

    if (!fileName.isEmpty())
    {
        this->newTab(fileName);
    }
}

// TODO: Optimize for size and less code
bool MainWindow::reloadFile(int index)
{
    // TODO: Check for failed

    bool result = false;

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

    if (!editor)
    {
        index = ui->tabWidget->currentIndex();
        editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

        if (!editor)
        {
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
        }
    }

    if (editor && !editor->fileName().isEmpty())
    {
        QString msg = tr("Do you really want to reload this file?") + "\n";
        msg += "\n" + editor->fileName() + "\n\n";
        msg += "It was modified and not saved. You may lose the last changes.";

        if (QMessageBox::question(this, tr("File modified"), msg) == QMessageBox::Yes)
        {
            QFile file(editor->fileName());

            if (file.open(QFile::ReadOnly))
            {
                QString data = file.readAll();
                file.close();

                editor->setText(data);

                result = true;

                editor->setModified(false);
                this->updateTitle(index);
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to reload file: ") + "\n" + editor->fileName());
            }
        }
    }

    return result;
}

// TODO: Optimize save routines for size and less code
// TODO: Fix blank line
bool MainWindow::saveFile(int index)
{
    // TODO: Check for read-only
    // TODO: Setting

    bool result = false;

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

    if (!editor)
    {
        index = ui->tabWidget->currentIndex();
        editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

        if (!editor)
        {
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
        }
    }

    if (editor)
    {
        if (!editor->fileName().isEmpty())
        {
            QFile file(editor->fileName());

            if (file.open(QFile::WriteOnly))
            {
                if (editor->isUtf8())
                {
                    file.write(editor->text().toUtf8());
                }
                else
                {
                    file.write(editor->text().toLatin1());
                }

                result = true;

                file.close();

                editor->setModified(false);
                this->updateTitle(index);
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save file: ") + "\n"  + editor->fileName());
            }
        }
        else
        {
            result = this->saveFileAs();
        }
    }

    return result;
}

bool MainWindow::saveFileAs(int index)
{
    // TODO: Check for read-only
    // TODO: Setting

    bool result = false;

    QString fileName = this->getSaveFileName();

    if (!fileName.isEmpty())
    {
        LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

        if (!editor)
        {
            index = ui->tabWidget->currentIndex();
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

            if (!editor)
            {
                editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
            }
        }

        if (editor)
        {
            QFile file(fileName);

            if (file.open(QFile::WriteOnly))
            {
                if (editor->isUtf8())
                {
                    file.write(editor->text().toUtf8());
                }
                else
                {
                    file.write(editor->text().toLatin1());
                }

                QFileInfo info(file);

                QString title = info.fileName();

                if (title.length() > maxTitleLen)
                {
                    title = info.baseName();

                    if (title.length() > maxTitleLen)
                    {
                        title = title.left(maxTitleLen - 3) + "...";
                    }
                }

                editor->setFileName(info.absoluteFilePath());

                file.close();

                ui->tabWidget->setTabText(index, title);

                result = true;

                editor->setModified(false);
                this->updateTitle(index);
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save file: ") + "\n"  + fileName);
            }
        }
    }

    return result;
}

bool MainWindow::saveFileCopy(int index)
{
    // TODO: Check for read-only
    // TODO: Setting

    bool result = false;

    QString fileName = this->getSaveFileName();

    if (!fileName.isEmpty())
    {
        LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

        if (!editor)
        {
            index = ui->tabWidget->currentIndex();
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));

            if (!editor)
            {
                editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
            }
        }

        if (editor)
        {
            QFile file(fileName);

            if (file.open(QFile::WriteOnly))
            {
                if (editor->isUtf8())
                {
                    file.write(editor->text().toUtf8());
                }
                else
                {
                    file.write(editor->text().toLatin1());
                }

                file.close();

                result = true;
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save file: ") + "\n"  + fileName);
            }
        }
    }

    return result;
}

bool MainWindow::closeFile(int index)
{
    bool result = false;

    if (!ui->tabWidget->widget(index))
    {
        index = ui->tabWidget->currentIndex();
    }

    if (index >= 0)
    {
        result = this->closeTab(index);
    }

    return result;
}

bool MainWindow::closeAllFiles()
{
    bool result = true;

    while (ui->tabWidget->count() > 0)
    {
        if (!this->closeTab(0))
        {
            result = false;
            break;
        }
    }

    return result;
}

void MainWindow::undo()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->undo();
    }
}

void MainWindow::redo()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->redo();
    }
}

void MainWindow::cut()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->cut();
    }
}

void MainWindow::copy()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->copy();
    }
}

void MainWindow::copyAll()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        QString text = editor->text();
        qApp->clipboard()->setText(text);
    }
}

void MainWindow::paste()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->paste();
    }
}

void MainWindow::clear()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        if (editor->hasSelectedText())
        {
            editor->removeSelectedText();
        }
        else
        {
            editor->clear();
        }
    }
}

void MainWindow::clearClipboard()
{
    qApp->clipboard()->clear();
}

void MainWindow::selectAll()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->selectAll(true);
    }
}

void MainWindow::showFinder()
{
    // TODO: Setting
    static int maxIndex = 36;

    ui->finder->show();
    ui->finder->setFocus();

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        QString selected = editor->selectedText();

        if (selected.contains(QRegExp("[\r\n]")))
        {
            int index = selected.indexOf(QRegExp("[\r\n]"));

            if (index > 0)
            {
                if (index > maxIndex)
                {
                    index = maxIndex;
                }

                selected = selected.left(index);
            }
        }
        else if (selected.length() > maxIndex)
        {
            selected = selected.left(maxIndex);
        }

        ui->finder->setFindText(selected);
    }

    ui->finder->selectFindText();
}

void MainWindow::toggleFinder()
{
    if (ui->finder->isVisible())
    {
        ui->finder->hide();
    }
    else
    {
        this->showFinder();
    }
}

void MainWindow::ensureFinderVisible()
{
    if (ui->finder->isHidden())
    {
        this->showFinder();
    }
}

void MainWindow::findPrev()
{
    this->ensureFinderVisible();

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        this->findOnEditor(false);
    }
}

void MainWindow::findNext()
{
    this->ensureFinderVisible();

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        this->findOnEditor(true);
    }
}

// TODO: Refactor
bool MainWindow::replace()
{
    bool result = false;

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        if (editor->hasSelectedText())
        {
            int fromLine = -1;
            int fromIndex = -1;
            int toLine = -1;
            int toIndex = -1;

            editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
            editor->setCursorPosition(fromLine, fromIndex);
        }

        bool select = ui->finder->selectToFind();
        ui->finder->setSelectToFind(select);

        result = this->findOnEditor(true);

        ui->finder->setSelectToFind(select);

        if (result)
        {
            QString text = ui->finder->replaceText();
            editor->replace(text);

            if (text.isEmpty())
            {
                select = ui->finder->selectToFind();
                ui->finder->setSelectToFind(select);

                this->findOnEditor(true);

                ui->finder->setSelectToFind(select);
            }
        }
    }

    return result;
}

void MainWindow::replacePrev()
{
    if (this->replace() == true)
    {
        bool select = ui->finder->selectToFind();
        ui->finder->setSelectToFind(select);

        this->findOnEditor(false);

        ui->finder->setSelectToFind(select);
    }
}

void MainWindow::replaceNext()
{
    if (this->replace() == true)
    {
        bool select = ui->finder->selectToFind();
        ui->finder->setSelectToFind(select);

        this->findOnEditor(true);

        ui->finder->setSelectToFind(select);
    }
}

void MainWindow::replaceInSel()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    QString findText = ui->finder->findText();

    if (editor && editor->hasSelectedText() && !findText.isEmpty())
    {
        int fromLine = -1;
        int fromIndex = -1;

        int toLine = -1;
        int toIndex = -1;

        editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);

        // TODO: Setting POSIX
        bool result = editor->findFirstInSelection(findText, ui->finder->regex(), ui->finder->caseSensitive(), ui->finder->wholeWord(), true, true, true);

        if (result == false)
        {
            QString message = tr("Find: '%1' not found in selection (%2:%3 to %4%5).").arg(findText).arg(fromLine).arg(fromIndex).arg(toLine).arg(toIndex);
            ui->statusBar->showMessage(message, 5000);
        }

        int count = 0;

        while (result)
        {
            editor->replace(ui->finder->replaceText());
            result = editor->findNext();

            ++count;
        }

        ui->statusBar->showMessage(tr("%1 occurrences of the specified text have been replaced.").arg(count), 5000);
    }
}

void MainWindow::replaceAll()
{
    int count = 0;

    while (this->replace() == true)
    {
        ++count;
    }

    ui->statusBar->showMessage(tr("%1 occurrences of the specified text have been replaced.").arg(count), 5000);
}

// TODO: Refactor
// FIXME: Cursor position not updated
// TODO: Accept new lines on regex
bool MainWindow::findOnEditor(bool forward)
{
    bool result = false;

    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    QString findText = ui->finder->findText();

    if (editor && !findText.isEmpty())
    {
        bool select = ui->finder->selectToFind();

        int line = -1;
        int index = -1;

        editor->getCursorPosition(&line, &index);

        int fromLine = -1;
        int fromIndex = -1;

        int toLine = -1;
        int toIndex = -1;

        editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);

        if (!forward)
        {
            if ((line > fromLine) || (line == fromLine && index > fromIndex))
            {
                line = fromLine;
                index = fromIndex;
            }
        }

        // TODO: Setting POSIX
        result = editor->findFirst(findText, ui->finder->regex(), ui->finder->caseSensitive(), ui->finder->wholeWord(), ui->finder->wrap(), forward, line, index, true, true);

        if (result == false)
        {
            ui->statusBar->showMessage(tr("Find: '%1' not found.").arg(findText), 5000);
        }

        if (result && select)
        {
            int endLine = -1;
            int endIndex = -1;

            editor->getSelection(&line, &index, &endLine, &endIndex);

            if ((fromLine > line) || (fromLine == line && fromIndex > index))
            {
                fromLine = line;
                fromIndex = index;
            }

            if ((toLine < endLine) || (toLine == endLine && toIndex < endIndex))
            {
                toLine = endLine;
                toIndex = endIndex;
            }

            if (fromLine >= 0 && fromIndex >= 0 && toLine >= 0 && toIndex >= 0)
            {
                editor->setSelection(fromLine, fromIndex, toLine, toIndex);
                editor->ensureLineVisible(forward ? toLine : fromLine);
            }
        }
    }

    return result;
}

void MainWindow::gotoLineIndex()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        GotoDialog gd(this);

        gd.setMaximumLine(editor->lines());

        if (gd.exec() == GotoDialog::Accepted)
        {
            int line = gd.line() - 1;

            if (line < 0)
            {
                line = 0;
            }

            if (line >= editor->lines())
            {
                line = editor->lines() - 1;
            }

            int index = gd.index();

            if (index < 0)
            {
                index = 0;
            }

            int lineLength = editor->lineLength(line);

            if (index >= lineLength)
            {
                index = lineLength - 1;
            }

            editor->setCursorPosition(line, index);
        }
    }
}

void MainWindow::moveUp()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int line = -1;
        int index = -1;

        editor->getCursorPosition(&line, &index);

        --line;

        if (line < 0 || line >= editor->lines())
        {
            return;
        }

        QString text = editor->text(line);
        editor->deleteLine(line);

        if (line == editor->lines() - 1)
        {
            QString ending = editor->eolString();

            QString lastLine = editor->text(line);

            if (!lastLine.isEmpty() && lastLine != ending)
            {
                text = text.left(text.length() - ending.length());
                text.prepend(ending);
            }
        }

        editor->insertAt(text, line + 1, 0);
        editor->setCursorPosition(line - 1, index);
    }
}

void MainWindow::moveDown()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int line = -1;
        int index = -1;

        editor->getCursorPosition(&line, &index);

        if (line < 0 || line >= editor->lines())
        {
            return;
        }

        QString text = editor->text(line);
        editor->deleteLine(line);

        if (line == editor->lines() - 1)
        {
            QString ending = editor->eolString();

            QString lastLine = editor->text(line);

            if (!lastLine.isEmpty() && lastLine != ending)
            {
                text = text.left(text.length() - ending.length());
                text.prepend(ending);
            }
        }

        editor->insertAt(text, line + 1, 0);
        editor->setCursorPosition(line + 1, index);
    }
}

void MainWindow::duplicateLine()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int line = editor->getCurrentLine();
        QString text = editor->text(line);
        editor->insertAt(text, line + 1, 0);
    }
}

void MainWindow::cutLine()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int line = editor->getCurrentLine();
        QString text = editor->text(line);
        editor->deleteLine(line);

        qApp->clipboard()->setText(text);
    }
}

void MainWindow::copyLine()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        QString text = editor->text(editor->getCurrentLine());
        qApp->clipboard()->setText(text);
    }
}

void MainWindow::deleteLine()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->deleteLine(editor->getCurrentLine());
    }
}

// FIXME: Does not work
void MainWindow::splitLines()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QStringList lines = editor->selectedText().split(editor->eolString());
        QStringList newLines;

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            int c = editor->edgeColumn();

            while (line.length() > c)
            {
                int cc = line.indexOf(" ", -c);

                if (cc == -1)
                {
                    cc = line.indexOf(" ", c);

                    if (cc == -1)
                    {
                        cc = c;
                    }
                }

                newLines.append(line.left(cc));
                line.remove(0, cc);
            }

            if (line.isEmpty())
            {
                newLines.append(line);
            }
        }

        editor->replaceSelectedText(newLines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::joinLines()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        editor->replaceSelectedText(editor->selectedText().replace(editor->eolString(), " "));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::indent()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int fromLine = -1;
        int fromIndex = -1;
        int toLine = -1;
        int toIndex = -1;

        editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);

        if (fromLine > -1 && toLine > -1)
        {
            for (; fromLine <= toLine; ++fromLine)
            {
                editor->indent(fromLine);
            }
        }
    }
}

void MainWindow::unindent()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int fromLine = -1;
        int fromIndex = -1;
        int toLine = -1;
        int toIndex = -1;

        editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);

        if (fromLine > -1 && toLine > -1)
        {
            for (; fromLine <= toLine; ++fromLine)
            {
                editor->unindent(fromLine);
            }
        }
    }
}

void MainWindow::encloseSelection()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        EncloseSelectionDialog esd(this);

        if (esd.exec() == EncloseSelectionDialog::Accepted)
        {
            QString before = esd.before();
            QString after = esd.after();

            if (before.isEmpty() && after.isEmpty())
            {
                return;
            }

            int fromLine = -1;
            int fromIndex = -1;
            int toLine = -1;
            int toIndex = -1;

            editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);

            if (fromLine > -1 && fromIndex > -1 && toLine > -1 && toIndex > -1)
            {
                if (!after.isEmpty())
                {
                    editor->insertAt(after, toLine, toIndex);
                }

                if (!before.isEmpty())
                {
                    editor->insertAt(before, fromLine, fromIndex);
                }

                int pos = editor->positionFromLineIndex(toLine, toIndex) + before.length() + after.length();
                editor->lineIndexFromPosition(pos, &toLine, &toIndex);
                editor->setCursorPosition(toLine, toIndex);
            }
        }
    }
}

void MainWindow::duplicateSelection()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        QString text = editor->selectedText();

        int fromLine = -1;
        int fromIndex = -1;
        int toLine = -1;
        int toIndex = -1;

        editor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
        editor->insertAt(text, toLine, toIndex);

        int pos = editor->positionFromLineIndex(toLine, toIndex) + text.length();
        editor->lineIndexFromPosition(pos, &toLine, &toIndex);
        editor->setCursorPosition(toLine, toIndex);
        editor->setSelection(fromLine, fromIndex, toLine, toIndex);
    }
}

void MainWindow::padWithSpaces()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        int max = 0;

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (line.length() > max)
            {
                max = line.length();
            }
        }

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (line.length() < max)
            {
                line.append(QString(max - line.length(), ' '));
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::stripTrailingBlanks()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (!line.isEmpty())
            {
                while (line.length() > 0 && line.at(line.length() - 1).isSpace())
                {
                    line.chop(1);
                }
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::compressWhitespace()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        QRegExp space("[\\s]{2,}");

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (!line.isEmpty())
            {
                line = line.trimmed();

                while (line.indexOf(space) != -1)
                {
                    line.replace(space, " ");
                }
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::removeBlankLines()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());
        QStringList newLines;

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (!line.isEmpty())
            {
                newLines.append(line);
            }
        }

        editor->replaceSelectedText(newLines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::modifyLines()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        ModifyLinesDialog mld(this);

        if (mld.exec() == ModifyLinesDialog::Accepted)
        {
            QString prefix = mld.prefix();
            QString append = mld.append();

            if (prefix.isEmpty() && append.isEmpty())
            {
                return;
            }

            int lineFrom = -1;
            int indexFrom = -1;
            int lineTo = -1;
            int indexTo = -1;

            editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

            QString text = editor->selectedText();
            QStringList lines = text.split(editor->eolString());

            for (int i = 0; i < lines.length(); ++i)
            {
                QString& line = lines[i];

                if (!prefix.isEmpty())
                {
                    line.prepend(prefix);
                }

                if (!append.isEmpty())
                {
                    line.append(append);
                }
            }

            editor->replaceSelectedText(lines.join(editor->eolString()));

            editor->getCursorPosition(&lineTo, &indexTo);
            editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
        }
    }
}

void MainWindow::sortLines()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());
        QStringList newLines;

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];

            if (!line.isEmpty())
            {
                newLines.append(line);
            }
        }

        newLines.sort();

        editor->replaceSelectedText(newLines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::uppercase()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        editor->replaceSelectedText(editor->selectedText().toUpper());

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::lowercase()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        editor->replaceSelectedText(editor->selectedText().toLower());

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::invertCase()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();

        for (int i = 0; i < text.length(); ++i)
        {
            if (text[i].isLetter())
            {
                if (text[i].isUpper())
                {
                    text[i] = text[i].toLower();
                }
                else
                {
                    text[i] = text[i].toUpper();
                }
            }
        }

        editor->replaceSelectedText(text);

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::tabifySelection()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString spaces(editor->tabWidth(), ' ');
        editor->replaceSelectedText(editor->selectedText().replace(spaces, "\t"));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::untabifySelection()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString spaces(editor->tabWidth(), ' ');
        editor->replaceSelectedText(editor->selectedText().replace('\t', spaces));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::tabifyIndent()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        QRegExp spaces(QString("([ ]{%1})[^ ]").arg(editor->tabWidth()));

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];
            int pos = -1;

            while ((pos = line.indexOf(spaces)) != -1)
            {
                line.replace(pos, editor->tabWidth(), '\t');
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::untabifyIndent()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor && editor->hasSelectedText())
    {
        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        QRegExp spaces("([\t])[^\t]");

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];
            int pos = -1;

            while ((pos = line.indexOf(spaces)) != -1)
            {
                line.replace(pos, 1, QString(editor->tabWidth(), ' '));
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        editor->getCursorPosition(&lineTo, &indexTo);
        editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }
}

void MainWindow::toggleEncoding()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setUtf8(!editor->isUtf8());
    }
}

// TODO: Refactor
void MainWindow::toggleLineComment()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        bool select = true;

        if (!editor->hasSelectedText())
        {
            int lineFrom = editor->getCurrentLine();
            int indexFrom = 0;
            int indexTo = editor->text(lineFrom).remove(editor->eolString()).length();

            editor->setSelection(lineFrom, indexFrom, lineFrom, indexTo);

            select = false;
        }

        int lineFrom = -1;
        int indexFrom = -1;
        int lineTo = -1;
        int indexTo = -1;

        editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        QString text = editor->selectedText();
        QStringList lines = text.split(editor->eolString());

        QRegExp comment("^\\s*(--)+\\s*");

        for (int i = 0; i < lines.length(); ++i)
        {
            QString& line = lines[i];
            int pos = -1;
            bool commented = false;

            while ((pos = comment.indexIn(line)) != -1)
            {
                pos = comment.pos(1);

                if (pos != -1)
                {
                    line.remove(pos, 2);

                    if (!commented)
                    {
                        commented = true;
                    }
                }
            }

            if (!commented)
            {
                line.prepend("--");
            }
        }

        editor->replaceSelectedText(lines.join(editor->eolString()));

        if (select)
        {
            editor->getCursorPosition(&lineTo, &indexTo);
            editor->setSelection(lineFrom, indexFrom, lineTo, indexTo);
        }
    }
}

void MainWindow::findMachingBrace()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->selectToMatchingBrace();

        int line = -1;
        int index = -1;

        editor->getCursorPosition(&line, &index);
        editor->setCursorPosition(line, index);
    }
}

void MainWindow::selectToMatchingBrace()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->selectToMatchingBrace();
    }
}

// TODO: Setting
void MainWindow::customizeScheme()
{
}

// TODO: Setting
void MainWindow::changeDefaultFont()
{
    QFontDialog fd(QFont("Consolas", 10), this);
    disableWidgets<QCheckBox*>(&fd, 2);

    if (fd.exec() == QFontDialog::Accepted)
    {
        for (int i = 0; i < ui->tabWidget->count(); ++i)
        {
            LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(i));

            if (editor)
            {
                editor->setLexerFont(fd.selectedFont());
            }
        }
    }
}

// TODO: Setting
void MainWindow::wordWrap()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setWrapMode(editor->wrapMode() == LuaEditor::WrapNone ? LuaEditor::WrapWord : LuaEditor::WrapNone);
    }
}

// TODO: Setting
void MainWindow::longLineMarker()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setEdgeMode(editor->edgeMode() == LuaEditor::EdgeNone ? LuaEditor::EdgeLine : LuaEditor::EdgeNone);
    }
}

// TODO: Setting
void MainWindow::indentationGuides()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setIndentationGuides(editor->indentationGuides() == false ? true : false);
    }
}

// TODO: Setting
void MainWindow::showWhitespace()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setWhitespaceVisibility(editor->whitespaceVisibility() == LuaEditor::WsInvisible ? LuaEditor::WsVisible : LuaEditor::WsInvisible);
    }
}

// TODO: Setting
void MainWindow::showLineEndings()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setEolVisibility(editor->eolVisibility() == false ? true : false);
    }
}

// TODO: Setting
void MainWindow::visualBraceMatching()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setBraceMatching(editor->braceMatching() == LuaEditor::NoBraceMatch ? LuaEditor::SloppyBraceMatch : LuaEditor::NoBraceMatch);
    }
}

// TODO: Setting
void MainWindow::highlightCurrentLine()
{
}

// TODO: Setting
// FIXME: Change margin width
void MainWindow::lineNumbers()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->setMarginLineNumbers(0, editor->marginLineNumbers(0) == false ? true : false);
    }
}

void MainWindow::zoomIn()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->zoomIn();
    }
}

void MainWindow::zoomOut()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->zoomOut();
    }
}

void MainWindow::resetZoom()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        editor->resetZoom();
    }
}

void MainWindow::insertTabsAsSpaces()
{

}

void MainWindow::tabSettings()
{

}

void MainWindow::wordWrapSettings()
{

}

void MainWindow::longLineSettings()
{

}

void MainWindow::autoIndentText()
{

}

void MainWindow::autoCloseMarkupTag()
{

}

void MainWindow::escNone()
{

}

void MainWindow::escMinimize()
{

}

void MainWindow::escExit()
{

}

void MainWindow::saveSettingsOnExit()
{

}

void MainWindow::saveSettingsNow()
{

}

void MainWindow::showAbout()
{
    QMessageBox::information(this, "Slash Lua IDE", "Slash Lua IDE\nfor Transformice Lua Devs\n\nCopyright © 2013\n\nlibrarianrabbit");
}

// TODO: Refactor, use already tested editor instead of checking inside every function
void MainWindow::updateStats()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int currentLine = 0;
        int currentIndex = 0;

        editor->getCursorPosition(&currentLine, &currentIndex);

        this->updateCursorStats(currentLine, currentIndex);
        this->updateSelectionStats();
        this->updateBytesStats();
        this->updateEncoding();
        this->updateInputState(editor->overwriteMode());
    }
    else
    {
        sLines->setText("Ln - : -");
        sCols->setText("Col - : -");
        sSel->setText("Sel -");
        sBytes->setText("-");
        sEncoding->setText("-");
        sInput->setText("-");
    }
}

void MainWindow::updateCursorStats(int line, int index)
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        int currentLine = line + 1;
        int currentIndex = index + 1;

        int maxLine = editor->lines();
        int maxIndex = editor->edgeColumn();

        sLines->setText(QString("Ln %1 : %2").arg(currentLine).arg(maxLine));
        sCols->setText(QString("Col %1 : %2").arg(currentIndex).arg(maxIndex));
    }
}

void MainWindow::updateSelectionStats()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        sSel->setText(QString("Sel %1").arg(editor->selectedText().length()));
    }
}

void MainWindow::updateBytesStats()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        double len = editor->length();
        QString unit = formatByteUnit(len);

        sBytes->setText(QString("%1 %2").arg(len, 0, 'g', 3).arg(unit));
        this->updateTitle(ui->tabWidget->currentIndex());
    }
}

void MainWindow::updateEncoding()
{
    LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());

    if (editor)
    {
        sEncoding->setText(editor->isUtf8() ? "UTF-8" : "ASCII");
    }
}

void MainWindow::updateInputState(bool mode)
{
    sInput->setText(mode ? "OVR" : "INS");
}

void MainWindow::updateFinderState(bool visible)
{
    sFind->setArrowType(visible ? Qt::DownArrow : Qt::UpArrow);
}

// TODO: Add recent files support
// TODO: Add about Scintilla and QScintilla2
void MainWindow::setupInterface()
{
    this->setupStatusBar();

    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newTab()));
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionReload, &QAction::triggered, this, &MainWindow::reloadFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionSaveCopy, &QAction::triggered, this, &MainWindow::saveFileCopy);

    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(ui->actionCloseAll, &QAction::triggered, this, &MainWindow::closeAllFiles);

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);

    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionCopyAll, &QAction::triggered, this, &MainWindow::copyAll);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::clear);
    connect(ui->actionClearClipboard, &QAction::triggered, this, &MainWindow::clearClipboard);
    connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAll);

    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFinder);
    connect(ui->actionFindPrevious, &QAction::triggered, this, &MainWindow::findPrev);
    connect(ui->actionFindNext, &QAction::triggered, this, &MainWindow::findNext);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showFinder);
    connect(ui->actionGoto, &QAction::triggered, this, &MainWindow::gotoLineIndex);

    connect(ui->actionMoveUp, &QAction::triggered, this, &MainWindow::moveUp);
    connect(ui->actionMoveDown, &QAction::triggered, this, &MainWindow::moveDown);
    connect(ui->actionDuplicateLine, &QAction::triggered, this, &MainWindow::duplicateLine);
    connect(ui->actionCutLine, &QAction::triggered, this, &MainWindow::cutLine);
    connect(ui->actionCopyLine, &QAction::triggered, this, &MainWindow::copyLine);
    connect(ui->actionDeleteLine, &QAction::triggered, this, &MainWindow::deleteLine);
    connect(ui->actionSplitLines, &QAction::triggered, this, &MainWindow::splitLines);
    connect(ui->actionJoinLines, &QAction::triggered, this, &MainWindow::joinLines);

    connect(ui->actionIndent, &QAction::triggered, this, &MainWindow::indent);
    connect(ui->actionUnindent, &QAction::triggered, this, &MainWindow::unindent);
    connect(ui->actionEncloseSelection, &QAction::triggered, this, &MainWindow::encloseSelection);
    connect(ui->actionDuplicateSelection, &QAction::triggered, this, &MainWindow::duplicateSelection);
    connect(ui->actionPadWithSpaces, &QAction::triggered, this, &MainWindow::padWithSpaces);
    connect(ui->actionStripTrailingBlanks, &QAction::triggered, this, &MainWindow::stripTrailingBlanks);
    connect(ui->actionCompressWhitespace, &QAction::triggered, this, &MainWindow::compressWhitespace);
    connect(ui->actionRemoveBlankLines, &QAction::triggered, this, &MainWindow::removeBlankLines);
    connect(ui->actionModifyLines, &QAction::triggered, this, &MainWindow::modifyLines);
    connect(ui->actionSortLines, &QAction::triggered, this, &MainWindow::sortLines);

    connect(ui->actionUppercase, &QAction::triggered, this, &MainWindow::uppercase);
    connect(ui->actionLowercase, &QAction::triggered, this, &MainWindow::lowercase);
    connect(ui->actionInvertCase, &QAction::triggered, this, &MainWindow::invertCase);
    connect(ui->actionTabifySelection, &QAction::triggered, this, &MainWindow::tabifySelection);
    connect(ui->actionUntabifySelection, &QAction::triggered, this, &MainWindow::untabifySelection);
    connect(ui->actionTabifyIndent, &QAction::triggered, this, &MainWindow::tabifyIndent);
    connect(ui->actionUntabifyIndent, &QAction::triggered, this, &MainWindow::untabifyIndent);
    connect(ui->actionToggleEncoding, &QAction::triggered, this, &MainWindow::toggleEncoding);

    connect(ui->actionToggleLineComment, &QAction::triggered, this, &MainWindow::toggleLineComment);
    connect(ui->actionFindMatchingBrace, &QAction::triggered, this, &MainWindow::findMachingBrace);
    connect(ui->actionSelectToMatchingBrace, &QAction::triggered, this, &MainWindow::selectToMatchingBrace);

    connect(ui->actionCustomizeScheme, &QAction::triggered, this, &MainWindow::customizeScheme);
    connect(ui->actionDefaultFont, &QAction::triggered, this, &MainWindow::changeDefaultFont);
    connect(ui->actionWordWrap, &QAction::triggered, this, &MainWindow::wordWrap);
    connect(ui->actionLongLineMarker, &QAction::triggered, this, &MainWindow::longLineMarker);
    connect(ui->actionIndentationGuides, &QAction::triggered, this, &MainWindow::indentationGuides);
    connect(ui->actionShowWhitespace, &QAction::triggered, this, &MainWindow::showWhitespace);
    connect(ui->actionShowLineEndings, &QAction::triggered, this, &MainWindow::showLineEndings);
    connect(ui->actionVisualBraceMatching, &QAction::triggered, this, &MainWindow::visualBraceMatching);
    connect(ui->actionHighlightCurrentLine, &QAction::triggered, this, &MainWindow::highlightCurrentLine);
    connect(ui->actionLineNumbers, &QAction::triggered, this, &MainWindow::lineNumbers);
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(ui->actionResetZoom, &QAction::triggered, this, &MainWindow::resetZoom);

    connect(ui->actionResetZoom, &QAction::triggered, this, &MainWindow::resetZoom);
    connect(ui->actionTabSettings, &QAction::triggered, this, &MainWindow::tabSettings);
    connect(ui->actionWordWrapSettings, &QAction::triggered, this, &MainWindow::wordWrapSettings);
    connect(ui->actionLongLineSettings, &QAction::triggered, this, &MainWindow::longLineSettings);
    connect(ui->actionAutoIndentText, &QAction::triggered, this, &MainWindow::autoIndentText);
    connect(ui->actionAutoCloseMarkupTag, &QAction::triggered, this, &MainWindow::autoCloseMarkupTag);
    connect(ui->actionEscNone, &QAction::triggered, this, &MainWindow::escNone);
    connect(ui->actionEscMinimize, &QAction::triggered, this, &MainWindow::escMinimize);
    connect(ui->actionEscExit, &QAction::triggered, this, &MainWindow::escExit);
    connect(ui->actionSaveSettingsOnExit, &QAction::triggered, this, &MainWindow::saveSettingsOnExit);
    connect(ui->actionSaveSettingsNow, &QAction::triggered, this, &MainWindow::saveSettingsNow);

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::updateTitle);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::updateStats);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    connect(ui->finder, &Finder::visibilityChanged, this, &MainWindow::updateFinderState);
    connect(ui->finder, &Finder::findNotification, [this](){ this->findOnEditor(true); });
    connect(ui->finder, &Finder::findPrevNotification, this, &MainWindow::findPrev);
    connect(ui->finder, &Finder::findNextNotification, this, &MainWindow::findNext);
    connect(ui->finder, &Finder::replaceNotification, this, &MainWindow::replace);
    connect(ui->finder, &Finder::replacePrevNotification, this, &MainWindow::replacePrev);
    connect(ui->finder, &Finder::replaceNextNotification, this, &MainWindow::replaceNext);
    connect(ui->finder, &Finder::replaceInSelNotification, this, &MainWindow::replaceInSel);
    connect(ui->finder, &Finder::replaceAllNotification, this, &MainWindow::replaceAll);

    ui->finder->hide();
}

void MainWindow::setupEditor(LuaEditor* editor)
{
    if (editor)
    {
        connect(editor, &LuaEditor::cursorPositionChanged, this, &MainWindow::updateCursorStats);
        connect(editor, &LuaEditor::selectionChanged, this, &MainWindow::updateSelectionStats);
        connect(editor, &LuaEditor::textChanged, this, &MainWindow::updateBytesStats);
        connect(editor, &LuaEditor::overwriteModeChanged, this, &MainWindow::updateInputState);
        connect(editor, &LuaEditor::encodingChanged, this, &MainWindow::updateEncoding);
    }
}

void MainWindow::setupStatusBar()
{
    sLines = new QLabel(ui->statusBar);
    sCols = new QLabel(ui->statusBar);
    sSel = new QLabel(ui->statusBar);
    sBytes = new QLabel(ui->statusBar);
    sEncoding = new QClickableLabel(ui->statusBar);
    sInput = new QLabel(ui->statusBar);
    sFind = new QToolButton(ui->statusBar);

    connect(sEncoding, &QClickableLabel::doubleClicked, this, &MainWindow::toggleEncoding);

    sFind->setArrowType(Qt::UpArrow);
    sFind->setAutoRaise(true);
    sFind->setToolTip(tr("Find..."));
    connect(sFind, &QToolButton::clicked, this, &MainWindow::toggleFinder);

    ui->statusBar->addPermanentWidget(sLines);
    ui->statusBar->addPermanentWidget(sCols);
    ui->statusBar->addPermanentWidget(sSel);
    ui->statusBar->addPermanentWidget(sBytes);
    ui->statusBar->addPermanentWidget(sEncoding);
    ui->statusBar->addPermanentWidget(sInput);
    ui->statusBar->addPermanentWidget(sFind);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!this->closeAllFiles())
    {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::updateTitle(int index)
{
    if (index == -1 && ui->tabWidget->count() > 0)
    {
        index = ui->tabWidget->currentIndex();
    }

    if (index >= 0)
    {
        LuaEditor* editor = qobject_cast<LuaEditor*>(ui->tabWidget->widget(index));
        bool modified = true;

        if (!editor)
        {
            editor = qobject_cast<LuaEditor*>(ui->tabWidget->currentWidget());
        }

        if (editor)
        {
            modified = editor->isModified();
        }

        ui->tabWidget->setTabIcon(index, (modified ? fileRed : fileBlue));
        this->setWindowTitle((modified ? "*" : "") + ui->tabWidget->tabText(index) + " - Slash Lua IDE");
    }
    else
    {
        this->setWindowTitle("Slash Lua IDE");
    }
}
