//
// Material Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "material.h"
#include "ui_materialwidget.h"
#include <QWidget>

//-------------------------- MaterialWidget ---------------------------------
//---------------------------------------------------------------------------

class MaterialWidget : public QWidget
{
  Q_OBJECT

  public:
    MaterialWidget(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_ShaderList_activated(int index);

    void on_AlbedoMap_itemDropped(QString const &path);
    void on_AlbedoMask_itemDropped(QString const &path);
    void on_TintRedSlider_valueChanged(double value);
    void on_TintRedSpinner_valueChanged(double value);
    void on_TintGreenSlider_valueChanged(double value);
    void on_TintGreenSpinner_valueChanged(double value);
    void on_TintBlueSlider_valueChanged(double value);
    void on_TintBlueSpinner_valueChanged(double value);    
    void on_TintAlphaSlider_valueChanged(double value);
    void on_TintAlphaSpinner_valueChanged(double value);
    void on_EmissiveSlider_valueChanged(double value);
    void on_EmissiveSpinner_valueChanged(double value);
    void on_AlbedoOutput_clicked();

    void on_MetalnessMap_itemDropped(QString const &path);
    void on_MetalnessSlider_valueChanged(double value);
    void on_MetalnessSpinner_valueChanged(double value);
    void on_MetalnessOutput1_clicked();
    void on_MetalnessOutput2_clicked();
    void on_MetalnessOutput3_clicked();
    void on_MetalnessOutput4_clicked();
    void on_MetalnessOutput5_clicked();
    void on_MetalnessOutput6_clicked();
    void on_MetalnessOutput7_clicked();
    void on_MetalnessOutput8_clicked();

    void on_RoughnessMap_itemDropped(QString const &path);
    void on_RoughnessSlider_valueChanged(double value);
    void on_RoughnessSpinner_valueChanged(double value);
    void on_RoughnessOutput1_clicked();
    void on_RoughnessOutput2_clicked();
    void on_RoughnessOutput3_clicked();
    void on_RoughnessOutput4_clicked();
    void on_RoughnessOutput5_clicked();
    void on_RoughnessOutput6_clicked();
    void on_RoughnessOutput7_clicked();
    void on_RoughnessOutput8_clicked();

    void on_ReflectivityMap_itemDropped(QString const &path);
    void on_ReflectivitySlider_valueChanged(double value);
    void on_ReflectivitySpinner_valueChanged(double value);
    void on_ReflectivityOutput1_clicked();
    void on_ReflectivityOutput2_clicked();
    void on_ReflectivityOutput3_clicked();
    void on_ReflectivityOutput4_clicked();
    void on_ReflectivityOutput5_clicked();
    void on_ReflectivityOutput6_clicked();
    void on_ReflectivityOutput7_clicked();
    void on_ReflectivityOutput8_clicked();

    void on_NormalMap_itemDropped(QString const &path);
    void on_NormalScaleSlider_valueChanged(double value);
    void on_NormalScaleSpinner_valueChanged(double value);
    void on_NormalOutput1_clicked();
    void on_NormalOutput2_clicked();
    void on_NormalOutput3_clicked();

    void on_ResetAlbedo_clicked();
    void on_ResetMetalness_clicked();
    void on_ResetRoughness_clicked();
    void on_ResetReflectivity_clicked();
    void on_ResetNormal_clicked();

  private:

    Ui::MaterialWidget ui;

    MaterialDocument m_document;
};
