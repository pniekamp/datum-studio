//
// Edit Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "editplugin.h"
#include "projectapi.h"
#include <QMessageBox>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- EditPlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// EditPlugin::Constructor ///////////////////////////
EditPlugin::EditPlugin()
{
}


///////////////////////// EditPlugin::Destructor ////////////////////////////
EditPlugin::~EditPlugin()
{
  shutdown();
}


///////////////////////// EditPlugin::initialise ////////////////////////////
bool EditPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  m_manager = new EditorManager;

  Studio::Core::instance()->add_object(m_manager);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  QIcon icon;
  icon.addFile(":/editplugin/edit.png", QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(":/editplugin/edit-selected.png", QSize(), QIcon::Normal, QIcon::On);

  m_metamode = new QAction(icon, "Edit", this);
  m_metamode->setToolTip("Switch to Edit\nctrl-3");
  m_metamode->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
  m_metamode->setEnabled(false);

  auto metamode = actionmanager->register_action("Edit.MetaMode", m_metamode);

  auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

  modemanager->add_metamode(3, metamode);

  connect(modemanager, &Studio::ModeManager::metamode_changed, this, &EditPlugin::on_metamode_changed);

  m_container = new QWidget;

  ui.setupUi(m_container);

  ui.Editor1->add_win_action(ui.Expand);
  ui.Editor2->add_win_action(ui.Collapse);

  ui.Editor2->hide();

  m_manager->register_editor(ui.Editor1);
  m_manager->register_editor(ui.Editor2);
  m_manager->set_current_editor(ui.Editor1);

  connect(ui.Expand, &QAction::triggered, this, &EditPlugin::on_expand_editor);
  connect(ui.Collapse, &QAction::triggered, this, &EditPlugin::on_collapse_editor);
  connect(ui.Editor1, &EditorView::focus_event, this, [&]() { m_manager->set_current_editor(ui.Editor1); });
  connect(ui.Editor2, &EditorView::focus_event, this, [&]() { m_manager->set_current_editor(ui.Editor2); });

  modemanager->container()->addWidget(m_container);

  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  connect(projectmanager, &Studio::ProjectManager::project_changed, this, &EditPlugin::on_project_changed);
  connect(projectmanager, &Studio::ProjectManager::project_saving, this, &EditPlugin::on_project_saving);
  connect(projectmanager, &Studio::ProjectManager::project_closing, this, &EditPlugin::on_project_closing);

  return true;
}


///////////////////////// EditPlugin::shutdown //////////////////////////////
void EditPlugin::shutdown()
{
}


///////////////////////// EditPlugin::project_changed ///////////////////////
void EditPlugin::on_project_changed(QString const &projectfile)
{
  m_metamode->setEnabled(true);
}


///////////////////////// EditPlugin::project_saving ////////////////////////
void EditPlugin::on_project_saving(QString const &projectfile)
{
  m_manager->save_all();
}


///////////////////////// EditPlugin::project_closing ///////////////////////
void EditPlugin::on_project_closing(bool *cancel)
{
  if (*cancel)
    return;

  if (m_manager->close_all() == QMessageBox::Cancel)
  {
    *cancel = true;
  }
}


///////////////////////// EditPlugin::metamode_changed //////////////////////
void EditPlugin::on_metamode_changed(QString const &mode)
{
  if (mode == "Edit")
  {
    auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

    modemanager->container()->setCurrentWidget(m_container);
  }
}


///////////////////////// EditPlugin::expand_editor /////////////////////////
void EditPlugin::on_expand_editor()
{
  ui.Splitter->setSizes({ 50, 50 });

  ui.Editor2->show();
  ui.Editor2->setFocus();
}


///////////////////////// EditPlugin::collapse_editor ///////////////////////
void EditPlugin::on_collapse_editor()
{
  if (ui.Editor2->close_all() != QMessageBox::Cancel)
  {
    ui.Editor2->hide();
    ui.Editor1->setFocus();
  }
}

