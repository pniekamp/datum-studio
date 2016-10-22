//
// Font Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "fonteditor.h"
#include "commandbar.h"
#include "assetpacker.h"
#include <QSettings>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- FontEditor ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// FontEditor::Constructor ///////////////////////////
FontEditor::FontEditor(QWidget *parent)
  : QMainWindow(parent)
{
  m_scaleslider = new QcDoubleSlider(Qt::Horizontal, this);
  m_scaleslider->setRange(0.1, 6.4);
  m_scaleslider->setValue(2.15);
  m_scaleslider->setMaximumWidth(100);

  m_toolbar = new CommandBar(this);
  m_toolbar->addWidget(new QLabel("Scale:", this));
  m_toolbar->addWidget(m_scaleslider);

  m_view = new FontView(this);

  setCentralWidget(m_view);

  m_properties = new FontProperties(this);

  addDockWidget(Qt::RightDockWidgetArea, m_properties);

  connect(m_scaleslider, &QcDoubleSlider::valueChanged, m_view, &FontView::set_zoom);
  connect(m_view, &FontView::zoom_changed, m_scaleslider, &QcDoubleSlider::updateValue);

  QSettings settings;
  restoreState(settings.value("fonteditor/state", QByteArray()).toByteArray());
}


///////////////////////// FontEditor::Destructor ////////////////////////////
FontEditor::~FontEditor()
{
  QSettings settings;

  settings.setValue("fonteditor/state", saveState());

  delete m_toolbar;
}


///////////////////////// FontEditor::toolbar ///////////////////////////////
QToolBar *FontEditor::toolbar() const
{
  return m_toolbar;
}


///////////////////////// FontEditor::view //////////////////////////////////
void FontEditor::view(Studio::Document *document)
{
  m_view->view(document);

  m_properties->hide();
}


///////////////////////// FontEditor::edit //////////////////////////////////
void FontEditor::edit(Studio::Document *document)
{
  m_view->view(document);

  m_properties->edit(document);

  m_properties->show();
}
