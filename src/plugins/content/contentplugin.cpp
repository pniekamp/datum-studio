//
// Content Browser Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "contentplugin.h"
#include "contentmanager.h"
#include "projectapi.h"
#include "editorapi.h"
#include <QWidgetAction>
#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- ContentPlugin -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ContentPlugin::Constructor ////////////////////////
ContentPlugin::ContentPlugin()
{
}


///////////////////////// ContentPlugin::Destructor /////////////////////////
ContentPlugin::~ContentPlugin()
{
  shutdown();
}


///////////////////////// ContentPlugin::initialise /////////////////////////
bool ContentPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  Studio::Core::instance()->add_object(new ContentManager);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  m_statusview = new QAction("Content Browser", this);
  m_statusview->setToolTip("Content Browser\nalt-1");
  m_statusview->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
  m_statusview->setEnabled(false);

  auto statusview = actionmanager->register_action("Content.Browser", m_statusview);

  auto statusmanager = Studio::Core::instance()->find_object<Studio::StatusManager>();

  statusmanager->add_statusview(1, statusview);

  connect(statusmanager, &Studio::StatusManager::statusview_changed, this, &ContentPlugin::on_statusview_changed);

  m_container = new QWidget;

  ui.setupUi(m_container);

  m_container->addAction(ui.Import);
  m_container->addAction(ui.Reimport);
  m_container->addAction(ui.CreateFolder);
  m_container->addAction(ui.Rename);
  m_container->addAction(ui.Delete);

  ui.Header->addWidget(new QLabel("Content Browser"));
  ui.Header->addSeparator();

  m_createmenu = new QMenu("Create");
  m_createmenu->addAction(ui.CreateFolder);
  m_createmenu->addSeparator();

  actionmanager->register_container("Content.Menu.Create", m_createmenu);

  auto create = new QWidgetAction(this);
  auto createbutton = new QPushButton("Create");
  createbutton->setMenu(m_createmenu);
  create->setDefaultWidget(createbutton);
  ui.Header->addAction(create);

  auto import = new QWidgetAction(this);
  auto importbutton = new QPushButton("Import");
  import->setDefaultWidget(importbutton);
  ui.Header->addAction(import);

  actionmanager->register_action("Content.Import", import);

  connect(importbutton, &QPushButton::clicked, ui.Import, &QAction::trigger);
  connect(ui.Import, &QAction::triggered, this, &ContentPlugin::on_Import_triggered);
  connect(ui.Reimport, &QAction::triggered, this, &ContentPlugin::on_Reimport_triggered);
  connect(ui.Rename, &QAction::triggered, this, &ContentPlugin::on_Rename_triggered);
  connect(ui.Delete, &QAction::triggered, this, &ContentPlugin::on_Delete_triggered);
  connect(m_createmenu, &QMenu::triggered, this, &ContentPlugin::on_Create_triggered);
  connect(ui.Folders, &FolderView::selection_changed, ui.Content, &ContentView::set_path);
  connect(ui.Folders, &FolderView::item_renamed, this, &ContentPlugin::on_item_renamed);
  connect(ui.Folders, &FolderView::customContextMenuRequested, this, &ContentPlugin::on_contextmenu_requested);
  connect(ui.Content, &ContentView::item_triggered, this, &ContentPlugin::on_item_triggered);
  connect(ui.Content, &ContentView::item_renamed, this, &ContentPlugin::on_item_renamed);
  connect(ui.Content, &ContentView::customContextMenuRequested, this, &ContentPlugin::on_contextmenu_requested);

  statusmanager->container()->addWidget(m_container);

  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  connect(projectmanager, &Studio::ProjectManager::project_changed, this, &ContentPlugin::on_project_changed);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  connect(contentmanager, &Studio::ContentManager::content_changed, ui.Folders, &FolderView::update);
  connect(contentmanager, &Studio::ContentManager::content_changed, ui.Content, &ContentView::update);

  auto mainwindow =  Studio::Core::instance()->find_object<Studio::MainWindow>();

  connect(mainwindow, &Studio::MainWindow::activated, ui.Folders, &FolderView::refresh);
  connect(mainwindow, &Studio::MainWindow::activated, ui.Content, &ContentView::refresh);

  QSettings settings;
  ui.Splitter->restoreState(settings.value("contentplugin/splitter", QByteArray()).toByteArray());

  return true;
}


///////////////////////// ContentPlugin::shutdown ///////////////////////////
void ContentPlugin::shutdown()
{
  QSettings settings;

  settings.setValue("contentplugin/splitter", ui.Splitter->saveState());
}


///////////////////////// ContentPlugin::project_changed ////////////////////
void ContentPlugin::on_project_changed(QString const &projectfile)
{
  m_statusview->setEnabled(true);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  ui.Folders->set_base(contentmanager->basepath());
}


