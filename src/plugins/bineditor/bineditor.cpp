//
// BinEditor Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "bineditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QScrollBar>
#include <QPainter>

#include <QDebug>

using namespace std;

const int BytesPerLine = 16;

//|---------------------- BinEditor -----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// BinEditor::Constructor ////////////////////////////
BinEditor::BinEditor(QWidget *parent)
  : QAbstractScrollArea(parent),
    m_document(nullptr)
{
  m_readonly = true;

  m_toolbar = new CommandBar(this);

  setFont(QFont("Courier"));
}


///////////////////////// BinEditor::Destructor /////////////////////////////
BinEditor::~BinEditor()
{
  delete m_toolbar;
}


///////////////////////// BinEditor::toolbar ////////////////////////////////
QToolBar *BinEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// BinEditor::view ///////////////////////////////////
void BinEditor::view(Studio::Document *document)
{
  m_readonly = true;

  m_document = document;

  connect(m_document, &Studio::Document::document_changed, this, &BinEditor::refresh);

  refresh();
}


///////////////////////// BinEditor::edit ///////////////////////////////////
void BinEditor::edit(Studio::Document *document)
{
  m_readonly = false;

  m_document = document;

  connect(m_document, &Studio::Document::document_changed, this, &BinEditor::refresh);

  refresh();
}


///////////////////////// BinEditor::refresh ////////////////////////////////
void BinEditor::refresh()
{
  if (!m_document)
    return;

  m_chunks.clear();

  m_document->lock();

  uint64_t position = sizeof(PackHeader);

  while (true)
  {
    PackChunk chunk;

    m_document->read(position, &chunk, sizeof(chunk));

    if (chunk.type == "HEND"_packchunktype)
      break;

    if (chunk.type == "DATA"_packchunktype)
    {
      DataChunk data;

      data.filepos = position + sizeof(chunk);
      data.datasize = chunk.length;

      m_chunks.push_back(data);
    }

    position += chunk.length + sizeof(chunk) + sizeof(uint32_t);
  }

  m_chunks.erase(m_chunks.begin());

  m_document->unlock();

  QFontMetrics tm = fontMetrics();

  m_ascent = tm.ascent();
  m_descent = tm.descent();
  m_charwidth = tm.averageCharWidth();
  m_charmargin = tm.averageCharWidth()/2;

  m_contentwidth = (13 + 3*BytesPerLine) * m_charwidth + BytesPerLine * (m_charmargin + m_charmargin);

  horizontalScrollBar()->setRange(0, m_contentwidth - viewport()->width());
  horizontalScrollBar()->setPageStep(viewport()->width());

  m_lines = 0;
  m_lineheight = tm.lineSpacing();
  m_linesperpage = viewport()->height() / m_lineheight;

  for(auto &chunk : m_chunks)
  {
    chunk.startline = m_lines;
    chunk.linecount = 3 + chunk.datasize / BytesPerLine + 1;

    m_lines += chunk.linecount;
  }

  verticalScrollBar()->setRange(0, m_lines - m_linesperpage);
  verticalScrollBar()->setPageStep(m_linesperpage);

  viewport()->update();
}


///////////////////////// BinEditor::resizeEvent ////////////////////////////
void BinEditor::resizeEvent(QResizeEvent *event)
{
  refresh();
}


///////////////////////// BinEditor::paintEvent /////////////////////////////
void BinEditor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int line = verticalScrollBar()->value();
  int column = horizontalScrollBar()->value();

  int x = -column;
  int y = -4;

  painter.setPen(palette().text().color());

  for(auto &chunk : m_chunks)
  {
    if (chunk.startline <= line && line < chunk.startline + chunk.linecount)
    {
      int cy = y + (chunk.startline - line) * m_lineheight;

      cy += m_lineheight;

      painter.drawText(x + m_charwidth, cy + m_ascent, QString("Data Block : %1 bytes").arg(chunk.datasize));

      cy += m_lineheight;

      painter.drawLine(x + m_charwidth, cy + 4, x + m_contentwidth - m_charwidth - m_charwidth, cy + 4);

      cy += m_lineheight;

      painter.drawLine(x + 10 * m_charwidth + m_charmargin, cy, x + 10 * m_charwidth + m_charmargin, cy + (chunk.linecount - 3) * m_lineheight);

      for(int i = 2; i < 2*BytesPerLine; i += 4)
      {
        QRect rect(x + 11 * m_charwidth + i * (m_charwidth + m_charmargin), cy, 2*(m_charwidth + m_charmargin), (chunk.linecount - 3) * m_lineheight);

        painter.fillRect(event->rect() & rect, palette().alternateBase());
      }
    }

    m_document->lock();

    uint64_t bufferpos = -1;
    uint8_t buffer[4096];

    while (chunk.startline <= line && line < chunk.startline + chunk.linecount && y - m_lineheight < event->rect().bottom())
    {
      if (line >= chunk.startline + 3)
      {
        uint64_t filepos = chunk.filepos + (line - chunk.startline - 3) * BytesPerLine;

        QString addressstr = QString("%1:%2").arg(filepos >> 16, 4, 16, QLatin1Char('0')).arg(filepos & 0xFFFF, 4, 16, QLatin1Char('0'));

        painter.drawText(x + m_charwidth, y + m_ascent, addressstr);

        if (filepos < bufferpos || filepos + BytesPerLine >= bufferpos + sizeof(buffer))
        {
          bufferpos = filepos;

          m_document->read(filepos, buffer, sizeof(buffer));
        }

        QString ascii = "";
        for(int i = 0; i < min(BytesPerLine, (int)(chunk.filepos + chunk.datasize - filepos)); ++i)
        {
          auto data = buffer[filepos + i - bufferpos];

          QString bytestr = QString("%1").arg(data, 2, 16, QLatin1Char('0'));

          painter.drawText(x + 11 * m_charwidth + i * 2 * (m_charwidth + m_charmargin) + m_charmargin, y + m_ascent, bytestr);

          ascii += QString("%1").arg(QChar(data).isPrint() ? QChar(data) : QChar('.'));
        }

        painter.drawText(x + 11 * m_charwidth + BytesPerLine * 2*(m_charwidth + m_charmargin) + m_charmargin, y + m_ascent, ascii);
      }

      y += m_lineheight;

      line += 1;
    }

    m_document->unlock();
  }
}

