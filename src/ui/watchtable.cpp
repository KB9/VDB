#include "watchtable.h"

#include <QTableWidgetItem>

#include <cstring>

WatchTable::WatchTable(QWidget *parent)
{
    setColumnCount(2);

    addWatchRow();

    connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(onWatchVarChanged(int, int)));
}

void WatchTable::setDebugEngine(DebugEngine *debug_engine)
{
    this->debug_engine = debug_engine;
}

void WatchTable::onWatchVarChanged(int row, int column)
{
    if (column > 0) return;

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
        // Generate a GetValueMessage and wait for the result via polling
        std::unique_ptr<GetValueMessage> msg = std::unique_ptr<GetValueMessage>(new GetValueMessage());
        msg->variable_name = item->text().toStdString();
        debug_engine->sendMessage(std::move(msg));

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

void WatchTable::onValueDeduced(const std::string& variable_name,
                                const std::string& value)
{
    for (int i = 0; i < rowCount() - 1; i++)
    {
        std::string cell_text = item(i, 0)->text().toStdString();
        if (cell_text == variable_name)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString::fromStdString(value));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            setItem(i, 1, item);
        }
    }
}
