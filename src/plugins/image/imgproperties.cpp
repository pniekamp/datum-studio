//
// Image Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "imgproperties.h"
#include "contentapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ImageProperties -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ImageProperties::Constructor //////////////////////
ImageProperties::ImageProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);

  ui.ImportSrc->set_browsetype(QcFileLineEdit::BrowseType::OpenFile);

  ui.ImportWidth->setValidator(new QIntValidator(0, 9999, this));
  ui.ImportHeight->setValidator(new QIntValidator(0, 9999, this));

  connect(ui.ImportWidthReset, &QToolButton::clicked, this, [=]() { ui.ImportWidth->setText(""); });
  connect(ui.ImportHeightReset, &QToolButton::clicked, this, [=]() { ui.ImportHeight->setText(""); });
}


///////////////////////// ImageProperties::edit /////////////////////////////
void ImageProperties::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ImageDocument::document_changed, this, &ImageProperties::refresh);

  refresh();
}


///////////////////////// ImageProperties::refresh //////////////////////////
void ImageProperties::refresh()
{
  m_document->lock();

  PackImageHeader imag;

  if (read_asset_header(m_document, 1, &imag))
  {
    ui.Width->setText(QString::number(imag.width));
    ui.Height->setText(QString::number(imag.height));
    ui.Layers->setText(QString::number(imag.layers));
  }

  ui.ImportSrc->setText(m_document->metadata("src").toString());
  ui.ImportWidth->setText(m_document->metadata("importwidth").toString());
  ui.ImportHeight->setText(m_document->metadata("importheight").toString());

  m_document->unlock();
}


///////////////////////// ImageProperties::Reimport /////////////////////////
void ImageProperties::on_Reimport_clicked()
{
  if (ui.ImportSrc->text() == "")
    return;

  m_document->lock_exclusive();

  m_document->set_metadata("src", ui.ImportSrc->text());
  m_document->set_metadata("importwidth", (ui.ImportWidth->text() != "") ? ui.ImportWidth->text().toInt() : QVariant());
  m_document->set_metadata("importheight", (ui.ImportHeight->text() != "") ? ui.ImportHeight->text().toInt() : QVariant());

  m_document->save();

  m_document->unlock_exclusive(false);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  contentmanager->reimport(documentmanager->path(m_document));
}
