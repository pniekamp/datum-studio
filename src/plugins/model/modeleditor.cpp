//
// Model Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "modeleditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ModelEditor ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelEditor::Constructor //////////////////////////
ModelEditor::ModelEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_exposureslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_exposureslider->setRange(0.01, 10.0);
  m_exposureslider->setValue(1.0);
  m_exposureslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Exposure:", this));
  m_toolbar->addWidget(m_exposureslider);

  m_view = new ModelView(this);

  setCentralWidget(m_view);

  m_properties = new ModelProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_exposureslider, &QcDoubleSlider::valueChanged, m_view, &ModelView::set_exposure);
  connect(m_view, &ModelView::exposure_changed, m_exposureslider, &QcDoubleSlider::updateValue);

  connect(m_properties, &ModelProperties::selection_changed, m_view, &ModelView::set_selection);
  connect(m_view, &ModelView::selection_changed, m_properties, &ModelProperties::set_selection);

  QSettings settings;
  restoreState(settings.value("modeleditor/state", QByteArray()).toByteArray());
}


///////////////////////// ModelEditor::Destructor ///////////////////////////
ModelEditor::~ModelEditor()
{
  QSettings settings;

  settings.setValue("modeleditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// ModelEditor::toolbar //////////////////////////////
QToolBar *ModelEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// ModelEditor::view /////////////////////////////////
void ModelEditor::view(Studio::Document *document)
{
  m_document = document;

  m_view->view(&m_document);

  m_properties->hide();
}


///////////////////////// ModelEditor::edit /////////////////////////////////
void ModelEditor::edit(Studio::Document *document)
{
  m_document = document;

  m_view->edit(&m_document);

  m_properties->edit(&m_document);

  m_properties->show();
}
