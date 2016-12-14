//
// TextEditor Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "texteditor.h"
#include "commandbar.h"
#include "assetfile.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTextBlock>

#include <QDebug>

using namespace std;

//|---------------------- LineNumberArea ------------------------------------
//|--------------------------------------------------------------------------

class LineNumberArea : public QWidget
{
  public:
    LineNumberArea(TextEditor *editor)
      : QWidget(editor),
        editor(editor)
    {
    }

    void paintEvent(QPaintEvent *event)
    {
      editor->lineNumberAreaPaintEvent(event);
    }

    TextEditor *editor;
};



//|---------------------- TextEditor ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TextEditor::Constructor ///////////////////////////
TextEditor::TextEditor(QWidget *parent)
  : QPlainTextEdit(parent),
    m_document(nullptr)
{
  m_filepos = 0;
  
  m_toolbar = new CommandBar(this);

  m_linenumberarea = new LineNumberArea(this);

  setFont(QFont("Courier"));
  setLineWrapMode(QPlainTextEdit::NoWrap);

  int linenumberareawidth = 3 + fontMetrics().width(QLatin1Char('9')) * 5;

  setViewportMargins(linenumberareawidth, 0, 0, 0);

  connect(this, &QPlainTextEdit::textChanged, this, &TextEditor::on_text_changed);
  connect(this, &QPlainTextEdit::updateRequest, this, &TextEditor::on_update_request);
}


///////////////////////// TextEditor::Destructor ////////////////////////////
TextEditor::~TextEditor()
{
  delete m_toolbar;
}


///////////////////////// TextEditor::toolbar ///////////////////////////////
QToolBar *TextEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// TextEditor::view //////////////////////////////////
void TextEditor::view(Studio::Document *document)
{
  setReadOnly(true);

  m_document = document;

  connect(m_document, &Studio::Document::document_changed, this, &TextEditor::refresh);

  refresh();
}


///////////////////////// TextEditor::edit //////////////////////////////////
void TextEditor::edit(Studio::Document *document)
{
  setReadOnly(false);

  m_document = document;

  connect(m_document, &Studio::Document::document_changed, this, &TextEditor::refresh);

  refresh();
}


///////////////////////// TextEditor::on_text_changed ///////////////////////
void TextEditor::on_text_changed()
{
  if (m_document)
  {
    QByteArray data = toPlainText().toUtf8();

    disconnect(m_document, &Studio::Document::document_changed, this, &TextEditor::refresh);

    m_document->lock_exclusive();

    auto position = m_filepos;

    position += write_text_asset(m_document, position, 1, data.size(), data.data());

    position += write_footer(m_document, position);

    m_document->set_metadata("build", buildtime());

    m_document->unlock_exclusive();

    connect(m_document, &Studio::Document::document_changed, this, &TextEditor::refresh);
  }
}


///////////////////////// TextEditor::on_update_request /////////////////////
void TextEditor::on_update_request(const QRect &rect, int dy)
{
  if (dy)
    m_linenumberarea->scroll(0, dy);
  else
    m_linenumberarea->update(0, rect.y(), m_linenumberarea->width(), rect.height());
}


///////////////////////// TextEditor::resizeEvent ///////////////////////////
void TextEditor::resizeEvent(QResizeEvent *event)
{
  auto contentsrect = contentsRect();
  auto viewportmargins = viewportMargins();

  m_linenumberarea->setGeometry(QRect(contentsrect.left(), contentsrect.top(), viewportmargins.left(), contentsrect.height()));

  QPlainTextEdit::resizeEvent(event);
}


///////////////////////// TextEditor::lineNumberAreaPaintEvent //////////////
void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
  QPainter painter(m_linenumberarea);
  painter.fillRect(event->rect(), QColor(240, 240, 240));

  QTextBlock block = firstVisibleBlock();

  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  int linenumber = block.blockNumber();

  while (block.isValid() && top <= event->rect().bottom())
  {
    if (block.isVisible() && bottom >= event->rect().top())
    {
      painter.setPen(Qt::black);
      painter.drawText(0, top, m_linenumberarea->width()-2, fontMetrics().height(), Qt::AlignRight, QString::number(linenumber + 1));
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();

    ++linenumber;
  }
}


///////////////////////// TextEditor::refresh ///////////////////////////////
void TextEditor::refresh()
{
  if (!m_document)
    return;

  QByteArray data;
  QByteArray olddata = toPlainText().toUtf8();

  m_document->lock();

  PackTextHeader text;

  if ((m_filepos = read_asset_header(m_document, 1, &text)))
  {
    data.resize(text.length);

    read_asset_payload(m_document, text.dataoffset, data.data(), data.size());
  }

  m_document->unlock();

  blockSignals(true);

  auto i = data.begin();
  auto j = olddata.begin();

  auto cursor = QTextCursor(document());

  while (i != data.end())
  {
    auto si = i;

    while (i != data.end() && (j == olddata.end() || *i != *j))
    {
      ++i;
    }

    if (i != si)
    {
      cursor.setPosition(si - data.begin());
      cursor.insertText(QByteArray(si, i - si));
    }

    while (i != data.end() && j != data.end() && *i == *j)
    {
      ++i;
      ++j;
    }
  }

  if (j != olddata.end())
  {
    cursor.setPosition(i - data.begin(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
  }

  if (olddata.size() == 0)
  {
    moveCursor(QTextCursor::Start);
    document()->clearUndoRedoStacks();
  }

  blockSignals(false);
}

