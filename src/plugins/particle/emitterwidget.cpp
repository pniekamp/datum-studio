//
// Emitter Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "emitterwidget.h"
#include "curveeditor.h"
#include <QColorDialog>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;
using leap::extentof;

const double rad2deg = 180.0 / pi<double>();
const double deg2rad = pi<double>() / 180.0;

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

  void update_distribution(ParticleSystemDocument::Distribution<float> const &distribution, QcDoubleSpinBox *min, QcDoubleSpinBox *max, double units = 1.0)
  {
    using DistributionType = typename ParticleSystemDocument::Distribution<float>::Type;

    min->updateValue(distribution.ya.front() * units);
    max->updateValue(distribution.ya.back() * units);

    min->setVisible(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    max->setVisible(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    min->setEnabled(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);
    max->setEnabled(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);

  }

  void update_distribution(ParticleSystemDocument::Distribution<Vec3> const &distribution, QcDoubleSpinBox *min, QcDoubleSpinBox *max, double units = 1.0)
  {
    using DistributionType = typename ParticleSystemDocument::Distribution<Vec3>::Type;

    min->updateValue(norm(distribution.ya.front()) * units);
    max->updateValue(norm(distribution.ya.back()) * units);

    min->setVisible(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    max->setVisible(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    min->setEnabled(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);
    max->setEnabled(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);
  }

  void update_distribution(ParticleSystemDocument::Distribution<Color4> const &distribution, QPushButton *min, QPushButton *max)
  {
    using DistributionType = typename ParticleSystemDocument::Distribution<Color4>::Type;

    min->setStyleSheet("background-color: " + qcolor(distribution.ya.front()).name() + ";border: 1px solid black;");
    max->setStyleSheet("background-color: " + qcolor(distribution.ya.back()).name() + ";border: 1px solid black;");

    min->setVisible(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    max->setVisible(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve || distribution.type == DistributionType::UniformCurve);
    min->setEnabled(distribution.type == DistributionType::Constant || distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);
    max->setEnabled(distribution.type == DistributionType::Uniform || distribution.type == DistributionType::Curve);
  }
}


//|---------------------- EmitterWidget -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// EmitterWidget::Constructor ////////////////////////
EmitterWidget::EmitterWidget(QWidget *parent)
  : QWidget(parent)
{
  m_index = -1;

  ui.setupUi(this);
}


///////////////////////// EmitterWidget::edit ///////////////////////////////
void EmitterWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ParticleSystemDocument::document_changed, this, &EmitterWidget::refresh);

  refresh();
}


///////////////////////// EmitterWidget::set_emitter ////////////////////////
void EmitterWidget::set_emitter(int index)
{
  if (m_index!= index)
  {
    m_index = index;

    refresh();
  }
}


///////////////////////// EmitterWidget::refresh ////////////////////////////
void EmitterWidget::refresh()
{
  if (0 <= m_index && m_index < m_document.emitters())
  {
    m_emitter = m_document.emitter(m_index);

    ui.Name->setText(m_emitter.name);
    ui.Duration->updateValue(m_emitter.duration);
    ui.Looping->setChecked(m_emitter.looping);

    ui.Rate->updateValue(m_emitter.rate);

    QcDoubleSpinBox *bursttimes[] = { ui.Burst1Time, ui.Burst2Time, ui.Burst3Time, ui.Burst4Time };

    for(size_t i = 0; i < min(m_emitter.bursttime.size(), extentof(bursttimes)); ++i)
    {
      bursttimes[i]->updateValue(m_emitter.bursttime[i]);
    }

    QcSpinBox *burstcounts[] = { ui.Burst1Count, ui.Burst2Count, ui.Burst3Count, ui.Burst4Count };

    for(size_t i = 0; i < min(m_emitter.burstcount.size(), extentof(burstcounts)); ++i)
    {
      burstcounts[i]->updateValue(m_emitter.burstcount[i]);
    }

    ui.Burst1->setVisible(m_emitter.bursttime.size() > 0);
    ui.Burst2->setVisible(m_emitter.bursttime.size() > 1);
    ui.Burst3->setVisible(m_emitter.bursttime.size() > 2);
    ui.Burst4->setVisible(m_emitter.bursttime.size() > 3);
    ui.BurstAdd->setEnabled(m_emitter.bursttime.size() < 4);

    ui.SizeX->updateValue(m_emitter.size.x);
    ui.SizeY->updateValue(m_emitter.size.y);

    update_distribution(m_emitter.life, ui.LifeMin, ui.LifeMax);
    update_distribution(m_emitter.scale, ui.ScaleMin, ui.ScaleMax);
    update_distribution(m_emitter.rotation, ui.RotationMin, ui.RotationMax, rad2deg);
    update_distribution(m_emitter.velocity, ui.VelocityMin, ui.VelocityMax);
    update_distribution(m_emitter.color, ui.ColorMin, ui.ColorMax);
    update_distribution(m_emitter.layer, ui.LayerMin, ui.LayerMax);

    ui.Accelerated->setChecked(m_emitter.accelerated);
    ui.AccelerationX->updateValue(m_emitter.acceleration.x);
    ui.AccelerationY->updateValue(m_emitter.acceleration.y);
    ui.AccelerationZ->updateValue(m_emitter.acceleration.z);

    ui.Shaped->setChecked(m_emitter.shaped);
    ui.Shape->setCurrentIndex(m_emitter.shape);
    ui.ShapeRadius->updateValue(m_emitter.shaperadius);
    ui.ShapeAngle->updateValue(m_emitter.shapeangle * rad2deg);

    ui.Scaled->setChecked(m_emitter.scaled);
    update_distribution(m_emitter.scaleoverlife, ui.ScaleOverLifeMin, ui.ScaleOverLifeMax);

    ui.Rotated->setChecked(m_emitter.rotated);
    update_distribution(m_emitter.rotateoverlife, ui.RotateOverLifeMin, ui.RotateOverLifeMax, rad2deg);

    ui.Colored->setChecked(m_emitter.colored);
    update_distribution(m_emitter.coloroverlife, ui.ColorOverLifeMin, ui.ColorOverLifeMax);

    ui.Animated->setChecked(m_emitter.animated);
    ui.LayerStart->updateValue(m_emitter.layerstart);
    ui.LayerCount->updateValue(m_emitter.layercount);
    update_distribution(m_emitter.layerrate, ui.LayerRateMin, ui.LayerRateMax);

    ui.Stretched->setChecked(m_emitter.stretched);
    ui.StretchMin->updateValue(m_emitter.velocitystretchmin);
    ui.StretchMax->updateValue(m_emitter.velocitystretchmax);

    ui.Aligned->setChecked(m_emitter.aligned);
    ui.AlignedAxisX->updateValue(m_emitter.alignedaxis.x);
    ui.AlignedAxisY->updateValue(m_emitter.alignedaxis.y);
    ui.AlignedAxisZ->updateValue(m_emitter.alignedaxis.z);
  }

  update();
}


///////////////////////// EmitterWidget::Duration ///////////////////////////
void EmitterWidget::on_Duration_valueChanged(double value)
{
  m_emitter.duration = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Looping ////////////////////////////
void EmitterWidget::on_Looping_clicked(bool value)
{
  m_emitter.looping = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Rate ///////////////////////////////
void EmitterWidget::on_Rate_valueChanged(double value)
{
  m_emitter.rate = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst1Time /////////////////////////
void EmitterWidget::on_Burst1Time_valueChanged(double value)
{
  m_emitter.bursttime[0] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst1Count ////////////////////////
void EmitterWidget::on_Burst1Count_valueChanged(int value)
{
  m_emitter.burstcount[0] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst1Remove ///////////////////////
void EmitterWidget::on_Burst1Remove_clicked()
{
  m_emitter.bursttime.erase(m_emitter.bursttime.begin() + 0);
  m_emitter.burstcount.erase(m_emitter.burstcount.begin() + 0);

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst2Time /////////////////////////
void EmitterWidget::on_Burst2Time_valueChanged(double value)
{
  m_emitter.bursttime[1] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst2Count ////////////////////////
void EmitterWidget::on_Burst2Count_valueChanged(int value)
{
  m_emitter.burstcount[1] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst2Remove ///////////////////////
void EmitterWidget::on_Burst2Remove_clicked()
{
  m_emitter.bursttime.erase(m_emitter.bursttime.begin() + 1);
  m_emitter.burstcount.erase(m_emitter.burstcount.begin() + 1);

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst3Time /////////////////////////
void EmitterWidget::on_Burst3Time_valueChanged(double value)
{
  m_emitter.bursttime[2] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst3Count ////////////////////////
void EmitterWidget::on_Burst3Count_valueChanged(int value)
{
  m_emitter.burstcount[2] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst3Remove ///////////////////////
void EmitterWidget::on_Burst3Remove_clicked()
{
  m_emitter.bursttime.erase(m_emitter.bursttime.begin() + 2);
  m_emitter.burstcount.erase(m_emitter.burstcount.begin() + 2);

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst4Time /////////////////////////
void EmitterWidget::on_Burst4Time_valueChanged(double value)
{
  m_emitter.bursttime[3] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst4Count ////////////////////////
void EmitterWidget::on_Burst4Count_valueChanged(int value)
{
  m_emitter.burstcount[3] = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Burst4Remove ///////////////////////
void EmitterWidget::on_Burst4Remove_clicked()
{
  m_emitter.bursttime.erase(m_emitter.bursttime.begin() + 3);
  m_emitter.burstcount.erase(m_emitter.burstcount.begin() + 3);

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::BurstAdd ///////////////////////////
void EmitterWidget::on_BurstAdd_clicked()
{
  m_emitter.bursttime.push_back(0.0f);
  m_emitter.burstcount.push_back(10);

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LifeMin ////////////////////////////
void EmitterWidget::on_LifeMin_valueChanged(double value)
{
  m_emitter.life.ya.front() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LifeMax ////////////////////////////
void EmitterWidget::on_LifeMax_valueChanged(double value)
{
  m_emitter.life.ya.back() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LifeEdit ///////////////////////////
void EmitterWidget::on_LifeEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.life, 0.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.life = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::SizeX //////////////////////////////
void EmitterWidget::on_SizeX_valueChanged(double value)
{
  m_emitter.size.x = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::SizeY //////////////////////////////
void EmitterWidget::on_SizeY_valueChanged(double value)
{
  m_emitter.size.y = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleMin ///////////////////////////
void EmitterWidget::on_ScaleMin_valueChanged(double value)
{
  m_emitter.scale.ya.front() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleMax ///////////////////////////
void EmitterWidget::on_ScaleMax_valueChanged(double value)
{
  m_emitter.scale.ya.back() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleEdit //////////////////////////
void EmitterWidget::on_ScaleEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.scale, 0.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.scale = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::RotationMin ////////////////////////
void EmitterWidget::on_RotationMin_valueChanged(double value)
{
  m_emitter.rotation.ya.front() = value * deg2rad;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::RotationMax ////////////////////////
void EmitterWidget::on_RotationMax_valueChanged(double value)
{
  m_emitter.rotation.ya.back() = value * deg2rad;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::RotationEdit ///////////////////////
void EmitterWidget::on_RotationEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.rotation, -2*pi<float>(), 2*pi<float>());

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.rotation = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::VelocityMin ////////////////////////
void EmitterWidget::on_VelocityMin_valueChanged(double value)
{
  m_emitter.velocity.ya.front() = value * safenormalise(m_emitter.velocity.ya.front(), Vec3(1.0f, 0.0f, 0.0f));

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::VelocityMax ////////////////////////
void EmitterWidget::on_VelocityMax_valueChanged(double value)
{
  m_emitter.velocity.ya.back() = value * safenormalise(m_emitter.velocity.ya.back(), Vec3(1.0f, 0.0f, 0.0f));

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::VelocityEdit ///////////////////////
void EmitterWidget::on_VelocityEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.velocity, Vec3(-500.0f, -500.0f, -500.0f), Vec3(500.0f, 500.0f, 500.0f));

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.velocity = dlg.distribution<Vec3>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::ColorMin ///////////////////////////
void EmitterWidget::on_ColorMin_clicked()
{
  QColorDialog dlg(this);
  dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);

  dlg.setCurrentColor(qcolor(unpremultiply(m_emitter.color.ya.front())));

  connect(&dlg, &QColorDialog::currentColorChanged, this, [&](QColor const &color) { m_emitter.color.ya.front() = premultiply(qcolor(color)); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::ColorMax ///////////////////////////
void EmitterWidget::on_ColorMax_clicked()
{
  QColorDialog dlg(this);
  dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);

  dlg.setCurrentColor(qcolor(unpremultiply(m_emitter.color.ya.back())));

  connect(&dlg, &QColorDialog::currentColorChanged, this, [&](QColor const &color) { m_emitter.color.ya.back() = premultiply(qcolor(color)); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::ColorEdit //////////////////////////
void EmitterWidget::on_ColorEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.color, Color4(0.0f, 0.0f, 0.0f, 0.0f), Color4(500.0f, 500.0f, 500.0f, 1.0f));

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.color = dlg.distribution<Color4>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::LayerMin ///////////////////////////
void EmitterWidget::on_LayerMin_valueChanged(double value)
{
  m_emitter.layer.ya.front() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerMax ///////////////////////////
void EmitterWidget::on_LayerMax_valueChanged(double value)
{
  m_emitter.layer.ya.back() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerEdit //////////////////////////
void EmitterWidget::on_LayerEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.layer, 0.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.layer = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::Accelerated ////////////////////////
void EmitterWidget::on_Accelerated_toggled(bool checked)
{
  m_emitter.accelerated = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AccelerationX //////////////////////
void EmitterWidget::on_AccelerationX_valueChanged(double value)
{
  m_emitter.acceleration.x = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AccelerationY //////////////////////
void EmitterWidget::on_AccelerationY_valueChanged(double value)
{
  m_emitter.acceleration.y = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AccelerationZ //////////////////////
void EmitterWidget::on_AccelerationZ_valueChanged(double value)
{
  m_emitter.acceleration.z = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Shaped /////////////////////////////
void EmitterWidget::on_Shaped_toggled(bool checked)
{
  m_emitter.shaped = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Shape //////////////////////////////
void EmitterWidget::on_Shape_activated(int index)
{
  m_emitter.shape = index;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ShapeRadius ////////////////////////
void EmitterWidget::on_ShapeRadius_valueChanged(double value)
{
  m_emitter.shaperadius = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ShapeAngle /////////////////////////
void EmitterWidget::on_ShapeAngle_valueChanged(double value)
{
  m_emitter.shapeangle = value * deg2rad;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Scaled /////////////////////////////
void EmitterWidget::on_Scaled_toggled(bool checked)
{
  m_emitter.scaled = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleOverLifeMin ///////////////////
void EmitterWidget::on_ScaleOverLifeMin_valueChanged(double value)
{
  m_emitter.scaleoverlife.ya.front() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleOverLifeMax ///////////////////
void EmitterWidget::on_ScaleOverLifeMax_valueChanged(double value)
{
  m_emitter.scaleoverlife.ya.back() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ScaleOverLifeEdit //////////////////
void EmitterWidget::on_ScaleOverLifeEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.scaleoverlife, 0.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.scaleoverlife = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::Rotated ////////////////////////////
void EmitterWidget::on_Rotated_toggled(bool checked)
{
  m_emitter.rotated = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::RotateOverLifeMin //////////////////
void EmitterWidget::on_RotateOverLifeMin_valueChanged(double value)
{
  m_emitter.rotateoverlife.ya.front() = value * deg2rad;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::RotateOverLifeMax //////////////////
void EmitterWidget::on_RotateOverLifeMax_valueChanged(double value)
{
  m_emitter.rotateoverlife.ya.back() = value * deg2rad;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::RotateOverLifeEdit /////////////////
void EmitterWidget::on_RotateOverLifeEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.rotateoverlife, -500.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.rotateoverlife = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::Colored ////////////////////////////
void EmitterWidget::on_Colored_toggled(bool checked)
{
  m_emitter.colored = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::ColorOverLifeMin ///////////////////
void EmitterWidget::on_ColorOverLifeMin_clicked()
{
  QColorDialog dlg(this);
  dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);

  dlg.setCurrentColor(qcolor(unpremultiply(m_emitter.coloroverlife.ya.front())));

  connect(&dlg, &QColorDialog::currentColorChanged, this, [&](QColor const &color) { m_emitter.coloroverlife.ya.front() = premultiply(qcolor(color)); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::ColorOverLifeMax ///////////////////
void EmitterWidget::on_ColorOverLifeMax_clicked()
{
  QColorDialog dlg(this);
  dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);

  dlg.setCurrentColor(qcolor(unpremultiply(m_emitter.coloroverlife.ya.back())));

  connect(&dlg, &QColorDialog::currentColorChanged, this, [&](QColor const &color) { m_emitter.coloroverlife.ya.back() = premultiply(qcolor(color)); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::ColorOverLifeEdit //////////////////
void EmitterWidget::on_ColorOverLifeEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.coloroverlife, Color4(0.0f, 0.0f, 0.0f, 0.0f), Color4(500.0f, 500.0f, 500.0f, 1.0f));

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.coloroverlife = dlg.distribution<Color4>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}



///////////////////////// EmitterWidget::Animated ///////////////////////////
void EmitterWidget::on_Animated_toggled(bool checked)
{
  m_emitter.animated = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerStart /////////////////////////
void EmitterWidget::on_LayerStart_valueChanged(double value)
{
  m_emitter.layerstart = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerCount /////////////////////////
void EmitterWidget::on_LayerCount_valueChanged(double value)
{
  m_emitter.layercount = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerRateMin ///////////////////////
void EmitterWidget::on_LayerRateMin_valueChanged(double value)
{
  m_emitter.layerrate.ya.front() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerRateMax ///////////////////////
void EmitterWidget::on_LayerRateMax_valueChanged(double value)
{
  m_emitter.layerrate.ya.back() = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::LayerRateEdit //////////////////////
void EmitterWidget::on_LayerRateEdit_clicked()
{
  CurveEditor dlg(this);

  dlg.set_distribution(m_emitter.layerrate, -500.0f, 500.0f);

  connect(&dlg, &CurveEditor::distribution_changed, this, [&]() { m_emitter.layerrate = dlg.distribution<float>(); m_document.update_emitter(m_index, m_emitter); });

  dlg.exec();
}


///////////////////////// EmitterWidget::Stretched //////////////////////////
void EmitterWidget::on_Stretched_toggled(bool checked)
{
  m_emitter.stretched = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::StretchMin /////////////////////////
void EmitterWidget::on_StretchMin_valueChanged(double value)
{
  m_emitter.velocitystretchmin = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::StretchMax /////////////////////////
void EmitterWidget::on_StretchMax_valueChanged(double value)
{
  m_emitter.velocitystretchmax = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::Aligned ////////////////////////////
void EmitterWidget::on_Aligned_toggled(bool checked)
{
  m_emitter.aligned = checked;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AlignedAxisX ///////////////////////
void EmitterWidget::on_AlignedAxisX_valueChanged(double value)
{
  m_emitter.alignedaxis.x = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AlignedAxisY ///////////////////////
void EmitterWidget::on_AlignedAxisY_valueChanged(double value)
{
  m_emitter.alignedaxis.y = value;

  m_document.update_emitter(m_index, m_emitter);
}


///////////////////////// EmitterWidget::AlignedAxisZ ///////////////////////
void EmitterWidget::on_AlignedAxisZ_valueChanged(double value)
{
  m_emitter.alignedaxis.z = value;

  m_document.update_emitter(m_index, m_emitter);
}
