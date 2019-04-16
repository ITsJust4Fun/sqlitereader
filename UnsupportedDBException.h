#ifndef UNSUPPORTEDDBEXCEPTION_H
#define UNSUPPORTEDDBEXCEPTION_H

#include "DBException.h"

class UnsupportedDBException : public DBException
{
public:
    UnsupportedDBException();
    virtual ~UnsupportedDBException();

};

#endif // UNSUPPORTEDDBEXCEPTION_H
