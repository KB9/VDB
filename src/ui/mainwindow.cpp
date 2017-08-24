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

    vdb = std::make_shared<VDB>();

    // Initialize the polling timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pollDebugEngine()));

    // Disable the breakpoint step controls until a breakpoint is hit
    setDebugButtonEnabled(false);
    setBreakpointStepControlsEnabled(false);

    connect(ui->fileTreeWidget, SIGNAL(onFileSelected(QString)),
            this, SLOT(onFileSelected(QString)));
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
            setDebugButtonEnabled(true);
            setBreakpointStepControlsEnabled(true);

            // Open the specified file and go to the line specified
            QString filepath = QString::fromStdString(bph_msg->file_name);
            QString filename = filepath.split('/').back();
            bool tab_found = ui->fileTabWidget->goToSourceLine(filepath, bph_msg->line_number);
            if (!tab_found)
            {
                CodeEditor *editor = new CodeEditor(filepath, vdb);
                ui->fileTabWidget->addTab(editor, filename);
                editor->goToLine(bph_msg->line_number);
            }
        }

        StepMessage *step_msg = dynamic_cast<StepMessage *>(msg.get());
        if (step_msg != nullptr)
        {
            // Enable breakpoint controls so that breakpoint action can be taken
            setDebugButtonEnabled(true);
            setBreakpointStepControlsEnabled(true);

            // Open the specified file and go to the line specified
            QString filepath = QString::fromStdString(step_msg->file_name);
            QString filename = filepath.split('/').back();
            bool tab_found = ui->fileTabWidget->goToSourceLine(filepath, step_msg->line_number);
            if (!tab_found)
            {
                CodeEditor *editor = new CodeEditor(filepath, vdb);
                ui->fileTabWidget->addTab(editor, filename);
                editor->goToLine(step_msg->line_number);
            }
        }

        TargetExitMessage *exit_msg = dynamic_cast<TargetExitMessage *>(msg.get());
        if (exit_msg != nullptr)
        {
            // Reset breakpoint control buttons to their default state
            setDebugButtonEnabled(true, "Start Debugging");
            setBreakpointStepControlsEnabled(false);
        }
    }
}

void MainWindow::onFileSelected(QString filepath)
{
    ui->fileTabWidget->addTab(new CodeEditor(filepath, vdb), filepath.split('/').back());
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
    ui->fileTreeWidget->populate(dwarf->info()->getCUHeaders());

    // Enable the debugging button once the executable's source is loaded
    setDebugButtonEnabled(true);
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
        setDebugButtonEnabled(false, "Continue");
        setBreakpointStepControlsEnabled(false);
    }
    else
    {
        // Continue execution, disable breakpoint controls until another is hit
        vdb->getDebugEngine()->continueExecution();
        setDebugButtonEnabled(false);
        setBreakpointStepControlsEnabled(false);
    }
}

void MainWindow::stepOver()
{
    if (!vdb->isInitialized()) return;
    if (vdb->getDebugEngine()->isDebugging())
        vdb->getDebugEngine()->stepOver();
}

void MainWindow::stepInto()
{
    if (!vdb->isInitialized()) return;
    if (vdb->getDebugEngine()->isDebugging())
        vdb->getDebugEngine()->stepInto();
}

void MainWindow::stepOut()
{
    if (!vdb->isInitialized()) return;
    if (vdb->getDebugEngine()->isDebugging())
        vdb->getDebugEngine()->stepOut();
}

void MainWindow::setDebugButtonEnabled(bool enabled, QString text)
{
    ui->debugButton->setEnabled(enabled);
    if (!text.isEmpty()) ui->debugButton->setText(text);
}

void MainWindow::setBreakpointStepControlsEnabled(bool enabled)
{
    ui->stepOverButton->setEnabled(enabled);
    ui->stepIntoButton->setEnabled(enabled);
    ui->stepOutButton->setEnabled(enabled);
}
