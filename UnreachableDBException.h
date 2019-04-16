#ifndef UNREACHABLEDBEXCEPTION_H
#define UNREACHABLEDBEXCEPTION_H

#include "DBException.h"

class UnreachableDBException : public DBException
{
public:
    UnreachableDBException();
    virtual ~UnreachableDBException();

};

#endif // UNREACHABLEDBEXCEPTION_H
