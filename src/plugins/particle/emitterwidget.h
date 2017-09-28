//
// Emitter Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "particlesystem.h"
#include "ui_emitterwidget.h"
#include <QWidget>

//-------------------------- EmitterWidget ----------------------------------
//---------------------------------------------------------------------------

class EmitterWidget : public QWidget
{
  Q_OBJECT

  public:
    EmitterWidget(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

    void set_emitter(int index);

  protected slots:

    void refresh();

    void on_Duration_valueChanged(double value);
    void on_Looping_clicked(bool value);

    void on_dx_valueChanged(double value);
    void on_dy_valueChanged(double value);
    void on_dz_valueChanged(double value);
    void on_ax_valueChanged(double value);
    void on_ay_valueChanged(double value);
    void on_az_valueChanged(double value);

    void on_Rate_valueChanged(double value);
    void on_Burst1Time_valueChanged(double value);
    void on_Burst1Count_valueChanged(int value);
    void on_Burst1Remove_clicked();
    void on_Burst2Time_valueChanged(double value);
    void on_Burst2Count_valueChanged(int value);
    void on_Burst2Remove_clicked();
    void on_Burst3Time_valueChanged(double value);
    void on_Burst3Count_valueChanged(int value);
    void on_Burst3Remove_clicked();
    void on_Burst4Time_valueChanged(double value);
    void on_Burst4Count_valueChanged(int value);
    void on_Burst4Remove_clicked();
    void on_BurstAdd_clicked();

    void on_LifeMin_valueChanged(double value);
    void on_LifeMax_valueChanged(double value);
    void on_LifeEdit_clicked();

    void on_SizeX_valueChanged(double value);
    void on_SizeY_valueChanged(double value);

    void on_ScaleMin_valueChanged(double value);
    void on_ScaleMax_valueChanged(double value);
    void on_ScaleEdit_clicked();

    void on_RotationMin_valueChanged(double value);
    void on_RotationMax_valueChanged(double value);
    void on_RotationEdit_clicked();

    void on_VelocityMin_valueChanged(double value);
    void on_VelocityMax_valueChanged(double value);
    void on_VelocityEdit_clicked();

    void on_ColorMin_clicked();
    void on_ColorMax_clicked();
    void on_ColorEdit_clicked();

    void on_LayerMin_valueChanged(double value);
    void on_LayerMax_valueChanged(double value);
    void on_LayerEdit_clicked();

    void on_Accelerated_toggled(bool checked);
    void on_AccelerationX_valueChanged(double value);
    void on_AccelerationY_valueChanged(double value);
    void on_AccelerationZ_valueChanged(double value);

    void on_Shaped_toggled(bool checked);
    void on_Shape_activated(int index);
    void on_ShapeRadius_valueChanged(double value);
    void on_ShapeAngle_valueChanged(double value);

    void on_Scaled_toggled(bool checked);
    void on_ScaleOverLifeMin_valueChanged(double value);
    void on_ScaleOverLifeMax_valueChanged(double value);
    void on_ScaleOverLifeEdit_clicked();

    void on_Rotated_toggled(bool checked);
    void on_RotateOverLifeMin_valueChanged(double value);
    void on_RotateOverLifeMax_valueChanged(double value);
    void on_RotateOverLifeEdit_clicked();

    void on_Colored_toggled(bool checked);
    void on_ColorOverLifeMin_clicked();
    void on_ColorOverLifeMax_clicked();
    void on_ColorOverLifeEdit_clicked();

    void on_Animated_toggled(bool checked);
    void on_LayerStart_valueChanged(double value);
    void on_LayerCount_valueChanged(double value);
    void on_LayerRateMin_valueChanged(double value);
    void on_LayerRateMax_valueChanged(double value);
    void on_LayerRateEdit_clicked();

    void on_Stretched_toggled(bool checked);
    void on_StretchMin_valueChanged(double value);
    void on_StretchMax_valueChanged(double value);

    void on_Aligned_toggled(bool checked);
    void on_AlignedAxisX_valueChanged(double value);
    void on_AlignedAxisY_valueChanged(double value);
    void on_AlignedAxisZ_valueChanged(double value);

  private:

    int m_index;

    Ui::EmitterWidget ui;

    ParticleSystemDocument::Emitter m_emitter;

    ParticleSystemDocument m_document;
};
