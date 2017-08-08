#ifndef FILETABS_H
#define FILETABS_H

#include <QTabWidget>

#include "codeeditor.h"

#include <memory>
#include <assert.h>

class FileTabs : public QTabWidget
{
    Q_OBJECT

public:
    FileTabs(QWidget *parent = 0);

    bool goToSourceLine(QString filepath, unsigned int line_number);

private slots:
    void onTabClosePressed(int index);
};

#endif // FILETABS_H
