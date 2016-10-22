//
// Editor Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "editormanager.h"
#include <QMessageBox>

#include <QtDebug>

using namespace std;

//|---------------------- EditorManager -------------------------------------
//|--------------------------------------------------------------------------
//| Editor Manager
//|

///////////////////////// EditorManager::Constructor ////////////////////////
EditorManager::EditorManager()
{
  m_currentview = nullptr;
}


///////////////////////// EditorManager::open_editor ////////////////////////
void EditorManager::open_editor(QString const &type, QString const &path)
{
  m_currentview->open_editor(type, path);
}


///////////////////////// EditorManager::register_editor ////////////////////
void EditorManager::register_editor(EditorView *editor)
{
  m_views.push_back(editor);
}


///////////////////////// EditorManager::set_current_editor /////////////////
void EditorManager::set_current_editor(EditorView *editor)
{
  m_currentview = editor;
}


///////////////////////// EditorManager::save_all ///////////////////////////
int EditorManager::save_all()
{
  for(auto &view : m_views)
  {
    view->save_all();
  }

  return QMessageBox::Ok;
}


///////////////////////// EditorManager::close_all //////////////////////////
int EditorManager::close_all()
{
  int reply = QMessageBox::Ok;

  for(auto &view : m_views)
  {
    if (view->close_all() == QMessageBox::Cancel)
    {
      reply = QMessageBox::Cancel;
      break;
    }
  }

  return reply;
}
