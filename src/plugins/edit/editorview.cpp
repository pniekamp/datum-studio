//
// Editor View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "editorview.h"
#include "commandbar.h"
#include <QApplication>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QListView>
#include <QFileInfo>
#include <QtDebug>

using namespace std;

//|---------------------- EditorView ----------------------------------------
//|--------------------------------------------------------------------------
//| Editor View
//|

///////////////////////// EditorView::Constructor ///////////////////////////
EditorView::EditorView(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);

  m_doclist = new QComboBox(this);
  m_doclist->setView(new QListView);
  m_doclist->setMinimumWidth(160);

  connect(m_doclist, &QComboBox::currentTextChanged, this, &EditorView::on_doclist_changed);

  ui.File->setMenu(new QMenu(this));

  ui.File->menu()->addAction(ui.Save);
  ui.File->menu()->addAction(ui.SaveAll);
  ui.File->menu()->addAction(ui.Revert);
  ui.File->menu()->addSeparator();
  ui.File->menu()->addAction(ui.Close);
  ui.File->menu()->addAction(ui.CloseAll);
  ui.File->menu()->addSeparator();
  ui.File->menu()->addAction(ui.OpenAsBinary);

  ui.NavBar->addWidget(m_doclist);
  ui.NavBar->addAction(ui.File);
  ui.NavBar->addAction(ui.Close);
  ui.NavBar->addSeparator();

  addAction(ui.Save);
  addAction(ui.SaveAll);
  addAction(ui.Revert);
  addAction(ui.Close);
  addAction(ui.CloseAll);

  connect(qApp, &QApplication::focusChanged, this, &EditorView::on_focus_changed);

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_renamed, this, &EditorView::on_document_renamed);
}


///////////////////////// EditorView::open_editor ///////////////////////////
void EditorView::open_editor(QString const &type, QString const &path)
{
  bool result = false;

  for(int i = 0; i < m_editors.size(); ++i)
  {
    if (m_editors[i].type == type && m_editors[i].path == path)
    {
      m_doclist->setCurrentIndex(i);

      result = true;
    }
  }

  if (!result)
  {
    auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    Editor editor = {};
    editor.type = type;
    editor.path = path;
    editor.document = documentmanager->open(path);
    editor.widget = viewfactory->create_view(type);

    if (editor.document && editor.widget)
    {
      ui.Container->addWidget(editor.widget);

      QMetaObject::invokeMethod(editor.widget, "edit", Q_ARG(Studio::Document*, editor.document));
    }

    m_editors.push_back(editor);

    m_doclist->addItem(QFileInfo(path).completeBaseName());

    m_doclist->setCurrentIndex(m_doclist->count()-1);
  }

  auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

  if (modemanager->metamode() != "Edit")
    modemanager->set_metamode("Edit");
}


///////////////////////// EditorView::add_win_action ////////////////////////
void EditorView::add_win_action(QAction *action)
{
  ui.WinBar->addAction(action);
}


///////////////////////// EditorView::save_all //////////////////////////////
int EditorView::save_all()
{
  for(int i = m_editors.size() - 1; i >= 0; --i)
  {
    if (m_editors[i].document->modified())
    {
      save_editor(i);
    }
  }

  return QMessageBox::Ok;
}


///////////////////////// EditorView::close_all /////////////////////////////
int EditorView::close_all()
{
  int reply = QMessageBox::Ok;

  for(int i = m_editors.size() - 1; i >= 0; --i)
  {
    if (close_editor(i) == QMessageBox::Cancel)
    {
      reply = QMessageBox::Cancel;
      break;
    }
  }

  return reply;
}


///////////////////////// EditorView::focus_changed /////////////////////////
void EditorView::on_focus_changed(QWidget *old, QWidget *now)
{
  if (focusWidget() && focusWidget() == now)
  {
    emit focus_event();
  }
}


///////////////////////// EditorView::doclist_changed ///////////////////////
void EditorView::on_doclist_changed()
{
  int index = m_doclist->currentIndex();

  if (index != -1)
  {
    auto &editor = m_editors[index];

    if (editor.widget)
    {
      ui.ToolBar->clear();

      QToolBar *toolbar = nullptr;

      QMetaObject::invokeMethod(editor.widget, "toolbar", Q_RETURN_ARG(QToolBar*, toolbar));

      if (toolbar)
      {
        ui.ToolBar->addWidget(toolbar)->setVisible(true);
      }

      ui.Container->setCurrentWidget(editor.widget);

      editor.widget->setFocus();
    }
  }
}


