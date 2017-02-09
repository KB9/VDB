#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>

#include "codeeditor.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
}
