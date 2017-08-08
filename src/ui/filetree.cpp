#include "filetree.h"

#include <QFileInfo>

FileTree::FileTree(QWidget *parent) : QTreeWidget(parent)
{
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(onFileClicked(QTreeWidgetItem *, int)));

    setColumnCount(1);
}

void FileTree::populate(std::vector<CUHeader> &cu_headers)
{
    for (CUHeader &header : cu_headers)
    {
        QString filename = QString(header.root_die->getName().c_str());
        QString directory = QString(header.root_die->getCompDir().c_str());
        QString absPath = directory + '/' + filename;
        QStringList pathParts = absPath.split('/', QString::SkipEmptyParts);

        insertFile(pathParts, this->invisibleRootItem());
    }
}

void FileTree::insertFile(QStringList pathParts, QTreeWidgetItem *item)
{
    if (pathParts.size() == 0) return;

    bool found = false;
    for (int i = 0; i < item->childCount(); i++)
    {
        if (item->child(i)->text(0) == pathParts.at(i))
        {
            pathParts.removeAt(0);
            insertFile(pathParts, item->child(i));
            found = true;
        }
    }

    if (!found)
    {
        QTreeWidgetItem *new_item = new QTreeWidgetItem(item, QStringList(pathParts.at(0)));
        pathParts.removeAt(0);
        insertFile(pathParts, new_item);
    }
}

void FileTree::onFileClicked(QTreeWidgetItem *item, int column)
{
    // Build the absolute path by navigating up the tree
    QString absolute_path;
    while (item != invisibleRootItem() && item != nullptr)
    {
        absolute_path = '/' + item->text(0) + absolute_path;
        item = item->parent();
    }

    // Ensure that it is an existing file and not a directory
    QFileInfo info(absolute_path);
    if (!info.isDir() && info.exists())
    {
        emit onFileSelected(absolute_path);
    }
}
