#include <QtWidgets>

#include "codeeditor.h"
#include "linenumberarea.h"

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    line_number_area = new LineNumberArea(this);

    // Line number area signals & slots
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));

    // Initialize the line number area width and highlight
    updateLineNumberAreaWidth(0);

    // Initialize selections by notifying editor that cursor position changed
    onCursorPositionChanged();

    // Set the font that the editor should use
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(12);
    this->setFont(font);

    // Set the tab width
    const int tab_stop = 4;
    QFontMetrics metrics(font);
    this->setTabStopWidth(tab_stop * metrics.width(' '));

    // Set the syntax highlighter for the editor
    highlighter = new Highlighter(this->document());
}

CodeEditor::CodeEditor(QStringList lines, QWidget *parent) : CodeEditor(parent)
{
    for (QString line : lines)
    {
        moveCursor(QTextCursor::End);
        insertPlainText(line);
        insertPlainText("\n");
        moveCursor(QTextCursor::End);
    }

    moveCursor(QTextCursor::Start);
}

// Calculates the width of the LineNumberArea widget
int CodeEditor::lineNumberAreaWidth()
{
    // Calculates the number of digits in the last line of the editor
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    // Multiplies the number of digits by the width of the font characters
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

// Updates the width of the line number area
void CodeEditor::updateLineNumberAreaWidth(int /*new_block_count*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

// Invoked when the editor's viewport is scrolled.
// rect is the part of the editing area that is to be updated (redrawn).
// dy is the number of pixels the view has been scrolled vertically.
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        line_number_area->scroll(0, dy);
    else
        line_number_area->update(0, rect.y(), line_number_area->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    // Resize the line number area on editor resize
    QRect cr = contentsRect();
    line_number_area->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::onCursorPositionChanged()
{
    QList<QTextEdit::ExtraSelection> selections;

    highlightCurrentLine(selections);
    matchParentheses(selections);

    setExtraSelections(selections);
}

// Highlights the line the cursor is currently on
void CodeEditor::highlightCurrentLine(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor line_color = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(line_color);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
}

// Called from LineNumberArea whenever it receives a paint event
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // Paint the widget's background
    QPainter painter(line_number_area);
    painter.fillRect(event->rect(), Qt::lightGray);

    // Loop through all visible lines and paint the line numbers in
    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingGeometry(block).height();
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(block_number + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, line_number_area->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++block_number;
    }
}

// Counts and displays matching parentheses
void CodeEditor::matchParentheses(QList<QTextEdit::ExtraSelection> &selections)
{
    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());
    if (data)
    {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int cur_pos = textCursor().position() - textCursor().block().position();
            if (info->position == cur_pos - 1 && info->character == '(')
            {
                if (matchLeftParenthesis(textCursor().block(), i + 1, 0, selections))
                    createParenthesisSelection(pos + info->position, selections);
            }
            else if (info->position == cur_pos - 1 && info->character == ')')
            {
                if (matchRightParenthesis(textCursor().block(), i - 1, 0, selections))
                    createParenthesisSelection(pos + info->position, selections);
            }
        }
    }
}

bool CodeEditor::matchLeftParenthesis(QTextBlock current_block, int i, int num_left_parentheses,
                                      QList<QTextEdit::ExtraSelection> &selections)
{
    TextBlockData *data = static_cast<TextBlockData *>(current_block.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int doc_pos = current_block.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == '(')
        {
            ++num_left_parentheses;
            continue;
        }

        if (info->character == ')' && num_left_parentheses == 0)
        {
            createParenthesisSelection(doc_pos + info->position, selections);
            return true;
        }
        else
        {
            --num_left_parentheses;
        }
    }

    current_block = current_block.next();
    if (current_block.isValid())
        return matchLeftParenthesis(current_block, 0, num_left_parentheses, selections);

    return false;
}

bool CodeEditor::matchRightParenthesis(QTextBlock current_block, int i, int num_right_parentheses,
                                       QList<QTextEdit::ExtraSelection> &selections)
{
    TextBlockData *data = static_cast<TextBlockData *>(current_block.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int doc_pos = current_block.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == ')')
        {
            ++num_right_parentheses;
            continue;
        }

        if (info->character == '(' && num_right_parentheses == 0)
        {
            createParenthesisSelection(doc_pos + info->position, selections);
            return true;
        }
        else
        {
            --num_right_parentheses;
        }
    }

    current_block = current_block.previous();
    if (current_block.isValid())
        return matchRightParenthesis(current_block, 0, num_right_parentheses, selections);

    return false;
}

void CodeEditor::createParenthesisSelection(int pos, QList<QTextEdit::ExtraSelection> &selections)
{
    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(Qt::green);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);
}
