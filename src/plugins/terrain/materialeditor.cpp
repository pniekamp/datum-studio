//
// Material Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialeditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MaterialEditor ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialEditor::Constructor ///////////////////////
MaterialEditor::MaterialEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_exposureslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_exposureslider->setRange(0.01, 10.0);
  m_exposureslider->setValue(1.0);
  m_exposureslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Exposure:", this));
  m_toolbar->addWidget(m_exposureslider);

  m_view = new MaterialView(this);

  setCentralWidget(m_view);

  m_properties = new MaterialProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_exposureslider, &QcDoubleSlider::valueChanged, m_view, &MaterialView::set_exposure);
  connect(m_view, &MaterialView::exposure_changed, m_exposureslider, &QcDoubleSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("terrainmaterialeditor/state", QByteArray()).toByteArray());
}


///////////////////////// MaterialEditor::Destructor ////////////////////////
MaterialEditor::~MaterialEditor()
{
  QSettings settings;

  settings.setValue("terrainmaterialeditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// MaterialEditor::toolbar ///////////////////////////
QToolBar *MaterialEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// MaterialEditor::view //////////////////////////////
void MaterialEditor::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// MaterialEditor::edit //////////////////////////////
void MaterialEditor::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
