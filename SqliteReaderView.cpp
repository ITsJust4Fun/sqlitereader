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


/*
 * Данный метод настраиает окно, а именно:
 * -появилось в центре экрана (сначала нужно определить рамер экрана)
 * -название берётся из переменной APP_NAME
 * -виджет должен принимать файлы через drag'n'drop
 * -начальные(и минимальные) размеры окна хранятся в переменных
 * WIDGET_WIDTH и WIDGET_HEIGHT
*/
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

/*
 * Данный метод определяет элементы виджета
 * -создаёт grid layout и верхнее меню
 * -в лэйаут заносится таблица и верхнее меню
 * -в верхнее меню добавляется всплывающее меню File,
 * в котором есть 2 пункта (выход и открыть файл)
*/
void SqliteReaderView::initWindowElements()
{
    gridLayout = new QGridLayout();
    fileMenu = new QMenu("File");
    fileMenu->addAction("Open", this, SLOT(selectFile()), Qt::CTRL + Qt::Key_O);
    fileMenu->addAction("Quit", this, SLOT(close()), Qt::CTRL + Qt::Key_Q);
    menuBar = new QMenuBar();  //верхнее меню
    menuBar->addMenu(fileMenu);
    gridLayout->addWidget(table);
    gridLayout->setMenuBar(menuBar);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    setLayout(gridLayout);
}

/*
 * Формирование таблицы, в которой количество и названия колонок
 * берутся из бд. Каждой колонке присваивается свой фильтр,
 * которому прописывается какой колонке он соответствует.
 * Каждый фильтр связывается со слотом в контроллере, который будет
 * реагировать на изменения.
*/
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

/*
 * Установка метаобъектных связей
*/
void SqliteReaderView::makeConnections()
{
    /*
     * Отправление пути в контроллер, который выбрал пользователь
     * через меню File
    */
    QObject::connect(this, SIGNAL(fileSelected(const QString &)),
                     controller, SLOT(fileOpen(const QString &)));

    /*
     * Когда контроллер проверит на пустоту файл, он отправит
     * путь в модель, чтобы открыть бд и заполнить поля модели.
     * Модель генерирует исключения в случае ошибок.
    */
    QObject::connect(controller, SIGNAL(fileOpened(const QString &)),
                     model, SLOT(connectToDatabase(const QString &)));

    /*
     * Контроллер создаёт запрос к бд и предаёт в модель
    */
    QObject::connect(controller, SIGNAL(requestReady(QString &)),
                     model, SLOT(makeRequest(QString &)));

    /*
     * Модель на основе фильтров и данных из бд создаёт двумерный лист строк
     * и отпрвляет его в вид для отображения в таблице.
    */
    QObject::connect(model, SIGNAL(queryReady(const QList<QStringList> &, const QStringList &)),
                     this, SLOT(fillTable(const QList<QStringList> &, const QStringList &)));

    /*
     * передача фильтров и его номера колонки в модель для дальнейшего
     * изменения таблицы
    */
    QObject::connect(controller, SIGNAL(filterChanged(int, const QString &)),
                     model, SLOT(changeFilter(int, const QString &)));

    /*
     * синхронизация с бд по таймеру
    */
    QObject::connect(model->timer, SIGNAL(timeout()),
                     model, SLOT(syncDatabase()));

    /*
     * Если бд стала недоступна, то вызывается метод onError
    */
    QObject::connect(model, SIGNAL(dbUnreachable(const DBException &)),
                     this, SLOT(onError(const DBException &)));
}

/*
 * проверка файлов, которые пользователь открывает через
 * drag'n'drop. Файл должен быть один и иметь расширение *.db3
*/
void SqliteReaderView::dragEnterEvent(QDragEnterEvent *e)
{
    bool isSingleDrop = e->mimeData()->urls().size() == 1;
    QString file = e->mimeData()->urls().at(0).toLocalFile();
    bool isDatabase = file.endsWith(".db3");
    if (isSingleDrop && isDatabase) {
        e->acceptProposedAction();
    }
}

/*
 * открытие файла при drag'n'drop
*/
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

/*
 * открытие файла через верхнее меню
*/
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

/*
 * при ошибке сбрасывается таблица и выводится
 * текст ошибки
*/
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

/*
 * Добавление ряда в таблицу.
 * Так как вертикальные хедеры не стандартные
 * приходится следить за нумерацией.
*/
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

/*
 * отчистка таблицы, но не затрагивая ряд фильтров
*/
void SqliteReaderView::removeTableRows()
{
    for (int i = table->rowCount() - 1; i > 0; i--) {
        table->removeRow(i);
        table->setRowCount(i);
    }
}

/*
 * Заполнение таблиы данными из модели.
 * Если dbColumns пустой, то в таблице нужно
 * просто обновить содержимое не затрагивая фильтры.
 * Если же dbColumns не пустой, то таблицу нужно
 * сначала полностью отчистить.
 * Так же тайтл окна приводится к формату:
 * [путь_к_файлу_бд] - название_приложения
*/
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

/*
 * сброс пути к файлу и тайтла окна
*/
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
