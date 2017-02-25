//
// Material Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialwidget.h"
#include <QPainter>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MaterialWidget ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialWidget::Constructor ///////////////////////
MaterialWidget::MaterialWidget(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);

  ui.AlbedoMap->set_droptype("Image");
  ui.AlbedoMask->set_droptype("Image");
  ui.MetalnessMap->set_droptype("Image");
  ui.RoughnessMap->set_droptype("Image");
  ui.ReflectivityMap->set_droptype("Image");
  ui.NormalMap->set_droptype("Image");
}


///////////////////////// MaterialWidget::edit //////////////////////////////
void MaterialWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &MaterialDocument::document_changed, this, &MaterialWidget::refresh);
  connect(&m_document, &MaterialDocument::dependant_changed, this, &MaterialWidget::refresh);

  refresh();
}


///////////////////////// MaterialWidget::refresh ///////////////////////////
void MaterialWidget::refresh()
{
  ui.ShaderList->setCurrentIndex(static_cast<int>(m_document.shader()));

  ui.AlbedoMap->setPixmap(m_document.image(MaterialDocument::Image::AlbedoMap));
  ui.AlbedoMask->setPixmap(m_document.image(MaterialDocument::Image::AlbedoMask));
  ui.TintRedSlider->updateValue(m_document.color().r);
  ui.TintRedSpinner->updateValue(m_document.color().r);
  ui.TintGreenSlider->updateValue(m_document.color().g);
  ui.TintGreenSpinner->updateValue(m_document.color().g);
  ui.TintBlueSlider->updateValue(m_document.color().b);
  ui.TintBlueSpinner->updateValue(m_document.color().b);  
  ui.TintAlphaSlider->updateValue(m_document.color().a);
  ui.TintAlphaSpinner->updateValue(m_document.color().a);
  ui.EmissiveSlider->updateValue(m_document.emissive());
  ui.EmissiveSpinner->updateValue(m_document.emissive());
  ui.AlbedoOutput->setChecked(true);

  ui.MetalnessMap->setPixmap(m_document.image(MaterialDocument::Image::MetalnessMap));
  ui.MetalnessSlider->updateValue(m_document.metalness());
  ui.MetalnessSpinner->updateValue(m_document.metalness());
  ui.MetalnessOutput1->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::r);
  ui.MetalnessOutput2->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::g);
  ui.MetalnessOutput3->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::b);
  ui.MetalnessOutput4->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::a);
  ui.MetalnessOutput5->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::invr);
  ui.MetalnessOutput6->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::invg);
  ui.MetalnessOutput7->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::invb);
  ui.MetalnessOutput8->setChecked(m_document.metalnessoutput() == MaterialDocument::MetalnessOutput::inva);

  ui.RoughnessMap->setPixmap(m_document.image(MaterialDocument::Image::RoughnessMap));
  ui.RoughnessSlider->updateValue(m_document.roughness());
  ui.RoughnessSpinner->updateValue(m_document.roughness());
  ui.RoughnessOutput1->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::r);
  ui.RoughnessOutput2->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::g);
  ui.RoughnessOutput3->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::b);
  ui.RoughnessOutput4->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::a);
  ui.RoughnessOutput5->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::invr);
  ui.RoughnessOutput6->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::invg);
  ui.RoughnessOutput7->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::invb);
  ui.RoughnessOutput8->setChecked(m_document.roughnessoutput() == MaterialDocument::RoughnessOutput::inva);

  ui.ReflectivityMap->setPixmap(m_document.image(MaterialDocument::Image::ReflectivityMap));
  ui.ReflectivitySlider->updateValue(m_document.reflectivity());
  ui.ReflectivitySpinner->updateValue(m_document.reflectivity());
  ui.ReflectivityOutput1->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::r);
  ui.ReflectivityOutput2->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::g);
  ui.ReflectivityOutput3->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::b);
  ui.ReflectivityOutput4->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::a);
  ui.ReflectivityOutput5->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::invr);
  ui.ReflectivityOutput6->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::invg);
  ui.ReflectivityOutput7->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::invb);
  ui.ReflectivityOutput8->setChecked(m_document.reflectivityoutput() == MaterialDocument::ReflectivityOutput::inva);

  ui.NormalMap->setPixmap(m_document.image(MaterialDocument::Image::NormalMap));
  ui.NormalOutput1->setChecked(m_document.normaloutput() == MaterialDocument::NormalOutput::xyz);
  ui.NormalOutput2->setChecked(m_document.normaloutput() == MaterialDocument::NormalOutput::xinvyz);
  ui.NormalOutput3->setChecked(m_document.normaloutput() == MaterialDocument::NormalOutput::bump);

  update();
}


///////////////////////// MaterialWidget::Shader ////////////////////////////
void MaterialWidget::on_ShaderList_activated(int index)
{
  m_document.set_shader(static_cast<MaterialDocument::Shader>(index));
}


