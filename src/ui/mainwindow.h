#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTimer>

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

public slots:
    void onFileSelected(QString filepath);

private slots:
    void importExecutable();
    void startDebugging();

    void stepOver();
    void stepInto();
    void stepOut();

    void pollDebugEngine();

private:
    void setDebugButtonEnabled(bool enabled, QString text = 0);
    void setBreakpointStepControlsEnabled(bool enabled);

    Ui::MainWindow *ui;

    std::shared_ptr<VDB> vdb = nullptr;

    QTimer *timer;
};

#endif // MAINWINDOW_H
