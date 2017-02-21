#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QVector>
#include <QWidget>

#include "codeeditor.h"

class CodeEditor;

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

    int getWidth() const;

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

signals:
    void lineNumberPressed(unsigned int);

private:
    CodeEditor *code_editor;
};

#endif // LINENUMBERAREA_H