///////////////////////// ContentPlugin::statusview_changed ///////////////////
void ContentPlugin::on_statusview_changed(QString const &view)
{
  if (view == "Content Browser")
  {
    auto statusmanager = Studio::Core::instance()->find_object<Studio::StatusManager>();

    statusmanager->container()->setCurrentWidget(m_container);
  }
}


///////////////////////// ContentPlugin::item_triggered /////////////////////
void ContentPlugin::on_item_triggered(QString const &path)
{
  if (QFileInfo(path).isDir())
  {
    ui.Folders->select_path(path);

    return;
  }

  QString editor = "Binary";

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  if (auto document = documentmanager->open(path))
  {
    editor = document->metadata("type", editor);

    documentmanager->close(document);
  }

  auto editormanager = Studio::Core::instance()->find_object<Studio::EditorManager>();

  editormanager->open_editor(editor, path);
}


///////////////////////// ContentPlugin::item_renamed ///////////////////////
void ContentPlugin::on_item_renamed(QString const &src, QString const &dst)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->rename_content(src, dst);
}


///////////////////////// ContentPlugin::contextmenu_requested //////////////
void ContentPlugin::on_contextmenu_requested(QPoint pos)
{
  QMenu menu;

  if (m_container->focusWidget() == ui.Folders)
  {
    menu.addMenu(m_createmenu);
    menu.addSeparator();

    if (ui.Folders->indexAt(pos) != ui.Folders->currentIndex())
      ui.Folders->setCurrentItem(ui.Folders->topLevelItem(0));

    if (ui.Folders->currentItem() != ui.Folders->topLevelItem(0))
    {
      menu.addAction(ui.Rename);
      menu.addAction(ui.Delete);
      menu.addSeparator();
    }

    menu.addAction(ui.Reimport);
  }

  if (m_container->focusWidget() == ui.Content)
  {
    if (ui.Content->indexAt(pos) != ui.Content->currentIndex())
      ui.Content->setCurrentIndex(QModelIndex());

    if (!ui.Content->currentIndex().isValid())
    {
      menu.addMenu(m_createmenu);
      menu.addSeparator();
    }

    if (ui.Content->currentIndex().isValid())
    {
      menu.addAction(ui.Rename);
      menu.addAction(ui.Delete);
      menu.addSeparator();
    }

    menu.addAction(ui.Reimport);
  }

  menu.exec(QCursor::pos());
}


///////////////////////// ContentPlugin::Create /////////////////////////////
void ContentPlugin::on_Create_triggered(QAction *action)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  QString type = action->text();

  QString suffix = (type == "Folder") ? "" : ".asset";

  QString path = ui.Folders->selected_path() + "/" + QString(type).remove("\\") + suffix;

  for(int k = 1; k < 256 && QFileInfo(path).exists(); ++k)
  {
    path = ui.Folders->selected_path() + "/" + QString(type).remove("\\") + "_" + QString::number(k) + suffix;
  }

  if (contentmanager->create(type, path))
  {
    ui.Content->select_path(path);

    ui.Content->trigger_rename(ui.Content->currentItem());
  }
}


///////////////////////// ContentPlugin::Import /////////////////////////////
void ContentPlugin::on_Import_triggered()
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  QString src = QFileDialog::getOpenFileName(m_container, "Import");

  if (src != "")
  {
    contentmanager->import(src, ui.Folders->selected_path());
  }
}


///////////////////////// ContentPlugin::Reimport ///////////////////////////
void ContentPlugin::on_Reimport_triggered()
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  if (m_container->focusWidget() == ui.Folders)
  {
    contentmanager->reimport(ui.Folders->selected_path());
  }

  if (m_container->focusWidget() == ui.Content)
  {
    for(auto &path : ui.Content->selected_paths())
    {
      contentmanager->reimport(path);
    }
  }
}


///////////////////////// ContentPlugin::Rename /////////////////////////////
void ContentPlugin::on_Rename_triggered()
{
  if (m_container->focusWidget() == ui.Folders && ui.Folders->currentIndex().parent().isValid())
  {
    ui.Folders->trigger_rename(ui.Folders->currentItem());
  }

  if (m_container->focusWidget() == ui.Content && ui.Content->currentIndex().isValid())
  {
    ui.Content->trigger_rename(ui.Content->currentItem());
  }
}


///////////////////////// ContentPlugin::Delete /////////////////////////////
void ContentPlugin::on_Delete_triggered()
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  if (QMessageBox::question(m_container, "Delete Content", "Delete Selected Content\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
  {
    if (m_container->focusWidget() == ui.Folders && ui.Folders->currentIndex().parent().isValid())
    {
      contentmanager->delete_content(ui.Folders->selected_path());
    }

    if (m_container->focusWidget() == ui.Content)
    {
      for(auto &path : ui.Content->selected_paths())
      {
        contentmanager->delete_content(path);
      }
    }
  }
}