///////////////////////// EditorView::document_renamed //////////////////////
void EditorView::on_document_renamed(Studio::Document *document, QString src, QString dst)
{
  for(int i = 0; i < m_editors.size(); ++i)
  {
    if (m_editors[i].document == document)
    {
      m_editors[i].path = dst;
      m_doclist->setItemText(i, QFileInfo(dst).completeBaseName());
    }
  }
}


///////////////////////// EditorView::save_editor ///////////////////////////
int EditorView::save_editor(int index)
{
  auto &editor = m_editors[index];

  if (editor.document)
  {
    editor.document->lock_exclusive();

    editor.document->save();

    editor.document->unlock_exclusive();
  }

  return QMessageBox::Ok;
}


///////////////////////// EditorView::close_editor //////////////////////////
int EditorView::close_editor(int index)
{
  int reply = QMessageBox::Ok;

  bool close = true;

  auto &editor = m_editors[index];

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  if (close && editor.document && editor.document->modified())
  {
    QString msg = QString("The content of \"%1\" has been modified...\n\nSave Changes ?\n").arg(QFileInfo(editor.path).completeBaseName());

    reply = QMessageBox::question(this, "Close", msg, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    editor.document->lock_exclusive();

    if (reply == QMessageBox::Save)
    {
      editor.document->save();
    }

    if (reply == QMessageBox::Discard)
    {
      editor.document->discard();
    }

    editor.document->unlock_exclusive();

    if (reply == QMessageBox::Cancel)
    {
      close = false;
    }
  }

  if (close && editor.widget && !editor.widget->close())
  {
    reply = QMessageBox::Cancel;
    close = false;
  }

  if (close)
  {
    if (editor.widget)
    {
      delete editor.widget;
    }

    if (editor.document)
    {
      documentmanager->close(editor.document);
    }

    m_editors.remove(index);

    m_doclist->removeItem(index);
  }

  return reply;
}


///////////////////////// EditorView::Save //////////////////////////////////
void EditorView::on_Save_triggered()
{
  int index = m_doclist->currentIndex();

  if (index != -1)
  {
    save_editor(index);
  }
}


///////////////////////// EditorView::SaveAll ///////////////////////////////
void EditorView::on_SaveAll_triggered()
{
  save_all();
}


///////////////////////// EditorView::Revert ////////////////////////////////
void EditorView::on_Revert_triggered()
{
  int index = m_doclist->currentIndex();

  if (index != -1)
  {
    auto &editor = m_editors[index];

    if (editor.document)
    {
      QString msg = QString("Revert \"%1\" ?\n").arg(QFileInfo(editor.path).completeBaseName());

      int reply = QMessageBox::question(this, "Revert", msg, QMessageBox::Ok | QMessageBox::Cancel);

      if (reply == QMessageBox::Ok)
      {
        editor.document->lock_exclusive();

        editor.document->discard();

        editor.document->unlock_exclusive();
      }
    }
  }
}


///////////////////////// EditorView::Close /////////////////////////////////
void EditorView::on_Close_triggered()
{
  int index = m_doclist->currentIndex();

  if (index != -1)
  {
    close_editor(index);
  }
}


///////////////////////// EditorView::CloseAll //////////////////////////////
void EditorView::on_CloseAll_triggered()
{
  close_all();
}


///////////////////////// EditorView::OpenAsBinary //////////////////////////
void EditorView::on_OpenAsBinary_triggered()
{
  int index = m_doclist->currentIndex();

  if (index != -1)
  {
    auto &editor = m_editors[index];

    auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

    delete editor.widget;

    editor.widget = viewfactory->create_view("Binary");

    if (editor.document && editor.widget)
    {
      ui.Container->addWidget(editor.widget);

      QMetaObject::invokeMethod(editor.widget, "edit", Q_ARG(Studio::Document*, editor.document));
    }

    on_doclist_changed();
  }
}
