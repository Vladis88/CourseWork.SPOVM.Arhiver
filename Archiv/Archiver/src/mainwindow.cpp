#include <QFileSystemModel>
#include <QDir>
#include <QDesktopServices>
#include <QTreeView>
#include <QLayout>
#include <QLineEdit>
#include <QDebug>
#include <QMessageBox>
#include <QModelIndexList>

#include "src/AddWindow.h"
#include "src/ExtractWindow.h"
#include "src/mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); //для построения дерева виджетов на родительском виджете

    init();     //инициализация окон ADD_WINDOW и EXTRACT_WINDOW
    createFileSystemModel(); //создание фаловой системы
    setQLineEditToToolBar(); //взаимодействие с панелью инструментов QLineEditToToolBar

    //Связываем между собой объекты используя сигналы и слоты
    connect(toolBarLineEdit, SIGNAL(returnPressed()),this, SLOT(EnterPressed()));
    connect(this->ui->actionUp_on_level, SIGNAL(triggered()), this, SLOT(upOnLevel())); //связываем upOnLevel() с actionUp_on_level
    connect(this, SIGNAL(emitText(const QString)), m_mapPages.value(ADD_WINDOW), SLOT(recvText(const QString))); //связываем ADD_WINDOW и MainWindow для установки пути
    connect(this, SIGNAL(emitText(const QString)), m_mapPages.value(EXTRACT_WINDOW), SLOT(recvText(const QString))); //связываем ADD_WINDOW и MainWindow для установки пути
}

MainWindow::~MainWindow()
{
    delete ui;
    if(!m_mapPages.isEmpty())
        m_mapPages.clear();
}

void MainWindow::setQLineEditToToolBar()
{
    //установка QLineEdit в QToolBar
    toolBarLineEdit = new QLineEdit();
    spacer = new QWidget(this);
    spacer->setFixedWidth(9);
    ui->subToolBar->addWidget(toolBarLineEdit);
    ui->subToolBar->addWidget(spacer);
}

void MainWindow::createFileSystemModel()
{
    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllEntries);
    model->setRootPath("");
    this->ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->ui->listView->setModel(model);
}

void MainWindow::init()
{
    m_mapPages.insert(ADD_WINDOW, new Add_window); //добавляем элементы Add_window
    m_mapPages.insert(EXTRACT_WINDOW, new Extract_window); //добавляем элементы Extract_window

    for(auto it = m_mapPages.begin(); it != m_mapPages.end(); ++it)
    {
        it.value()->setModal(true); // инициализируем
    }
}

void MainWindow::on_actionAdd_files_to_archive_triggered()
{
    QString path = toolBarLineEdit->text();
    QStringList pathList = path.split(".");

    QItemSelectionModel *selectionModel = ui->listView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if(selectedIndexes.size() <= 0||
            toolBarLineEdit->text() == "C:/"||
            toolBarLineEdit->text() == "D:/" ||
            toolBarLineEdit->text() == "E:/") {
        m_mapPages.value(ADD_WINDOW)->exec();
    }
    else if(pathList.back() == "txt" || pathList.back() == "cpp" || pathList.back() == "java")
    {
        m_mapPages.value(ADD_WINDOW)->show();
        qDebug()<<toolBarLineEdit->text();
        emit emitText(toolBarLineEdit->text());
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Warning", "Choose .txt/.cpp/.java file");
        msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        msgBox.exec();
    }
}

void MainWindow::on_actionExtract_to_triggered()
{
    QString path = toolBarLineEdit->text();
    QStringList pathList = path.split(".");

    //создаём экземпляры классов для проверки и получения нужного нам файла и пути к нему
    QItemSelectionModel *selectionModel = ui->listView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    //проверяем в какой мы директории
    if(selectedIndexes.size() <= 0||
            toolBarLineEdit->text() == "C:/"||
            toolBarLineEdit->text() == "D:/" ||
            toolBarLineEdit->text() == "E:/") {
        //если мы не заходили не в какую директорию, то путь указываем сами в новом окне
        m_mapPages.value(EXTRACT_WINDOW)->exec();
    } //иначе проверяем правильный ли файл выбран
    else if(pathList.back() == "rle" || pathList.back() == "huf")
    {
        m_mapPages.value(EXTRACT_WINDOW)->show();
        qDebug()<<toolBarLineEdit->text();
        emit emitText(toolBarLineEdit->text());
    } else {//если файл не выбран, то выводим ою этом сообщение
        QMessageBox msgBox(QMessageBox::Warning, "Warning", "Choose .rle/.huf file");
        msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        msgBox.exec();
    }
}


void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QFileInfo fileInfo = model->fileInfo(index);
    toolBarLineEdit->setText(fileInfo.absoluteFilePath());
    toolBarTextBeforeChanged = toolBarLineEdit->text();
    qDebug() << toolBarTextBeforeChanged;
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    QFileInfo fileInfo = model->fileInfo(index);
    if(fileInfo.fileName() == "..")
    {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        this->ui->listView->setRootIndex(model->index(dir.absolutePath()));
    }
    else if(fileInfo.fileName() == ".")
    {
        this->ui->listView->setRootIndex(model->index(""));
    }
    else if(fileInfo.isDir())
    {
        this->ui->listView->setRootIndex(index);
    }
    toolBarTextBeforeChanged = toolBarLineEdit->text();
    toolBarLineEdit->setText(fileInfo.absoluteFilePath());
}

void MainWindow::upOnLevel()
{
    this->ui->listView->setRootIndex(this->ui->listView->rootIndex().parent());
    toolBarLineEdit->setText(model->fileInfo(this->ui->listView->rootIndex()).absolutePath());
}

void MainWindow::EnterPressed()
{
    if(QDir(toolBarLineEdit->text()).exists())
    {
        toolBarTextBeforeChanged += toolBarLineEdit->text();
        this->ui->listView->setRootIndex(model->index(toolBarLineEdit->text()));
    }
    else
    {
        QString errorMsg = "Path \""  + toolBarLineEdit->text() + "\" is not accessimble";
        QMessageBox::critical(this, tr("Error"), errorMsg);
        toolBarLineEdit->setText(toolBarTextBeforeChanged);
    }
}

void MainWindow::synchronizeListFromEdit()
{
    qDebug()<<toolBarLineEdit->text();
    if(QDir(toolBarLineEdit->text()).exists())
    {
        this->ui->listView->setRootIndex(model->index(toolBarLineEdit->text()));
        qDebug()<<toolBarLineEdit->text();
    }
}


void MainWindow::on_actionDelete_file_triggered()
{

    if(toolBarLineEdit->text() == "" ||
            toolBarLineEdit->text() == "C:/"||
            toolBarLineEdit->text() == "D:/")
        return;

    QFileInfo fileInfo(toolBarLineEdit->text());

    if(fileInfo.isDir())
    {
        QDir dir(toolBarLineEdit->text());
        dir.removeRecursively();
    }
    else if (fileInfo.isFile())
    {
        QDir dir(toolBarLineEdit->text());
        dir.remove(toolBarLineEdit->text());
    }
}
