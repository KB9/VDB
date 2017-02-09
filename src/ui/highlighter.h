#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

struct ParenthesisInfo
{
    char character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses();
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlighting_rules;

    QRegExp comment_start_expression;
    QRegExp comment_end_expression;

    QTextCharFormat keyword_format;
    QTextCharFormat class_format;
    QTextCharFormat single_line_comment_format;
    QTextCharFormat multi_line_comment_format;
    QTextCharFormat quotation_format;
    QTextCharFormat function_format;
};

#endif // HIGHLIGHTER_H
