#ifndef FILETREE_H
#define FILETREE_H

#include <QTreeWidget>
#include <QStringList>

class FileTree : public QTreeWidget
{
    Q_OBJECT

public:
    FileTree(QWidget *parent = 0);
};

#endif // FILETREE_H
