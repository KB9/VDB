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

    // Read the text from the text file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox((QMessageBox::Icon)0, "Error", file.errorString());
    }

    // Initialize the text stream
    QTextStream in(&file);

    // Get the contents of the text file line by line
    QStringList text_lines;
    while (!in.atEnd())
    {
        text_lines.append(in.readLine());
    }

    // Close the file
    file.close();

    ui->fileTabWidget->addTab(new CodeEditor(text_lines), filename);
}
