//
// Sprite Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "spriteeditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SpriteEditor --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SpriteEditor::Constructor /////////////////////////
SpriteEditor::SpriteEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_scaleslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_scaleslider->setRange(0.1, 6.4);
  m_scaleslider->setValue(2.15);
  m_scaleslider->setMaximumWidth(100);

  m_layerslider = new QcSlider(Qt::Horizontal, this);
  m_layerslider->setRange(0, 8);
  m_layerslider->setValue(0);
  m_layerslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Scale:", this));
  m_toolbar->addWidget(m_scaleslider);
  m_toolbar->addWidget(new QLabel("Layer:", this));
  m_toolbar->addWidget(m_layerslider);

  m_view = new SpriteView(this);

  setCentralWidget(m_view);

  m_properties = new SpriteProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_scaleslider, &QcDoubleSlider::valueChanged, m_view, &SpriteView::set_zoom);
  connect(m_view, &SpriteView::zoom_changed, m_scaleslider, &QcDoubleSlider::updateValue);

  connect(m_layerslider, &QcSlider::valueChanged, m_view, &SpriteView::set_layer);
  connect(m_view, &SpriteView::layer_changed, m_layerslider, &QcSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("spriteeditor/state", QByteArray()).toByteArray());
}


///////////////////////// SpriteEditor::Destructor //////////////////////////
SpriteEditor::~SpriteEditor()
{
  QSettings settings;

  settings.setValue("spriteeditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// SpriteEditor::toolbar /////////////////////////////
QToolBar *SpriteEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// SpriteEditor::view ////////////////////////////////
void SpriteEditor::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// SpriteEditor::edit ////////////////////////////////
void SpriteEditor::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
