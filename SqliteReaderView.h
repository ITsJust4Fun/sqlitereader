#ifndef SQLITEREADERVIEW_H
#define SQLITEREADERVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QMenu>
#include <QMenuBar>
#include <QTableWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QHeaderView>
#include <QSqlQuery>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QLineEdit>
#include <QMessageBox>

#include "SqliteReaderController.h"
#include "SqliteReaderModel.h"
#include "DBException.h"

class SqliteReaderView : public QWidget
{
    Q_OBJECT

public:
    const QString APP_NAME = "SQLite Reader";  //название приложения
    const int WIDGET_HEIGHT = 400;  //минимальная высота окна
    const int WIDGET_WIDTH = 800;  //минимальная ширина окна
    const QString FILTER_PLACEHOLDER = "Filter";  //текст отображаемый на фильтрах, когда они пустые
    SqliteReaderView(QWidget *parent = nullptr);
    void initWindow();
    void initWindowElements();
    void initTable(const QStringList &columns);
    void makeConnections();
    void addRowToTable(const QStringList &data);
    void removeTableRows();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    virtual ~SqliteReaderView();
    QGridLayout *gridLayout;
    QMenu *fileMenu;
    QMenuBar *menuBar;
    QTableWidget *table;
    SqliteReaderController *controller;
    SqliteReaderModel *model;
    QStringList *verticalHeaderLabels;

public slots:
    void selectFile();
    void fillTable(const QList<QStringList> &db, const QStringList &dbColumns);
    void resetPath();
    void onError(const DBException &e);

signals:
    void fileSelected(const QString &path);

private:
    int screen_height_;
    int screen_width_;
    QList<QScreen *> screens_;
    QString path_;
};

#endif // SQLITEREADERVIEW_H
