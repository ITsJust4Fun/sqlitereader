#include "SqliteReaderModel.h"

SqliteReaderModel::SqliteReaderModel()
{
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    timer = new QTimer();
    timer->setInterval(SYNC_TIME);
    timer->start();
}

/*
 * Из query заполняется двумерный список строк
 * с учётом фильтров.
*/
void SqliteReaderModel::fillListFromSqlQuery(QList<QStringList> &dbCopy, QSqlQuery &query)
{
    query.first();
    query.previous();
    while (query.next()) {
        QStringList tableRow;
        bool filter = true;
        for (int i = 0; i < db_columns_.size(); i++) {
            QString value = query.value(i).toString();
            filter = filter && value.contains(filter_list_[i]);
            tableRow.append(value);
        }
        if (filter) {
            dbCopy.append(tableRow);
        }
    }
}

/*
 * Установка соединения с бд и получение
 * списка таблиц и колонок. Если что-то идёт не так, то
 * выбрасываются исключения.
 * Приложение работает только с первой таблицей в бд.
*/
void SqliteReaderModel::connectToDatabase(const QString &path)
{
    if (db_.isOpen()) {
        clearModel();
    }
    db_.setDatabaseName(path);
    db_.open();
    if (!db_.isOpen()) {
        throw UnreachableDBException();
    }
    query_ = new QSqlQuery(db_);
    db_tables_ = db_.tables();
    if (db_tables_.size() > 0) {
        try {
            startRequest(*query_, QString("pragma table_info({})").replace("{}", db_tables_[0]));
        } catch (UnreachableDBException e) {
            emit dbUnreachable(e);
            return;
        }
        while (query_->next()) {
            db_columns_.append(query_->value(1).toString());
            filter_list_.append("");
        }
    } else {
        clearModel();
        throw UnsupportedDBException();
    }
}

/*
 * Запрос к бд.
 * Если поизошла ошибка, то
 * выбрасывается исключение
*/
void SqliteReaderModel::startRequest(QSqlQuery &query, const QString &request)
{
    if (!query.exec(request)) {
        clearModel();
        throw UnreachableDBException();
    }
}

/*
 * Переход модели в изначальное
 * состояние
*/
void SqliteReaderModel::clearModel()
{
    db_.close();
    delete query_;
    query_ = nullptr;
    db_columns_.clear();
    filter_list_.clear();
}

/*
 * Запрос к бд.
 * Обратотка исключений в случае ошибок.
 * Формирование двумерного списка строк
 * для отображения в таблицу
*/
void SqliteReaderModel::makeRequest(QString &request)
{
    if (!query_) {
        return;
    }
    request.replace("{}", db_tables_[0]);
    try {
        startRequest(*query_, request);
    } catch (UnreachableDBException e) {
        emit dbUnreachable(e);
        return;
    }
    last_request_ = request;
    QList<QStringList> dbCopy;
    fillListFromSqlQuery(dbCopy, *query_);
    emit queryReady(dbCopy, db_columns_);
}

/*
 * Фиксирование изменений фильтров и
 * создание двумерного списка строк для
 * отображения в таблице с учётом фильтров.
*/
void SqliteReaderModel::changeFilter(int column, const QString &filter)
{
    filter_list_[column] = filter;
    QList<QStringList> dbCopy;
    QStringList emptyList;
    fillListFromSqlQuery(dbCopy, *query_);
    emit queryReady(dbCopy, emptyList);
}

/*
 * Синхронизация бд с программой.
 * Выполняется по таймеру.
 * Создаются 2 двумерных списка строк
 * и сравниваются. Если занчения не совпадают, то
 * таблица обновляется.
*/
void SqliteReaderModel::syncDatabase()
{
    if (!query_) {
        return;
    }
    QSqlQuery query(db_);
    try {
        startRequest(query, last_request_);
    } catch (UnreachableDBException e) {
        emit dbUnreachable(e);
        return;
    }
    QList<QStringList> dbCopyOld;
    QList<QStringList> dbCopyNew;
    fillListFromSqlQuery(dbCopyOld, *query_);
    fillListFromSqlQuery(dbCopyNew, query);
    bool isEqual = dbCopyNew.size() == dbCopyOld.size();
    if (isEqual) {
        for (int i = 0; i < dbCopyNew.size(); i++) {
            isEqual = isEqual && (dbCopyNew[i] == dbCopyOld[i]);
        }
    }
    if (!isEqual) {
        dbCopyOld.clear();
        try {
            startRequest(*query_, last_request_);
        } catch (UnreachableDBException e) {
            emit dbUnreachable(e);
            return;
        }
        QStringList emptyList;
        emit queryReady(dbCopyNew, emptyList);
    }
}

SqliteReaderModel::~SqliteReaderModel()
{
    delete timer;
    if (db_.isOpen()) {
        db_.close();
    }
    if (query_) {
        delete query_;
    }
}
