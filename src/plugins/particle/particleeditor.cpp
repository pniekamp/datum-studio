//
// Particle Editor
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "particleeditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ParticleEditor ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ParticleEditor::Constructor ///////////////////////
ParticleEditor::ParticleEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_exposureslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_exposureslider->setRange(0.01, 10.0);
  m_exposureslider->setValue(1.0);
  m_exposureslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Exposure:", this));
  m_toolbar->addWidget(m_exposureslider);

  m_view = new ParticleView(this);

  setCentralWidget(m_view);

  m_properties = new ParticleProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_exposureslider, &QcDoubleSlider::valueChanged, m_view, &ParticleView::set_exposure);
  connect(m_view, &ParticleView::exposure_changed, m_exposureslider, &QcDoubleSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("particleeditor/state", QByteArray()).toByteArray());
}


///////////////////////// ParticleEditor::Destructor ////////////////////////
ParticleEditor::~ParticleEditor()
{
  QSettings settings;

  settings.setValue("particleeditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// ParticleEditor::toolbar ///////////////////////////
QToolBar *ParticleEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// ParticleEditor::view //////////////////////////////
void ParticleEditor::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// ParticleEditor::edit //////////////////////////////
void ParticleEditor::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
