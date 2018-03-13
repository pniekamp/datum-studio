//
// Material Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "oceanmaterial.h"
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

    void on_ShallowColor_clicked();
    void on_DeepColor_clicked();
    void on_FresnelColor_clicked();

    void on_DepthScaleSlider_valueChanged(double value);
    void on_DepthScaleSpinner_valueChanged(double value);

    void on_RoughnessSlider_valueChanged(double value);
    void on_RoughnessSpinner_valueChanged(double value);

    void on_ReflectivitySlider_valueChanged(double value);
    void on_ReflectivitySpinner_valueChanged(double value);

    void on_EmissiveSlider_valueChanged(double value);
    void on_EmissiveSpinner_valueChanged(double value);

    void on_SurfaceMap_itemDropped(QString const &path);

    void on_NormalMap_itemDropped(QString const &path);

    void on_ResetAlbedo_clicked();
    void on_ResetSurface_clicked();
    void on_ResetNormal_clicked();

  private:

    Ui::MaterialWidget ui;

    OceanMaterialDocument m_document;
};
