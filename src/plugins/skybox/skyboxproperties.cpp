//
// SkyboxProperties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "skyboxproperties.h"
#include <leap.h>

#include <QDebug>

using namespace std;
using leap::extentof;

//|---------------------- SkyboxProperties ----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SkyboxProperties::Constructor /////////////////////
SkyboxProperties::SkyboxProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this); 

  ui.Width->setValidator(new QIntValidator(0, 9999, this));
  ui.Height->setValidator(new QIntValidator(0, 9999, this));

  ui.EnvMap->set_droptype("Image");
  ui.Front->set_droptype("Image");
  ui.Left->set_droptype("Image");
  ui.Right->set_droptype("Image");
  ui.Back->set_droptype("Image");
  ui.Top->set_droptype("Image");
  ui.Bottom->set_droptype("Image");
}


///////////////////////// SkyboxProperties::edit ////////////////////////////
void SkyboxProperties::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &SkyboxDocument::document_changed, this, &SkyboxProperties::refresh);
  connect(&m_document, &SkyboxDocument::dependant_changed, this, &SkyboxProperties::refresh);

  refresh();
}


///////////////////////// SkyboxProperties::refresh /////////////////////////
void SkyboxProperties::refresh()
{
  ui.Width->setText(QString::number(m_document.width()));
  ui.Height->setText(QString::number(m_document.height()));

  ui.Type->updateValue(static_cast<int>(m_document.type()));

  ui.Stack->setCurrentIndex(ui.Type->currentIndex());

  ui.Front->setPixmap(m_document.image(SkyboxDocument::Image::Front));
  ui.Left->setPixmap(m_document.image(SkyboxDocument::Image::Left));
  ui.Right->setPixmap(m_document.image(SkyboxDocument::Image::Right));
  ui.Back->setPixmap(m_document.image(SkyboxDocument::Image::Back));
  ui.Top->setPixmap(m_document.image(SkyboxDocument::Image::Top));
  ui.Bottom->setPixmap(m_document.image(SkyboxDocument::Image::Bottom));
  ui.EnvMap->setPixmap(m_document.image(SkyboxDocument::Image::EnvMap));
}


///////////////////////// SkyboxProperties::Width ///////////////////////////
void SkyboxProperties::on_Width_textEdited(QString const &text)
{
  m_document.set_width(text.toInt());
}


///////////////////////// SkyboxProperties::Height //////////////////////////
void SkyboxProperties::on_Height_textEdited(QString const &text)
{
  m_document.set_height(text.toInt());
}


///////////////////////// SkyboxProperties::Type ////////////////////////////
void SkyboxProperties::on_Type_valueChanged(int value)
{
  m_document.set_type(static_cast<SkyboxDocument::Type>(value));
}


///////////////////////// SkyboxProperties::Front //////////////////////////
void SkyboxProperties::on_Front_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Front, path);
}


///////////////////////// SkyboxProperties::Left ////////////////////////////
void SkyboxProperties::on_Left_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Left, path);
}


///////////////////////// SkyboxProperties::Right ///////////////////////////
void SkyboxProperties::on_Right_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Right, path);
}


///////////////////////// SkyboxProperties::Back ////////////////////////////
void SkyboxProperties::on_Back_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Back, path);
}


///////////////////////// SkyboxProperties::Top /////////////////////////////
void SkyboxProperties::on_Top_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Top, path);
}


///////////////////////// SkyboxProperties::Bottom //////////////////////////
void SkyboxProperties::on_Bottom_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::Bottom, path);
}


///////////////////////// SkyboxProperties::EnvMap //////////////////////////
void SkyboxProperties::on_EnvMap_itemDropped(QString const &path)
{
  m_document.set_image(SkyboxDocument::Image::EnvMap, path);
}

