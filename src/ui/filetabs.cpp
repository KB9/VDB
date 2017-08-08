#include "filetabs.h"

FileTabs::FileTabs(QWidget *parent) : QTabWidget(parent)
{
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabClosePressed(int)));
}

bool FileTabs::goToSourceLine(QString filepath, unsigned int line_number)
{
    for (int i = 0; i < count(); i++)
    {
        CodeEditor *editor = dynamic_cast<CodeEditor *>(this->widget(i));
        if (editor != nullptr)
        {
            if (editor->getFilePath() == filepath)
            {
                setCurrentWidget(editor);
                editor->goToLine(line_number);
                return true;
            }
        }
    }
    return false;
}

void FileTabs::onTabClosePressed(int index)
{
    removeTab(index);
}
