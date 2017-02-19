#include "filetree.h"

FileTree::FileTree(QWidget *parent) : QTreeWidget(parent)
{
    setColumnCount(1);

    /*
    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i)
    {
        QStringList strings;
        strings.append(QString("item %1").arg(i));
        strings.append(QString("another item %1").arg(i));
        items.append(new QTreeWidgetItem((QTreeWidget *)0, strings));
    }
    insertTopLevelItems(0, items);
    */
}
