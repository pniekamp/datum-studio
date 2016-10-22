//
// Image Viewer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "imgviewer.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ImageViewer ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ImageViewer::Constructor //////////////////////////
ImageViewer::ImageViewer(QWidget *parent)
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

  m_exposureslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_exposureslider->setRange(0.01, 10.0);
  m_exposureslider->setValue(1.0);
  m_exposureslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Scale:", this));
  m_toolbar->addWidget(m_scaleslider);
  m_toolbar->addWidget(new QLabel("Layer:", this));
  m_toolbar->addWidget(m_layerslider);
  m_toolbar->addWidget(new QLabel("Exposure:", this));
  m_toolbar->addWidget(m_exposureslider);

  m_view = new ImageView(this);

  setCentralWidget(m_view);

  m_properties = new ImageProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_scaleslider, &QcDoubleSlider::valueChanged, m_view, &ImageView::set_zoom);
  connect(m_view, &ImageView::zoom_changed, m_scaleslider, &QcDoubleSlider::updateValue);

  connect(m_layerslider, &QcSlider::valueChanged, m_view, &ImageView::set_layer);
  connect(m_view, &ImageView::layer_changed, m_layerslider, &QcSlider::updateValue);

  connect(m_exposureslider, &QcDoubleSlider::valueChanged, m_view, &ImageView::set_exposure);
  connect(m_view, &ImageView::exposure_changed, m_exposureslider, &QcDoubleSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("imgviewer/state", QByteArray()).toByteArray());
}


///////////////////////// ImageViewer::Destructor ///////////////////////////
ImageViewer::~ImageViewer()
{
  QSettings settings;

  settings.setValue("imgviewer/state", saveState());

  delete m_toolbar;
}


///////////////////////// ImageViewer::toolbar //////////////////////////////
QToolBar *ImageViewer::toolbar() const
{
  return m_toolbar;
}


///////////////////////// ImageViewer::view /////////////////////////////////
void ImageViewer::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// ImageViewer::edit /////////////////////////////////
void ImageViewer::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
