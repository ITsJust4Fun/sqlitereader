#include "SqliteReaderController.h"

SqliteReaderController::SqliteReaderController()
{

}

/*
 * Вызывается при выбре файла бд пользователем.
 * Путь проверяется на пустоту, после файл открывается
 * в модели и формируется запрос к бд.
*/
void SqliteReaderController::fileOpen(const QString &path)
{
    if(path == "") {
        return;
    }
    emit fileOpened(path);
    QString request = "select * from {}";
    emit requestReady(request);
}

/*
 * Сообщает модели о изменении фильтров.
*/
void SqliteReaderController::onTextChanged(const QString &text)
{
    QLineEdit *line = qobject_cast<QLineEdit*>(sender());
    int column = line->property("column").toInt();
    emit filterChanged(column, text);
}

SqliteReaderController::~SqliteReaderController()
{

}