///////////////////////// MaterialWidget::AlbedoMap /////////////////////////
void MaterialWidget::on_AlbedoMap_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::AlbedoMap, path);
}


///////////////////////// MaterialWidget::AlbedoMask ////////////////////////
void MaterialWidget::on_AlbedoMask_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::AlbedoMask, path);
}


///////////////////////// MaterialWidget::TintRedSlider /////////////////////
void MaterialWidget::on_TintRedSlider_valueChanged(double value)
{
  m_document.set_color(Color4(value, ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value(), ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintRedSpinner ////////////////////
void MaterialWidget::on_TintRedSpinner_valueChanged(double value)
{
  m_document.set_color(Color4(value, ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value(), ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintGreenSlider ///////////////////
void MaterialWidget::on_TintGreenSlider_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), value, ui.TintBlueSpinner->value(), ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintGreenSpinner //////////////////
void MaterialWidget::on_TintGreenSpinner_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), value, ui.TintBlueSpinner->value(), ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintBlueSlider ////////////////////
void MaterialWidget::on_TintBlueSlider_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), value, ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintBlueSpinner ///////////////////
void MaterialWidget::on_TintBlueSpinner_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), value, ui.TintAlphaSpinner->value()));
}


///////////////////////// MaterialWidget::TintAlphaSlider ///////////////////
void MaterialWidget::on_TintAlphaSlider_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value(), value));
}


///////////////////////// MaterialWidget::TintAlphaSpinner //////////////////
void MaterialWidget::on_TintAlphaSpinner_valueChanged(double value)
{
  m_document.set_color(Color4(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value(), value));
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


///////////////////////// MaterialWidget::AlbedoOutput //////////////////////
void MaterialWidget::on_AlbedoOutput_clicked()
{
  m_document.set_albedooutput(MaterialDocument::AlbedoOutput::rgba);
}


///////////////////////// MaterialWidget::MetalnessMap //////////////////////
void MaterialWidget::on_MetalnessMap_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::MetalnessMap, path);
}


///////////////////////// MaterialWidget::MetalnessSlider ///////////////////
void MaterialWidget::on_MetalnessSlider_valueChanged(double value)
{
  m_document.set_metalness(value);
}


///////////////////////// MaterialWidget::MetalnessSpinner //////////////////
void MaterialWidget::on_MetalnessSpinner_valueChanged(double value)
{
  m_document.set_metalness(value);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput1_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::r);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput2_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::g);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput3_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::b);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput4_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::a);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput5_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::invr);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput6_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::invg);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput7_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::invb);
}


///////////////////////// MaterialWidget::MetalnessOutput ///////////////////
void MaterialWidget::on_MetalnessOutput8_clicked()
{
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::inva);
}


///////////////////////// MaterialWidget::RoughnessMap //////////////////////
void MaterialWidget::on_RoughnessMap_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::RoughnessMap, path);
}


///////////////////////// MaterialWidget::RoughnessSlider ///////////////////
void MaterialWidget::on_RoughnessSlider_valueChanged(double value)
{
  m_document.set_roughness(value);
}


///////////////////////// MaterialWidget::RouchnessSpinner //////////////////
void MaterialWidget::on_RoughnessSpinner_valueChanged(double value)
{
  m_document.set_roughness(value);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput1_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::r);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput2_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::g);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput3_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::b);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput4_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::a);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput5_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::invr);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput6_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::invg);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput7_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::invb);
}


///////////////////////// MaterialWidget::RoughnessOutput ///////////////////
void MaterialWidget::on_RoughnessOutput8_clicked()
{
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::inva);
}


///////////////////////// MaterialWidget::ReflectivityMap ///////////////////
void MaterialWidget::on_ReflectivityMap_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::ReflectivityMap, path);
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


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput1_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::r);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput2_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::g);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput3_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::b);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput4_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::a);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput5_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::invr);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput6_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::invg);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput7_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::invb);
}


///////////////////////// MaterialWidget::ReflectivityOutput ////////////////
void MaterialWidget::on_ReflectivityOutput8_clicked()
{
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::inva);
}


///////////////////////// MaterialWidget::NormalMap /////////////////////////
void MaterialWidget::on_NormalMap_itemDropped(QString const &path)
{
  m_document.set_image(MaterialDocument::Image::NormalMap, path);
}


///////////////////////// MaterialWidget::NormalOutput //////////////////////
void MaterialWidget::on_NormalOutput1_clicked()
{
  m_document.set_normaloutput(MaterialDocument::NormalOutput::xyz);
}


///////////////////////// MaterialWidget::NormalOutput //////////////////////
void MaterialWidget::on_NormalOutput2_clicked()
{
  m_document.set_normaloutput(MaterialDocument::NormalOutput::xinvyz);
}


