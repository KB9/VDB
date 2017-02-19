#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QVector>

#include "codeeditor.h"

class CodeEditor;

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

    int getWidth() const;

    QVector<int> &getBreakpoints();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    CodeEditor *code_editor;

    QVector<int> breakpoint_line_numbers;
};

#endif // LINENUMBERAREA_H
