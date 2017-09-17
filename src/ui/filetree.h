#ifndef FILETREE_H
#define FILETREE_H

#include <QTreeWidget>
#include <QStringList>

#include "filetabs.h"

#include "dwarf/CUHeader.hpp"

class FileTree : public QTreeWidget
{
    Q_OBJECT

public:
    FileTree(QWidget *parent = 0);

    void populate(std::vector<CUHeader> &cu_headers);

signals:
    void onFileSelected(QString filepath);

private slots:
    void onFileClicked(QTreeWidgetItem *item, int column);

private:
    void insertFile(QStringList path_parts, QTreeWidgetItem *item);
};

#endif // FILETREE_H
