#include "watchtable.h"

#include <QTableWidgetItem>

WatchTable::WatchTable(QWidget *parent)
{
    setColumnCount(2);

    addWatchRow();

    connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(onWatchVarChanged(int, int)));
}

void WatchTable::onWatchVarChanged(int row, int column)
{
    QTableWidgetItem *item = this->item(row, column);
    if (item->text().isEmpty())
    {
        // If the row is not the first or last row, remove it
        if (rowCount() > 1 && row < (rowCount() - 1))
            removeRow(row);
    }
    // Only add a new row if there is no row above it
    else if (row == (rowCount() - 1))
    {
        addWatchRow();
    }
}

void WatchTable::addWatchRow()
{
    int row = rowCount();
    insertRow(row);

    // Disable editing for the second column
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    setItem(row, 1, item);
}
