#ifndef SQLITEREADERCONTROLLER_H
#define SQLITEREADERCONTROLLER_H

#include <QFileDialog>
#include <QLineEdit>

class SqliteReaderController : public QObject
{
    Q_OBJECT

public:
    SqliteReaderController();
    virtual ~SqliteReaderController();

public slots:
    void fileOpen(const QString &path);
    void onTextChanged(const QString &text);

signals:
    void fileOpened(const QString &path);
    void requestReady(QString &request);
    void filterChanged(int column, const QString &filter);
};

#endif // SQLITEREADERCONTROLLER_H
