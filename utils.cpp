#include "utils.h"
#include <QString>
#include <QMap>

static const char* bytesUnit[] = {"bytes", "KB", "MB", "GB", "TB"};

QString formatByteUnit(double &bytes)
{
    int count = 0;
    int maxCount = sizeof(bytesUnit) / sizeof(const char*);

    while (bytes >= 1024 && count < maxCount)
    {
        bytes /= 1024;
        ++count;
    }

    if (count >= maxCount)
    {
        count = maxCount - 1;
    }

    return QString::fromLatin1(bytesUnit[count]);
}

static QMap<QChar, QChar> escapeTable;
static const QString hexTable = "0123456789abcdef";

QString unespace(const QString& text)
{
    if (text.isEmpty())
    {
        return QString();
    }

    if (escapeTable.isEmpty())
    {
        escapeTable['a'] = '\a'; // Bell (beep)
        escapeTable['b'] = '\b'; // Backspace
        escapeTable['f'] = '\f'; // Formfeed
        escapeTable['n'] = '\n'; // New line
        escapeTable['r'] = '\r'; // Carriage return
        escapeTable['t'] = '\t'; // Horizontal tab
        escapeTable['"'] = '"'; // Double quotation mark
        escapeTable['\''] = '\''; // Single quotation mark
        escapeTable['\\'] = '\\'; // Backslash
    }

    QString escaped = text;
    int pos = 0;

    pos = escaped.indexOf('\\', pos);

    while (pos != -1 && escaped.length() > pos)
    {
        int epos = pos++;

        if (escaped.length() <= pos)
        {
            break;
        }

        QChar e = escaped.at(pos);

        if (escapeTable.contains(e))
        {
            e = escapeTable.value(e, e);
            escaped.replace(epos, 2, e);
        }
        else if (e == 'x')
        {
            QString hex = escaped.mid(pos + 1);

            int i = 0;

            for (; i < 2 && hex.length() > i; ++i)
            {
                if (!hexTable.contains(hex.at(i), Qt::CaseInsensitive))
                {
                    break;
                }
            }

            hex = hex.left(i);

            e = escaped.at(pos);

            bool ok = false;

            short num = hex.toShort(&ok, 16);

            if (ok)
            {
                escaped.replace(epos, i + 2, QChar(num));
            }
        }
        else if (e.isDigit())
        {
            QString decimal = escaped.mid(pos);

            int i = 0;

            for (; i < 3 && decimal.length() > i; ++i)
            {
                if (!decimal.at(i).isDigit())
                {
                    break;
                }
            }

            decimal = decimal.left(i);

            bool ok = false;

            short num = decimal.toShort(&ok, 10);

            if (ok)
            {
                escaped.replace(epos, i + 1, QChar(num));
            }
        }

        pos = escaped.indexOf('\\', pos);
    }

    return escaped;
}
