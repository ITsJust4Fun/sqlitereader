#include "SqliteReader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SqliteReader w;
    w.show();

    return a.exec();
}
