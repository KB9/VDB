#include "filetabs.h"

FileTabs::FileTabs(QWidget *parent) : QTabWidget(parent)
{
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabClosePressed(int)));
}

int FileTabs::addTab(QWidget *widget, const QIcon &icon, const QString &label)
{
    int index = QTabWidget::addTab(widget, icon, label);
    return index;
}

int FileTabs::addTab(QWidget *widget, const QString &label)
{
    int index = QTabWidget::addTab(widget, label);
    return index;
}

void FileTabs::onTabClosePressed(int index)
{
    removeTab(index);
}
