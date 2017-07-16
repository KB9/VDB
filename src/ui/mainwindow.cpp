#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>

#include "codeeditor.h"

#include "dwarf/CUHeader.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->fileTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(onFileSelected(QTreeWidgetItem*,int)));

    vdb = std::make_shared<VDB>();

    // Initialize the polling timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pollDebugEngine()));

    // Disable the breakpoint step controls until a breakpoint is hit
    setBreakpointStepControlsEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pollDebugEngine()
{
    std::unique_ptr<DebugMessage> msg = vdb->getDebugEngine()->tryPoll();
    if (msg != nullptr)
    {
        GetValueMessage *value_msg = dynamic_cast<GetValueMessage *>(msg.get());
        if (value_msg != nullptr)
        {
            ui->watchTable->onValueDeduced(value_msg->variable_name, value_msg->value);
        }

        BreakpointHitMessage *bph_msg = dynamic_cast<BreakpointHitMessage *>(msg.get());
        if (bph_msg != nullptr)
        {
            // Enable breakpoint controls so that breakpoint action can be taken
            ui->debugButton->setEnabled(true);
            setBreakpointStepControlsEnabled(true);

            // TODO: Should open source file if not already open in editor
            QString filename = QString::fromStdString(bph_msg->file_name);
            for (int i = 0; i < ui->fileTabWidget->count(); i++)
            {
                // TODO: This is a really poor test for a matching source files.
                // Will need to change this once file tree setup is improved.
                QString tab_text = ui->fileTabWidget->tabText(i);
                if (filename.contains(tab_text))
                {
                    CodeEditor *editor = dynamic_cast<CodeEditor *>(ui->fileTabWidget->widget(i));
                    if (editor != nullptr)
                    {
                        editor->goToLine(bph_msg->line_number);
                    }
                }
            }
        }

        TargetExitMessage *exit_msg = dynamic_cast<TargetExitMessage *>(msg.get());
        if (exit_msg != nullptr)
        {
            // Reset breakpoint control buttons to their default state
            ui->debugButton->setText("Start Debugging");
            ui->debugButton->setEnabled(true);
            setBreakpointStepControlsEnabled(false);
        }
    }
}

void MainWindow::importExecutable()
{
    // Open a file dialog to select the appropriate file
    QFileDialog dialog;
    dialog.setFilter(QDir::Executable);
    QString filename = dialog.getOpenFileName(this, tr("Import executable"), QDir::homePath());

    // Check if the file is executable
    if (!QFileInfo(filename).isExecutable()) return;

    // Create the debugger
    vdb->init(filename.toStdString().c_str());

    // Clear the files window
    ui->fileTreeWidget->clear();

    // Find the associated source files
    std::shared_ptr<DwarfDebug> dwarf = vdb->getDwarfDebugData();
    for (CUHeader &header : dwarf->info()->getCUHeaders())
    {
        QString filename = QString(header.root_die->getName().c_str());
        QString directory = QString(header.root_die->getCompDir().c_str());

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

void MainWindow::startDebugging()
{
    // If a target process is not currently being debugged
    if (!vdb->getDebugEngine()->isDebugging())
    {
        vdb->getDebugEngine()->run();
        ui->watchTable->setDebugEngine(vdb->getDebugEngine().get());

        // Start the polling timer
        timer->start(500);

        // Disable breakpoint controls until a breakpoint is hit
        ui->debugButton->setText("Continue");
        ui->debugButton->setEnabled(false);
        setBreakpointStepControlsEnabled(false);
    }
    else
    {
        // Continue execution, disable breakpoint controls until another is hit
        vdb->getDebugEngine()->continueExecution();
        ui->debugButton->setEnabled(false);
        setBreakpointStepControlsEnabled(false);
    }
}

void MainWindow::stepOver()
{

}

void MainWindow::stepInto()
{

}

void MainWindow::stepOut()
{

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
    ui->fileTabWidget->addTab(new CodeEditor(absolute_path, lines, vdb), filename);
}

void MainWindow::setBreakpointStepControlsEnabled(bool enabled)
{
    ui->stepOverButton->setEnabled(enabled);
    ui->stepIntoButton->setEnabled(enabled);
    ui->stepOutButton->setEnabled(enabled);
}
