#ifndef STACKTRACELIST_H
#define STACKTRACELIST_H

#include <QListWidget>

class StackTraceList : public QListWidget
{
    Q_OBJECT

public:
    StackTraceList(QWidget *parent = 0);
};

#endif // STACKTRACELIST_H
