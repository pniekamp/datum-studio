//
// Pack Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "packplugin.h"
#include "projectapi.h"
#include "editorapi.h"
#include "contentapi.h"
#include "pack.h"
#include "dialogfactory.h"
#include "ui_properties.h"
#include <QWidgetAction>
#include <QSettings>
#include <QtPlugin>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

using namespace std;

//|---------------------- PackPlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// PackPlugin::Constructor ///////////////////////////
PackPlugin::PackPlugin()
{
}


///////////////////////// PackPlugin::Destructor ////////////////////////////
PackPlugin::~PackPlugin()
{
  shutdown();
}


///////////////////////// PackPlugin::initialise ////////////////////////////
bool PackPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  m_manager = new PackManager;

  Studio::Core::instance()->add_object(m_manager);

//  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  QIcon icon;
  icon.addFile(":/packplugin/pack.png", QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(":/packplugin/pack-selected.png", QSize(), QIcon::Normal, QIcon::On);

  m_metamode = new QAction(icon, "Pack", this);
  m_metamode->setToolTip("Switch to Pack\nctrl-2");
  m_metamode->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
  m_metamode->setEnabled(false);

  auto metamode = actionmanager->register_action("Pack.MetaMode", m_metamode);

  auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

  modemanager->add_metamode(2, metamode);

  connect(modemanager, &Studio::ModeManager::metamode_changed, this, &PackPlugin::on_metamode_changed);

  m_build = new QAction(QIcon(":/packplugin/build.png"), "Build", this);
  m_build->setToolTip("Build Pack\nctrl-b");
  m_build->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  m_build->setEnabled(false);

  auto build = actionmanager->register_action("Pack.Build", m_build);

  actionmanager->container("Studio.Meta.Box")->add_back(build);

  connect(m_build, &QAction::triggered, this, &PackPlugin::build);

  m_pack = new PackModel(this);

  m_container = new QMainWindow;

  ui.setupUi(m_container);

  ui.MenuBar->clear();

  m_container->addAction(ui.CreateFolder);
  m_container->addAction(ui.Rename);
  m_container->addAction(ui.Remove);

  actionmanager->register_action("Pack.Menu.PackProperties", ui.PackProperties);

  actionmanager->container("Studio.Menu")->add_back(actionmanager->register_container("Pack.Menu", ui.PackMenu));

  ui.Splitter->setStretchFactor(0, 1);
  ui.Splitter->setStretchFactor(1, 12);

  ui.Navigator->set_model(m_pack);

  connect(ui.CreateFolder, &QAction::triggered, this, &PackPlugin::on_CreateFolder_triggered);
  connect(ui.Rename, &QAction::triggered, this, &PackPlugin::on_Rename_triggered);
  connect(ui.Remove, &QAction::triggered, this, &PackPlugin::on_Remove_triggered);
  connect(ui.PackProperties, &QAction::triggered, this, &PackPlugin::on_PackProperties_triggered);
  connect(ui.Navigator, &TreeView::current_changed, ui.Viewport, &FileView::set_asset);
  connect(ui.Navigator, &TreeView::item_triggered, this, &PackPlugin::on_item_triggered);
  connect(ui.Navigator, &TreeView::item_renamed, this, &PackPlugin::on_item_renamed);
  connect(ui.Navigator, &TreeView::customContextMenuRequested, this, &PackPlugin::on_contextmenu_requested);

  modemanager->container()->addWidget(m_container);

  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  connect(projectmanager, &Studio::ProjectManager::project_changed, this, &PackPlugin::on_project_changed);
  connect(projectmanager, &Studio::ProjectManager::project_saving, this, &PackPlugin::on_project_saving);
  connect(projectmanager, &Studio::ProjectManager::project_closing, this, &PackPlugin::on_project_closing);
  connect(projectmanager, &Studio::ProjectManager::project_closed, this, &PackPlugin::on_project_closed);

  QSettings settings;
  ui.Splitter->restoreState(settings.value("packplugin/splitter", QByteArray()).toByteArray());

  return true;
}


///////////////////////// PackPlugin::shutdown //////////////////////////////
void PackPlugin::shutdown()
{
  QSettings settings;

  settings.setValue("packplugin/splitter", ui.Splitter->saveState());
}


///////////////////////// PackPlugin::build /////////////////////////////////
void PackPlugin::build()
{
  m_build->setEnabled(false);

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  DialogFactory<Ui::Build> dlg(mainwindow->handle());

  connect(dlg.ui.Export, &QPushButton::clicked, this, &PackPlugin::build_export);

  dlg.show();

  try
  {
    m_manager->build(m_pack, QDir(projectmanager->basepath()).filePath("Build/asset.pack"), &dlg.ui);
  }
  catch(exception &e)
  {
    qCritical() << "Build Error:" << e.what();

    dlg.ui.Message->setText(QString("Build Failed: %1").arg(e.what()));
    dlg.ui.Close->setText("Close");
  }

  dlg.exec();

  m_build->setEnabled(true);
}


///////////////////////// PackPlugin::build_export //////////////////////////
void PackPlugin::build_export()
{
  QString basepath = QSettings().value("packplugin/exportpath").toString();

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  QString exportfile = QFileDialog::getSaveFileName(mainwindow->handle(), "Export Build", basepath, "Asset Pack (*.pack)");

  if (exportfile != "")
  {
    try
    {
      QProgressDialog progress("Export Build", "Abort", 0, 100, mainwindow->handle());

      QFile::remove(exportfile);

      for(int k = 0; k < 2; ++k)
      {
        if (QFile::copy(QDir(projectmanager->basepath()).filePath("Build/asset.pack"), exportfile))
          break;
      }
    }
    catch(exception &e)
    {
      QMessageBox::information(mainwindow->handle(), "Export Error", e.what());
    }

    QSettings().setValue("packplugin/exportpath", QFileInfo(exportfile).path());
  }
}

