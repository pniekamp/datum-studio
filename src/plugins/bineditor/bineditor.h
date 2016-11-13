//
// BinEditor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <QAbstractScrollArea>
#include <QToolBar>

//-------------------------- BinEditor --------------------------------------
//---------------------------------------------------------------------------

class BinEditor : public QAbstractScrollArea
{
  Q_OBJECT

  public:
    BinEditor(QWidget *parent = nullptr);
    virtual ~BinEditor();

    static constexpr int BytesPerLine = 16;

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  protected:

    void refresh();

    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    bool m_readonly;

    QToolBar *m_toolbar;

    struct DataChunk
    {
      uint64_t filepos;
      uint32_t datasize;

      int startline;
      int linecount;
    };

    std::vector<DataChunk> m_chunks;

    int m_ascent;
    int m_descent;
    int m_charwidth;
    int m_charmargin;

    int m_lines;
    int m_lineheight;
    int m_linesperpage;
    int m_contentwidth;

    Studio::Document *m_document;
};

