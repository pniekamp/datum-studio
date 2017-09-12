//
// Material Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "materialwidget.h"
#include <QPainter>
#include <QColorDialog>

#include <QDebug>

using namespace std;
using namespace lml;

namespace
{
  QColor qcolor(Color4 const &color)
  {
    return QColor(clamp(color.r, 0.0f, 1.0f) * 255, clamp(color.g, 0.0f, 1.0f) * 255, clamp(color.b, 0.0f, 1.0f) * 255, clamp(color.a, 0.0f, 1.0f) * 255);
  }

  Color4 qcolor(QColor const &color)
  {
    return Color4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
  }
}

//|---------------------- MaterialWidget ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialWidget::Constructor ///////////////////////
MaterialWidget::MaterialWidget(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);

  ui.SurfaceMap->set_droptype("Image");
  ui.NormalMap->set_droptype("Image");
}


///////////////////////// MaterialWidget::edit //////////////////////////////
void MaterialWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &OceanMaterialDocument::document_changed, this, &MaterialWidget::refresh);
  connect(&m_document, &OceanMaterialDocument::dependant_changed, this, &MaterialWidget::refresh);

  refresh();
}


///////////////////////// MaterialWidget::refresh ///////////////////////////
void MaterialWidget::refresh()
{
  ui.ShallowColor->setStyleSheet("background-color: " + qcolor(m_document.shallowcolor()).name() + ";border: 1px solid black;");
  ui.DeepColor->setStyleSheet("background-color: " + qcolor(m_document.deepcolor() * 10).name() + ";border: 1px solid black;");
  ui.FresnelColor->setStyleSheet("background-color: " + qcolor(m_document.fresnelcolor() * 10).name() + ";border: 1px solid black;");

  ui.DepthScale->updateValue(m_document.depthscale());

  ui.RoughnessSlider->updateValue(m_document.roughness());
  ui.RoughnessSpinner->updateValue(m_document.roughness());

  ui.ReflectivitySlider->updateValue(m_document.reflectivity());
  ui.ReflectivitySpinner->updateValue(m_document.reflectivity());

  ui.EmissiveSlider->updateValue(m_document.emissive());
  ui.EmissiveSpinner->updateValue(m_document.emissive());

  ui.SurfaceMap->setPixmap(m_document.image(OceanMaterialDocument::Image::SurfaceMap));

  ui.NormalMap->setPixmap(m_document.image(OceanMaterialDocument::Image::NormalMap));

  update();
}


///////////////////////// MaterialWidget::ShallowColor //////////////////////
void MaterialWidget::on_ShallowColor_clicked()
{
  QColorDialog dlg(this);

  dlg.setCurrentColor(qcolor(m_document.shallowcolor()));

  if (dlg.exec() == QDialog::Accepted)
  {
    m_document.set_shallowcolor(qcolor(dlg.currentColor()).rgb);
  }
}


///////////////////////// MaterialWidget::DeepColor /////////////////////////
void MaterialWidget::on_DeepColor_clicked()
{
  QColorDialog dlg(this);

  dlg.setCurrentColor(qcolor(m_document.deepcolor() * 10));

  if (dlg.exec() == QDialog::Accepted)
  {
    m_document.set_deepcolor(qcolor(dlg.currentColor()).rgb / 10);
  }
}


///////////////////////// MaterialWidget::FresnelColor //////////////////////
void MaterialWidget::on_FresnelColor_clicked()
{
  QColorDialog dlg(this);

  dlg.setCurrentColor(qcolor(m_document.fresnelcolor() * 10));

  if (dlg.exec() == QDialog::Accepted)
  {
    m_document.set_fresnelcolor(qcolor(dlg.currentColor()).rgb / 10);
  }
}


///////////////////////// MaterialWidget::DepthScale ////////////////////////
void MaterialWidget::on_DepthScale_valueChanged(double value)
{
  m_document.set_depthscale(value);
}


///////////////////////// MaterialWidget::RoughnessSlider ///////////////////
void MaterialWidget::on_RoughnessSlider_valueChanged(double value)
{
  m_document.set_roughness(value);
}


///////////////////////// MaterialWidget::RoughnessSpinner //////////////////
void MaterialWidget::on_RoughnessSpinner_valueChanged(double value)
{
  m_document.set_roughness(value);
}


///////////////////////// MaterialWidget::ReflectivitySlider ////////////////
void MaterialWidget::on_ReflectivitySlider_valueChanged(double value)
{
  m_document.set_reflectivity(value);
}


///////////////////////// MaterialWidget::ReflectivitySpinner ///////////////
void MaterialWidget::on_ReflectivitySpinner_valueChanged(double value)
{
  m_document.set_reflectivity(value);
}


///////////////////////// MaterialWidget::EmissiveSlider ////////////////////
void MaterialWidget::on_EmissiveSlider_valueChanged(double value)
{
  m_document.set_emissive(value);
}


///////////////////////// MaterialWidget::EmissiveSpinner ///////////////////
void MaterialWidget::on_EmissiveSpinner_valueChanged(double value)
{
  m_document.set_emissive(value);
}


///////////////////////// MaterialWidget::SurfaceMap ////////////////////////
void MaterialWidget::on_SurfaceMap_itemDropped(QString const &path)
{
  m_document.set_image(OceanMaterialDocument::Image::SurfaceMap, path);
}


///////////////////////// MaterialWidget::NormalMap /////////////////////////
void MaterialWidget::on_NormalMap_itemDropped(QString const &path)
{
  m_document.set_image(OceanMaterialDocument::Image::NormalMap, path);
}


///////////////////////// MaterialWidget::ResetAlbedo ///////////////////////
void MaterialWidget::on_ResetAlbedo_clicked()
{
  m_document.set_color(Color4(1.0f, 1.0f, 1.0f, 1.0f));
  m_document.set_shallowcolor(Color3(1.0f, 1.0f, 1.0f));
  m_document.set_deepcolor(Color3(1.0f, 1.0f, 1.0f));
  m_document.set_fresnelcolor(Color3(1.0f, 1.0f, 1.0f));

  m_document.set_metalness(0);
  m_document.set_roughness(0.4f);
  m_document.set_reflectivity(0.5f);
  m_document.set_emissive(0);
}


///////////////////////// MaterialWidget::ResetSurface //////////////////////
void MaterialWidget::on_ResetSurface_clicked()
{
  m_document.set_image(OceanMaterialDocument::Image::SurfaceMap, "");
}


///////////////////////// MaterialWidget::ResetNormal ///////////////////////
void MaterialWidget::on_ResetNormal_clicked()
{
  m_document.set_image(OceanMaterialDocument::Image::NormalMap, "");
}


///////////////////////// MaterialWidget::paintEvent ////////////////////////
void MaterialWidget::paintEvent(QPaintEvent *event)
{
  QPainterPath path;

  auto wire = [&](QWidget const *left, QWidget const *right, int in, int out) {

    auto a = QPoint(ui.Wiring->pos().x(), left->mapTo(this, left->rect().center()).y() + 2);
    auto b = QPoint(ui.Wiring->pos().x() + ui.Wiring->width(), right->mapTo(this, right->rect().center()).y() + 2);

    path.moveTo(a);
    path.cubicTo(a + QPoint(in, 0), b - QPoint(out, 0), b);
  };

  wire(ui.AlbedoOutput, ui.AlbedoInput, 20, 30);
  wire(ui.SurfaceOutput, ui.SurfaceInput, 40, 90);
  wire(ui.NormalOutput, ui.NormalInput, 60, 30);

  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(QPen(Qt::darkGray, 2));

  painter.drawPath(path);
}
