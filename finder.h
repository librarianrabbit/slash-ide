#ifndef FINDER_H
#define FINDER_H

#include <QWidget>
#include <QTimer>

// TODO: Add better icons
// TODO: Advanced find/replace
// TODO: Setting - Search on type
namespace Ui {
    class Finder;
}

class Finder : public QWidget
{
    Q_OBJECT
    
public:
    explicit Finder(QWidget* parent = 0);
    ~Finder();

public:
    virtual QString findText();
    virtual QString replaceText();
    virtual bool selectToFind();
    virtual bool regex();
    virtual bool caseSensitive();
    virtual bool wholeWord();
    virtual bool wrap();
    virtual bool escape();

public slots:
    virtual void setFindText(const QString& text);
    virtual void setSelectToFind(bool select);
    virtual void selectFindText();

protected:
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void showEvent(QShowEvent* e);
    virtual void hideEvent(QHideEvent* e);

    virtual void setupInterface();

protected slots:
    virtual void emitFindNotification();
    virtual void emitFindPrevNotification();
    virtual void emitFindNextNotification();
    virtual void emitReplaceNotification();
    virtual void emitReplacePrevNotification();
    virtual void emitReplaceNextNotification();
    virtual void emitReplaceInSelNotification();
    virtual void emitReplaceAllNotification();

    virtual void updateFindChanged();

signals:
    void visibilityChanged(bool visible);

    void findNotification();
    void findPrevNotification();
    void findNextNotification();
    void replaceNotification();
    void replacePrevNotification();
    void replaceNextNotification();
    void replaceInSelNotification();
    void replaceAllNotification();

private:
    Ui::Finder *ui;
    bool replaceNext;
    bool findChanged;
};

#endif // FINDER_H
