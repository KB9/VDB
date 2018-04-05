#include "filetree.h"

#include <QFileInfo>

FileTree::FileTree(QWidget *parent) : QTreeWidget(parent)
{
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(onFileClicked(QTreeWidgetItem *, int)));

    setColumnCount(1);
}

void FileTree::populate(std::vector<SourceFile> files)
{
    for (const SourceFile &file : files)
    {
        QString filename = QString(file.name.c_str());
        QString directory = QString(file.dir.c_str());
        QString abs_path = directory + '/' + filename;
        QStringList path_parts = abs_path.split('/', QString::SkipEmptyParts);

        insertFile(path_parts, this->invisibleRootItem());
    }
}

void FileTree::insertFile(QStringList path_parts, QTreeWidgetItem *item)
{
    if (path_parts.size() == 0) return;

    bool found = false;
    for (int i = 0; i < item->childCount(); i++)
    {
        if (item->child(i)->text(0) == path_parts.at(0))
        {
            path_parts.removeAt(0);
            insertFile(path_parts, item->child(i));
            found = true;
        }
    }

    if (!found)
    {
        QTreeWidgetItem *new_item = new QTreeWidgetItem(item, QStringList(path_parts.at(0)));
        path_parts.removeAt(0);
        insertFile(path_parts, new_item);
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
