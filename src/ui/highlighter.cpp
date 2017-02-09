#include "highlighter.h"

TextBlockData::TextBlockData()
{

}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}

void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}

Highlighter::Highlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keyword_format.setForeground(Qt::darkBlue);
    keyword_format.setFontWeight(QFont::Bold);
    QStringList keyword_patterns;
    keyword_patterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                     << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                     << "\\bfriend\\b" << "\\binline\\b" << "\\bexplicit\\b"
                     << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                     << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                     << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                     << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                     << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                     << "\\bunion\\b" << "\\bunsigned\\b" << "\\btypename\\b"
                     << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bint\\b"
                     << "\\bunsigned\\b" << "\\bvirtual\\b" << "\\boverride\\b";
    foreach (const QString &pattern, keyword_patterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keyword_format;
        highlighting_rules.append(rule);
    }

    /*
    class_format.setFontWeight(QFont::Bold);
    class_format.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = class_format;
    highlighting_rules.append(rule);
    */

    quotation_format.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotation_format;
    highlighting_rules.append(rule);

    function_format.setFontItalic(true);
    function_format.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = function_format;
    highlighting_rules.append(rule);

    single_line_comment_format.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = single_line_comment_format;
    highlighting_rules.append(rule);

    multi_line_comment_format.setForeground(Qt::darkGreen);

    comment_start_expression = QRegExp("/\\*");
    comment_end_expression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlighting_rules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int start_index = 0;
    if (previousBlockState() != 1)
        start_index = comment_start_expression.indexIn(text);

    while (start_index >= 0)
    {
        int end_index = comment_end_expression.indexIn(text, start_index);
        int comment_length;
        if (end_index == -1)
        {
            setCurrentBlockState(1);
            comment_length = text.length() - start_index;
        }
        else
        {
            comment_length = end_index - start_index + comment_end_expression.matchedLength();
        }
        setFormat(start_index, comment_length, multi_line_comment_format);
        start_index = comment_start_expression.indexIn(text, start_index + comment_length);
    }

    // Parenthesis matching below

    TextBlockData *data = new TextBlockData;

    int left_pos = text.indexOf('(');
    while (left_pos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = '(';
        info->position = left_pos;

        data->insert(info);
        left_pos = text.indexOf('(', left_pos + 1);
    }

    int right_pos = text.indexOf(')');
    while (right_pos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ')';
        info->position = right_pos;

        data->insert(info);
        right_pos = text.indexOf(')', right_pos + 1);
    }

    setCurrentBlockUserData(data);
}
