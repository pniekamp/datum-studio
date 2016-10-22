//
// Skybox Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "skyboxeditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SkyboxEditor --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SkyboxEditor::Constructor /////////////////////////
SkyboxEditor::SkyboxEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_layerslider = new QcSlider(Qt::Horizontal, this);
  m_layerslider->setRange(0, 8);
  m_layerslider->setValue(0);
  m_layerslider->setMaximumWidth(100);

  m_exposureslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_exposureslider->setRange(0.01, 10.0);
  m_exposureslider->setValue(1.0);
  m_exposureslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Layer:", this));
  m_toolbar->addWidget(m_layerslider);
  m_toolbar->addWidget(new QLabel("Exposure:", this));
  m_toolbar->addWidget(m_exposureslider);

  m_view = new SkyboxView(this);

  setCentralWidget(m_view);

  m_properties = new SkyboxProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_layerslider, &QcSlider::valueChanged, m_view, &SkyboxView::set_layer);
  connect(m_view, &SkyboxView::layer_changed, m_layerslider, &QcSlider::updateValue);

  connect(m_exposureslider, &QcDoubleSlider::valueChanged, m_view, &SkyboxView::set_exposure);
  connect(m_view, &SkyboxView::exposure_changed, m_exposureslider, &QcDoubleSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("skyboxeditor/state", QByteArray()).toByteArray());
}


///////////////////////// SkyboxEditor::Destructor //////////////////////////
SkyboxEditor::~SkyboxEditor()
{
  QSettings settings;

  settings.setValue("skyboxeditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// SkyboxEditor::toolbar /////////////////////////////
QToolBar *SkyboxEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// SkyboxEditor::view ////////////////////////////////
void SkyboxEditor::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// SkyboxEditor::edit ////////////////////////////////
void SkyboxEditor::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
