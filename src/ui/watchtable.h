#ifndef WATCHTABLE_H
#define WATCHTABLE_H

#include <QTableWidget>

#include "vdb.hpp"

class WatchTable : public QTableWidget
{
    Q_OBJECT

public:
    WatchTable(QWidget *parent = 0);

    void setDebugEngine(DebugEngine *debug_engine);

private slots:
    void onWatchVarChanged(int row, int column);

private:
    DebugEngine *debug_engine = nullptr;

    void addWatchRow();
};

#endif // WATCHTABLE_H
