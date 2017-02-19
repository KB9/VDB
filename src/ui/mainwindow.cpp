#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>

#include "codeeditor.h"

#include <iostream>
#include <memory>

#include "CUHeader.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->fileTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(onFileSelected(QTreeWidgetItem*,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actionImportExecutable()
{
    // Open a file dialog to select the appropriate file
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "/home/kavan", tr("ELF executables (*)"));

    // Check if the file is executable
    if (!QFileInfo(filename).isExecutable()) return;

    // Create the debugger
    vdb.run(filename.toStdString().c_str());

    // Clear the files window
    ui->fileTreeWidget->clear();

    // Find the associated source files
    std::shared_ptr<DwarfDebug> dwarf = vdb.getDwarfDebugData();
    for (std::shared_ptr<CUHeader> header : dwarf->info()->getCUHeaders())
    {
        QString filename = QString(header->root_die->getName().c_str());
        QString directory = QString(header->root_die->getCompDir().c_str());

        QTreeWidgetItem *file_item = new QTreeWidgetItem((QTreeWidget *)0, QStringList(filename));
        if (ui->fileTreeWidget->findItems(directory, 0).size() == 0)
        {
            QTreeWidgetItem *dir_item = new QTreeWidgetItem((QTreeWidget *)0, QStringList(directory));
            dir_item->addChild(file_item);
            ui->fileTreeWidget->insertTopLevelItem(0, dir_item);
        }
        else
        {
            QTreeWidgetItem *dir_item = ui->fileTreeWidget->findItems(directory, 0)[0];
            dir_item->addChild(file_item);
        }
    }
}

void MainWindow::onFileSelected(QTreeWidgetItem *item, int column)
{
    // Get the name of the file selected
    QString filename = item->text(column);

    // Build the absolute path from all higher children
    QString absolute_path = item->text(column);
    while (item->parent())
    {
        item = item->parent();
        absolute_path = item->text(column) + "/" + absolute_path;
    }

    // Ensure the path exists and is actually a file
    QFile source_file(absolute_path);
    QFileInfo info(source_file);
    if (info.isDir() || !info.exists()) return;

    // Open the file and read in the text line-by-line
    QStringList lines;
    if (source_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&source_file);
        while (!in.atEnd())
        {
            lines.append(in.readLine());
        }
        source_file.close();
    }

    // Add a new code editor tab
    ui->fileTabWidget->addTab(new CodeEditor(lines), filename);
}
