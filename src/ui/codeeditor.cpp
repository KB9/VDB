#include <QFileInfo>
#include <QTextStream>
#include <QPainter>

#include "codeeditor.h"
#include "linenumberarea.h"

CodeEditor::CodeEditor(QString filepath, std::shared_ptr<VDB> vdb, QWidget *parent) : QPlainTextEdit(parent)
{
    this->filepath = filepath;
    this->vdb = vdb;

    line_number_area = new LineNumberArea(this);

    // Line number area signals & slots
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));

    connect(line_number_area, SIGNAL(lineNumberPressed(unsigned int)), this, SLOT(toggleBreakpoint(unsigned int)));

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

    // Disable line wrapping
    setLineWrapMode(NoWrap);

    // Set the tab width
    const int tab_stop = 4;
    QFontMetrics metrics(font);
    this->setTabStopWidth(tab_stop * metrics.width(' '));

    // Set the syntax highlighter for the editor
    highlighter = new Highlighter(this->document());

    QStringList lines;
    bool ok = readFile(filepath, lines);
    if (ok)
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
}

bool CodeEditor::readFile(QString filepath, QStringList &lines)
{
    // Ensure the path exists and is actually a file
    QFile source_file(filepath);
    QFileInfo info(source_file);
    if (info.isDir() || !info.exists()) return false;

    // Open the file and read in the text line-by-line
    if (source_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&source_file);
        while (!in.atEnd())
        {
            lines.append(in.readLine());
        }
        source_file.close();
    }
    return true;
}

// Updates the width of the line number area
void CodeEditor::updateLineNumberAreaWidth(int /*new_block_count*/)
{
    setViewportMargins(line_number_area->getWidth(), 0, 0, 0);
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
    line_number_area->setGeometry(QRect(cr.left(), cr.top(), line_number_area->getWidth(), cr.height()));
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
    unsigned int block_number = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingGeometry(block).height();
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(block_number + 1);
            painter.setPen(Qt::black);

            // Color the number's background red if it has an assigned breakpoint
            std::shared_ptr<DebugEngine> engine = vdb->getDebugEngine();
            if (engine->isBreakpoint(this->filepath.toStdString().c_str(), block_number + 1))
            {
                painter.fillRect(QRectF(0, top, line_number_area->width(), fontMetrics().height()), Qt::red);
            }

            // Draw the line number
            painter.drawText(0, top, line_number_area->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++block_number;
    }
}

// Returns the line number from a given y-position
unsigned int CodeEditor::getLineNumberFromY(int y)
{
    QTextBlock block = firstVisibleBlock();
    int total_height = 0;
    while (block.isValid() && block.isVisible())
    {
        QRectF rect = block.layout()->boundingRect();
        if (y >= total_height && y < (total_height + rect.height()))
        {
            return block.blockNumber() + 1;
        }
        total_height += rect.height();
        block = block.next();
    }
    return -1;
}

void CodeEditor::toggleBreakpoint(unsigned int line_number)
{
    std::shared_ptr<DebugEngine> engine = vdb->getDebugEngine();
    if (engine->isBreakpoint(this->filepath.toStdString().c_str(), line_number))
    {
        engine->removeBreakpoint(filepath.toStdString().c_str(), line_number);
    }
    else
    {
        engine->addBreakpoint(filepath.toStdString().c_str(), line_number);
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

void CodeEditor::goToLine(unsigned int line_number)
{
    QTextDocument *document = this->document();
    QTextCursor cursor(document->findBlockByLineNumber(line_number - 1));
    this->setTextCursor(cursor);
}

QString CodeEditor::getFilePath()
{
    return filepath;
}
