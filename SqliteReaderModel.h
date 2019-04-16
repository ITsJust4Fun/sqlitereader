#ifndef SQLITEREADERMODEL_H
#define SQLITEREADERMODEL_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QTimer>

#include "DBException.h"
#include "UnsupportedDBException.h"
#include "UnreachableDBException.h"

class SqliteReaderModel : public QObject
{
    Q_OBJECT

public:
    SqliteReaderModel();
    void fillListFromSqlQuery(QList<QStringList> &dbCopy, QSqlQuery &query);
    void clearModel();
    void startRequest(QSqlQuery &query, const QString &request);
    virtual ~SqliteReaderModel();
    QTimer *timer;
    const int SYNC_TIME = 1000;

public slots:
    void connectToDatabase(const QString &path);
    void makeRequest(QString &request);
    void syncDatabase();
    void changeFilter(int column, const QString &filter);

signals:
    void queryReady(const QList<QStringList> &db, const QStringList &dbColumns);
    void dbUnreachable(const DBException &e);

private:
    QSqlDatabase db_;
    QSqlQuery *query_ = nullptr;
    QString last_request_ = "";
    QStringList db_tables_;
    QStringList db_columns_;
    QStringList filter_list_;
};

#endif // SQLITEREADERMODEL_H
