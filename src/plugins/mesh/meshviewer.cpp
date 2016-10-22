//
// Mesh Viewer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshviewer.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MeshViewer ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshViewer::Constructor ///////////////////////////
MeshViewer::MeshViewer(QWidget *parent)
  : QMainWindow(parent)
{
  m_toolbar = new CommandBar(this);

  m_view = new MeshView(this);

  setCentralWidget(m_view);

  m_properties = new MeshProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  QSettings settings;
  restoreState(settings.value("meshviewer/state", QByteArray()).toByteArray());
}


///////////////////////// MeshViewer::Destructor ////////////////////////////
MeshViewer::~MeshViewer()
{
  QSettings settings;

  settings.setValue("meshviewer/state", saveState());

  delete m_toolbar;
}


///////////////////////// MeshViewer::toolbar ///////////////////////////////
QToolBar *MeshViewer::toolbar() const
{
  return m_toolbar;
}


///////////////////////// MeshViewer::view //////////////////////////////////
void MeshViewer::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// MeshViewer::edit //////////////////////////////////
void MeshViewer::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