///////////////////////// MaterialWidget::NormalOutput //////////////////////
void MaterialWidget::on_NormalOutput3_clicked()
{
  m_document.set_normaloutput(MaterialDocument::NormalOutput::bump);
}


///////////////////////// MaterialWidget::ResetAlbedo ///////////////////////
void MaterialWidget::on_ResetAlbedo_clicked()
{
  m_document.set_image(MaterialDocument::Image::AlbedoMap, "");
  m_document.set_image(MaterialDocument::Image::AlbedoMask, "");
  m_document.set_color(Color4(1.0f, 1.0f, 1.0f, 1.0f));
  m_document.set_emissive(0);
  m_document.set_albedooutput(MaterialDocument::AlbedoOutput::rgba);
}


///////////////////////// MaterialWidget::ResetMetalness ////////////////////
void MaterialWidget::on_ResetMetalness_clicked()
{
  m_document.set_image(MaterialDocument::Image::MetalnessMap, "");
  m_document.set_metalness(1);
  m_document.set_metalnessoutput(MaterialDocument::MetalnessOutput::r);
}


///////////////////////// MaterialWidget::ResetRoughness ////////////////////
void MaterialWidget::on_ResetRoughness_clicked()
{
  m_document.set_image(MaterialDocument::Image::RoughnessMap, "");
  m_document.set_roughness(1);
  m_document.set_roughnessoutput(MaterialDocument::RoughnessOutput::a);
}


///////////////////////// MaterialWidget::ResetReflectivity /////////////////
void MaterialWidget::on_ResetReflectivity_clicked()
{
  m_document.set_image(MaterialDocument::Image::ReflectivityMap, "");
  m_document.set_reflectivity(0.5f);
  m_document.set_reflectivityoutput(MaterialDocument::ReflectivityOutput::g);
}


///////////////////////// MaterialWidget::ResetNormal ///////////////////////
void MaterialWidget::on_ResetNormal_clicked()
{
  m_document.set_image(MaterialDocument::Image::NormalMap, "");
  m_document.set_normaloutput(MaterialDocument::NormalOutput::xyz);
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

  wire(ui.ShaderOutput, ui.ShaderInput, 10, 10);

  wire(ui.AlbedoOutput, ui.AlbedoInput, 20, 30);

  if (ui.MetalnessOutput1->isChecked()) wire(ui.MetalnessOutput1, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput2->isChecked()) wire(ui.MetalnessOutput2, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput3->isChecked()) wire(ui.MetalnessOutput3, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput4->isChecked()) wire(ui.MetalnessOutput4, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput5->isChecked()) wire(ui.MetalnessOutput5, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput6->isChecked()) wire(ui.MetalnessOutput6, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput7->isChecked()) wire(ui.MetalnessOutput7, ui.MetalnessInput, 40, 90);
  if (ui.MetalnessOutput8->isChecked()) wire(ui.MetalnessOutput8, ui.MetalnessInput, 40, 90);

  if (ui.RoughnessOutput1->isChecked()) wire(ui.RoughnessOutput1, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput2->isChecked()) wire(ui.RoughnessOutput2, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput3->isChecked()) wire(ui.RoughnessOutput3, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput4->isChecked()) wire(ui.RoughnessOutput4, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput5->isChecked()) wire(ui.RoughnessOutput5, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput6->isChecked()) wire(ui.RoughnessOutput6, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput7->isChecked()) wire(ui.RoughnessOutput7, ui.RoughnessInput, 60, 70);
  if (ui.RoughnessOutput8->isChecked()) wire(ui.RoughnessOutput8, ui.RoughnessInput, 60, 70);

  if (ui.ReflectivityOutput1->isChecked()) wire(ui.ReflectivityOutput1, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput2->isChecked()) wire(ui.ReflectivityOutput2, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput3->isChecked()) wire(ui.ReflectivityOutput3, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput4->isChecked()) wire(ui.ReflectivityOutput4, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput5->isChecked()) wire(ui.ReflectivityOutput5, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput6->isChecked()) wire(ui.ReflectivityOutput6, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput7->isChecked()) wire(ui.ReflectivityOutput7, ui.ReflectivityInput, 80, 50);
  if (ui.ReflectivityOutput8->isChecked()) wire(ui.ReflectivityOutput8, ui.ReflectivityInput, 80, 50);

  if (ui.NormalOutput1->isChecked()) wire(ui.NormalOutput1, ui.NormalInput, 100, 30);
  if (ui.NormalOutput2->isChecked()) wire(ui.NormalOutput2, ui.NormalInput, 100, 30);
  if (ui.NormalOutput3->isChecked()) wire(ui.NormalOutput3, ui.NormalInput, 100, 30);

  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(QPen(Qt::darkGray, 2));

  painter.drawPath(path);
}
