//
// Animation Viewer
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "animationviewer.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- AnimationViewer -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AnimationViewer::Constructor //////////////////////
AnimationViewer::AnimationViewer(QWidget *parent)
  : QMainWindow(parent)
{
  m_toolbar = new CommandBar(this);

  m_view = new AnimationView(this);

  setCentralWidget(m_view);

  m_properties = new AnimationProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  QSettings settings;
  restoreState(settings.value("animationviewer/state", QByteArray()).toByteArray());
}


///////////////////////// AnimationViewer::Destructor ///////////////////////
AnimationViewer::~AnimationViewer()
{
  QSettings settings;

  settings.setValue("animationviewer/state", saveState());

  delete m_toolbar;
}


///////////////////////// AnimationViewer::toolbar //////////////////////////
QToolBar *AnimationViewer::toolbar() const
{
  return m_toolbar;
}


///////////////////////// AnimationViewer::view /////////////////////////////
void AnimationViewer::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// AnimationViewer::edit /////////////////////////////
void AnimationViewer::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
