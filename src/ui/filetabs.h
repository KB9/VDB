#ifndef FILETABS_H
#define FILETABS_H

#include <QTabWidget>

#include <assert.h>

class FileTabs : public QTabWidget
{
    Q_OBJECT

public:
    FileTabs(QWidget *parent = 0);

    int addTab(QWidget *widget, const QIcon &icon, const QString &label);
    int addTab(QWidget *widget, const QString &label);

private slots:
    void onTabClosePressed(int index);
};

#endif // FILETABS_H
