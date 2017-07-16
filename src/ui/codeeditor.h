#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

#include "highlighter.h"
#include "linenumberarea.h"

#include <memory>
#include "vdb.hpp"

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QString filepath, std::shared_ptr<VDB> vdb, QWidget *parent = 0);
    CodeEditor(QString filepath, QStringList lines, std::shared_ptr<VDB> vdb, QWidget *parent = 0);

    // Called from LineNumberArea whenever it receives a paint event
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    // Returns the line number from a given y-position
    unsigned int getLineNumberFromY(int y);

    void goToLine(unsigned int line_number);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Updates the width of the line number area
    void updateLineNumberAreaWidth(int new_block_count);

    // Called when the text cursor position changes
    void onCursorPositionChanged();

    // TODO: Move this from slots
    // Highlights the line the cursor is currently on
    void highlightCurrentLine(QList<QTextEdit::ExtraSelection> &selections);

    // Invoked when the editor's viewport is scrolled.
    // rect is the part of the editing area that is to be updated (redrawn).
    // dy is the number of pixels the view has been scrolled vertically.
    void updateLineNumberArea(const QRect &, int);

    // TODO: Move this from slots
    // Counts and displays matching parentheses
    void matchParentheses(QList<QTextEdit::ExtraSelection> &selections);

    void toggleBreakpoint(unsigned int line_number);

private:
    QString filepath;
    std::shared_ptr<VDB> vdb = nullptr;

    LineNumberArea *line_number_area;
    QVector<unsigned int> breakpoints;

    Highlighter *highlighter;

    bool matchLeftParenthesis(QTextBlock current_block, int index, int num_right_parentheses,
                              QList<QTextEdit::ExtraSelection> &selections);
    bool matchRightParenthesis(QTextBlock current_block, int index, int num_left_parentheses,
                               QList<QTextEdit::ExtraSelection> &selections);
    void createParenthesisSelection(int pos, QList<QTextEdit::ExtraSelection> &selections);
};

#endif // CODEEDITOR_H
