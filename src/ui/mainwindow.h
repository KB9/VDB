#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>

#include "vdb.hpp"

#include <memory>

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
    void importExecutable();
    void startDebugging();

    void stepOver();
    void stepInto();
    void stepOut();

    void onFileSelected(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;

    std::shared_ptr<VDB> vdb = nullptr;
};

#endif // MAINWINDOW_H
