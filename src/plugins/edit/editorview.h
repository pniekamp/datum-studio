//
// Editor View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "ui_editorview.h"
#include <QWidget>
#include <QComboBox>

//-------------------------- EditorView -------------------------------------
//---------------------------------------------------------------------------

class EditorView : public QWidget
{
  Q_OBJECT

  public:
    EditorView(QWidget *parent = 0);

    void open_editor(QString const &type, QString const &path);

    void add_win_action(QAction *action);

  signals:

    void focus_event();

  public slots:

    int save_all();
    int close_all();

  protected:

    void on_focus_changed(QWidget *old, QWidget *now);

    void on_doclist_changed();

    void on_document_renamed(Studio::Document *document, QString src, QString dst);

    int save_editor(int index);
    int close_editor(int index);

  protected slots:

    void on_Save_triggered();
    void on_SaveAll_triggered();
    void on_Revert_triggered();

    void on_Close_triggered();
    void on_CloseAll_triggered();

    void on_OpenAsBinary_triggered();

  private:

    QComboBox *m_doclist;

    struct Editor
    {
      QString type;
      QString path;
      QWidget *widget;
      Studio::Document *document;
    };

    QVector<Editor> m_editors;

  private:

    Ui::EditorView ui;
};
