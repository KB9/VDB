#include "linenumberarea.h"

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor)
{
    code_editor = editor;
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(getWidth(), 0);
}

int LineNumberArea::getWidth() const
{
    // Calculates the number of digits in the last line of the editor
    int digits = 1;
    int max_lines = qMax(1, code_editor->blockCount());
    while (max_lines >= 10)
    {
        max_lines /= 10;
        ++digits;
    }

    // Multiplies the number of digits by the width of the font characters
    int char_width = code_editor->fontMetrics().width(QLatin1Char('9'));
    int space = 3 + char_width * digits;

    return space;
}

QVector<int> &LineNumberArea::getBreakpoints()
{
    return breakpoint_line_numbers;
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    code_editor->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
    int line_number = code_editor->getLineNumberFromY(event->y());
    if (line_number < 0) return;

    if (breakpoint_line_numbers.contains(line_number))
    {
        int index = breakpoint_line_numbers.indexOf(line_number);
        breakpoint_line_numbers.remove(index);
    }
    else
    {
        breakpoint_line_numbers.push_back(line_number);
    }

    update();
}
