//
// TextEditor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <QPlainTextEdit>
#include <QToolBar>

//-------------------------- TextEditor -------------------------------------
//---------------------------------------------------------------------------

class TextEditor : public QPlainTextEdit
{
  Q_OBJECT

  public:
    TextEditor(QWidget *parent = nullptr);
    virtual ~TextEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  protected:

    void refresh();

    void on_text_changed();

    void on_update_request(QRect const &rect, int dy);

    void resizeEvent(QResizeEvent *event);

  public:

    void lineNumberAreaPaintEvent(QPaintEvent *event);

  private:

    QToolBar *m_toolbar;

    QWidget *m_linenumberarea;

    uint64_t m_filepos;

    Studio::Document *m_document;
};

