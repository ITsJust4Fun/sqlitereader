#include "SqliteReaderView.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SqliteReaderView w;
    w.show();

    return a.exec();
}
