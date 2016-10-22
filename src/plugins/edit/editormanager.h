//
// Editor Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "documentapi.h"
#include "editorapi.h"
#include "editorview.h"

//-------------------------- EditorManager ----------------------------------
//---------------------------------------------------------------------------

class EditorManager : public Studio::EditorManager
{
  Q_OBJECT

  public:
    EditorManager();

    void open_editor(QString const &type, QString const &path);

    int save_all();
    int close_all();

  public slots:

    void register_editor(EditorView *editor);

    void set_current_editor(EditorView *editor);

  private:

    EditorView *m_currentview;

    std::vector<EditorView*> m_views;
};
