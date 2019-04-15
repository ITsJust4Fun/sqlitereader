#ifndef SQLITEREADER_H
#define SQLITEREADER_H

#include <QWidget>

class SqliteReader : public QWidget
{
    Q_OBJECT

public:
    SqliteReader(QWidget *parent = 0);
    ~SqliteReader();
};

#endif // SQLITEREADER_H
