#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>

#include "vdb.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void actionImportExecutable();

    void onFileSelected(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;

    VDB vdb;
};

#endif // MAINWINDOW_H
