//
// FontProperties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "fontproperties.h"
#include <QFontDialog>

#include <QDebug>

using namespace std;

//|---------------------- FontProperties ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// FontProperties::Constructor ///////////////////////
FontProperties::FontProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this); 

  ui.AtlasWidth->setValidator(new QIntValidator(0, 9999, this));
  ui.AtlasHeight->setValidator(new QIntValidator(0, 9999, this));
}


///////////////////////// FontProperties::edit //////////////////////////////
void FontProperties::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &FontDocument::document_changed, this, &FontProperties::refresh);

  refresh();
}


///////////////////////// FontProperties::refresh ///////////////////////////
void FontProperties::refresh()
{
  ui.FontName->setText(QString("%1 %2pt").arg(m_document.name()).arg(m_document.size()));

  ui.AtlasWidth->setText(QString::number(m_document.atlaswidth()));
  ui.AtlasHeight->setText(QString::number(m_document.atlasheight()));
}


///////////////////////// FontProperties::Font //////////////////////////////
void FontProperties::on_FontButton_clicked()
{
  bool ok;

  QFont font = QFontDialog::getFont(&ok, m_document.font(), this);

  if (ok)
  {
    m_document.set_font(font);
  }
}


/////////////////////////// FontProperties::AtlasWidth //////////////////////
void FontProperties::on_AtlasWidth_textEdited(QString const &text)
{
  m_document.set_atlaswidth(text.toInt());
}


/////////////////////////// FontProperties::AtlasHeight /////////////////////
void FontProperties::on_AtlasHeight_textEdited(QString const &text)
{
  m_document.set_atlasheight(text.toInt());
}
