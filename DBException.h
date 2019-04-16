#ifndef DBEXCEPTION_H
#define DBEXCEPTION_H

#include <QString>

class DBException
{
public:
    DBException();
    QString exceptionText;
    virtual ~DBException();

};

#endif // DBEXCEPTION_H
