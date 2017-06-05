#ifndef WATCHTABLE_H
#define WATCHTABLE_H

#include <QTableWidget>

class WatchTable : public QTableWidget
{
    Q_OBJECT

public:
    WatchTable(QWidget *parent = 0);

private slots:
    void onWatchVarChanged(int row, int column);

private:
    void addWatchRow();
};

#endif // WATCHTABLE_H