///////////////////////// PackPlugin::project_changed ///////////////////////
void PackPlugin::on_project_changed(QString const &projectfile)
{
  m_pack->load(projectfile.toStdString());

  m_metamode->setEnabled(true);

  m_build->setEnabled(true);

  ui.PackProperties->setEnabled(true);
}


///////////////////////// PackPlugin::project_saving ////////////////////////
void PackPlugin::on_project_saving(QString const &projectfile)
{
  m_pack->save(projectfile.toStdString());
}


///////////////////////// PackPlugin::project_closing ///////////////////////
void PackPlugin::on_project_closing(bool *cancel)
{
  if (*cancel)
    return;

  if (m_pack->modified())
  {
    auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

    QString msg = QString("Project has been modified...\n\nSave Changes ?\n");

    int reply = QMessageBox::question(mainwindow->handle(), "Close", msg, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Save)
    {
      auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

      try
      {
        QProgressDialog progress("Save Project", "Abort", 0, 100, mainwindow->handle());

        projectmanager->save_project(&progress);
      }
      catch(exception &e)
      {
        QMessageBox::information(mainwindow->handle(), "Project Error", e.what());
      }
    }

    if (reply == QMessageBox::Cancel)
    {
      *cancel = true;
    }
  }
}


///////////////////////// PackPlugin::on_project_closed /////////////////////
void PackPlugin::on_project_closed()
{
  ui.Viewport->set_asset(nullptr);

  m_pack->clear();  
}


///////////////////////// PackPlugin::metamode_changed //////////////////////
void PackPlugin::on_metamode_changed(QString const &mode)
{
  if (mode == "Pack")
  {
    auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

    modemanager->container()->setCurrentWidget(m_container);
  }
}


///////////////////////// PackPlugin::item_triggered ////////////////////////
void PackPlugin::on_item_triggered(PackModel::Node *node)
{
  if (auto asset = node_cast<PackModel::Asset>(node))
  {
    if (asset->document())
    {
      auto editormanager = Studio::Core::instance()->find_object<Studio::EditorManager>();

      editormanager->open_editor(asset->document()->metadata("type", QString("Binary")), asset->path());
    }
  }
}


///////////////////////// PackPlugin::item_renamed ////////////////////////
void PackPlugin::on_item_renamed(PackModel::Node *node, QString const &str)
{
  if (auto group = node_cast<PackModel::Group>(node))
  {
    m_pack->set_data(group, PackModel::DataRole::Name, str);
  }

  if (auto asset = node_cast<PackModel::Asset>(node))
  {
    auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

    QString src = asset->path();
    QString dst = QFileInfo(src).dir().absolutePath() + "/" + str + "." + QFileInfo(src).suffix();

    contentmanager->rename_content(src, dst);
  }
}


///////////////////////// PackPlugin::contextmenu_requested ////////////////
void PackPlugin::on_contextmenu_requested(QPoint pos)
{
  QMenu menu;

  if (m_container->focusWidget() == ui.Navigator)
  {
    if (ui.Navigator->indexAt(pos) != ui.Navigator->currentIndex())
      ui.Navigator->setCurrentIndex(QModelIndex());

    if (!ui.Navigator->current_node() || node_cast<PackModel::Group>(ui.Navigator->current_node()))
    {
      menu.addAction(ui.CreateFolder);
      menu.addSeparator();
    }

    if (ui.Navigator->current_node())
    {
      menu.addAction(ui.Rename);
      menu.addAction(ui.Remove);
    }
  }

  menu.exec(QCursor::pos());
}


///////////////////////// PackPlugin::CreateFolder //////////////////////////
void PackPlugin::on_CreateFolder_triggered()
{
  auto group = ui.Navigator->current_node() ? ui.Navigator->current_node() : m_pack->root();

  m_pack->add_group(group, group->children(), "New Folder");
}


///////////////////////// PackPlugin::Rename ////////////////////////////////
void PackPlugin::on_Rename_triggered()
{
  if (m_container->focusWidget() == ui.Navigator)
  {
    if (ui.Navigator->currentIndex().isValid())
    {
      ui.Navigator->trigger_rename(ui.Navigator->currentIndex());
    }
  }
}


///////////////////////// PackPlugin::Remove ////////////////////////////////
void PackPlugin::on_Remove_triggered()
{
  if (QMessageBox::question(m_container, "Remove Asset", "Remove Selected Asset\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
  {
    if (m_container->focusWidget() == ui.Navigator)
    {
      while (ui.Navigator->selectionModel()->hasSelection())
      {
        m_pack->erase(ui.Navigator->selected_nodes().front());
      }
    }
  }
}


///////////////////////// PackPlugin::PackProperties ////////////////////////
void PackPlugin::on_PackProperties_triggered()
{
  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  DialogFactory<Ui::Properties> dlg(mainwindow->handle());

  dlg.ui.Signature->setText(m_pack->signature());
  dlg.ui.Version->setText(m_pack->version());

  if (dlg.exec() == QDialog::Accepted)
  {
    m_pack->set_parameter("signature", dlg.ui.Signature->text());
    m_pack->set_parameter("version", dlg.ui.Version->text());
  }
}
