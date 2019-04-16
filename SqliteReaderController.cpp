#include "SqliteReaderController.h"

SqliteReaderController::SqliteReaderController()
{

}

void SqliteReaderController::fileOpen(const QString &path)
{
    if(path == "") {
        return;
    }
    emit fileOpened(path);
    QString request = "select * from {}";
    emit requestReady(request);
}

void SqliteReaderController::onTextChanged(const QString &text)
{
    QLineEdit *line = qobject_cast<QLineEdit*>(sender());
    int column = line->property("column").toInt();
    emit filterChanged(column, text);
}

SqliteReaderController::~SqliteReaderController()
{

}
