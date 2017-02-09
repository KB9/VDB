#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor)
    {
        code_editor = editor;
    }

    QSize sizeHint() const override
    {
        return QSize(code_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        code_editor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *code_editor;
};

#endif // LINENUMBERAREA_H
