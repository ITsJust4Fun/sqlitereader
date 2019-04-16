#include "SqliteReaderView.h"

SqliteReaderView::SqliteReaderView(QWidget *parent)
    : QWidget(parent)
{
    table = new QTableWidget();
    controller = new SqliteReaderController();
    model = new SqliteReaderModel();
    verticalHeaderLabels = new QStringList;
    initWindow();
    initWindowElements();
    makeConnections();
}

void SqliteReaderView::initWindow()
{
    screens_ = QGuiApplication::screens();
    screen_height_ = screens_[0]->size().height();
    screen_width_ = screens_[0]->size().width();
    setWindowTitle(APP_NAME);
    setAcceptDrops(true);
    setMinimumSize(QSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    setGeometry((screen_width_ / 2) - (WIDGET_WIDTH / 2),
                (screen_height_ / 2) - (WIDGET_HEIGHT / 2),
                WIDGET_WIDTH, WIDGET_HEIGHT);
}

void SqliteReaderView::initWindowElements()
{
    gridLayout = new QGridLayout;
    fileMenu = new QMenu("File");
    fileMenu->addAction("Open", this, SLOT(selectFile()), Qt::CTRL + Qt::Key_O);
    fileMenu->addAction("Quit", this, SLOT(close()), Qt::CTRL + Qt::Key_Q);
    menuBar = new QMenuBar;
    menuBar->addMenu(fileMenu);
    gridLayout->addWidget(table);
    gridLayout->setMenuBar(menuBar);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    setLayout(gridLayout);
}

void SqliteReaderView::initTable(const QStringList &columns)
{
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(columns.size());
    table->setShowGrid(true);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectColumns);
    table->setHorizontalHeaderLabels(columns);
    table->horizontalHeader()->setMinimumSectionSize(110);
    table->horizontalHeader()->sortIndicatorOrder();
    table->insertRow(table->rowCount());
    for(int i = 0; i < columns.size(); i++) {
        QLineEdit *line = new QLineEdit();
        line->setClearButtonEnabled(true);
        line->setPlaceholderText(FILTER_PLACEHOLDER);
        line->setProperty("column", i);

        QObject::connect(line, SIGNAL(textEdited(const QString &)),
                         controller, SLOT(onTextChanged(const QString &)));

        table->setCellWidget(0, i, line);
    }
    table->setVerticalHeaderLabels(*verticalHeaderLabels);
}

void SqliteReaderView::makeConnections()
{
    QObject::connect(this, SIGNAL(fileSelected(const QString &)),
                     controller, SLOT(fileOpen(const QString &)));

    QObject::connect(controller, SIGNAL(fileOpened(const QString &)),
                     model, SLOT(connectToDatabase(const QString &)));

    QObject::connect(controller, SIGNAL(requestReady(QString &)),
                     model, SLOT(makeRequest(QString &)));

    QObject::connect(model, SIGNAL(queryReady(const QList<QStringList> &, const QStringList &)),
                     this, SLOT(fillTable(const QList<QStringList> &, const QStringList &)));

    QObject::connect(controller, SIGNAL(filterChanged(int, const QString &)),
                     model, SLOT(changeFilter(int, const QString &)));

    QObject::connect(model->timer, SIGNAL(timeout()),
                     model, SLOT(syncDatabase()));

    QObject::connect(model, SIGNAL(dbUnreachable(const DBException &)),
                     this, SLOT(onError(const DBException &)));
}

void SqliteReaderView::dragEnterEvent(QDragEnterEvent *e)
{
    bool isSingleDrop = e->mimeData()->urls().size() == 1;
    QString file = e->mimeData()->urls().at(0).toLocalFile();
    bool isDatabase = file.endsWith(".db3");
    if (isSingleDrop && isDatabase) {
        e->acceptProposedAction();
    }
}

void SqliteReaderView::dropEvent(QDropEvent *e)
{
    QString path = e->mimeData()->urls().at(0).toLocalFile();
    path_ = path;
    try {
        emit fileSelected(path);
    } catch (DBException e) {
        onError(e);
    }
}

void SqliteReaderView::selectFile()
{
    QString path = QFileDialog::getOpenFileName(
              this,
              "Open db3",
              QDir::currentPath(),
              "Sqlite files (*.db3)");
    path_ = path;
    try {
        emit fileSelected(path);
    } catch (DBException e) {
        onError(e);
    }
}

void SqliteReaderView::onError(const DBException &e)
{
    QMessageBox messageBox;
    messageBox.critical(nullptr, "Error", e.exceptionText);
    messageBox.setFixedSize(500,200);
    resetPath();
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
}

void SqliteReaderView::addRowToTable(const QStringList &data)
{
    table->insertRow(table->rowCount());
    for (int i = 0; i < data.size(); i++) {
        QTableWidgetItem* item = new QTableWidgetItem(data[i]);
        item->setFlags(Qt::ItemIsEnabled |
                       Qt::ItemIsSelectable);
        table->setItem(table->rowCount() - 1, i, item);
    }
    *verticalHeaderLabels << QString::number(table->rowCount() - 1);
    table->setVerticalHeaderLabels(*verticalHeaderLabels);
}

void SqliteReaderView::removeTableRows()
{
    for (int i = table->rowCount() - 1; i > 0; i--) {
        table->removeRow(i);
        table->setRowCount(i);
    }
}

void SqliteReaderView::fillTable(const QList<QStringList> &db, const QStringList &dbColumns)
{
    setWindowTitle("[" + path_ + "] - " + APP_NAME);
    verticalHeaderLabels->clear();
    verticalHeaderLabels->append("");
    if (!dbColumns.isEmpty()) {
        initTable(dbColumns);
    }
    removeTableRows();
    for (auto row : db) {
        addRowToTable(row);
    }
}

void SqliteReaderView::resetPath()
{
    path_ = "";
    setWindowTitle(APP_NAME);
}

SqliteReaderView::~SqliteReaderView()
{
    delete table;
    delete gridLayout;
    delete fileMenu;
    delete menuBar;
    delete controller;
    delete model;
    delete verticalHeaderLabels;
}
