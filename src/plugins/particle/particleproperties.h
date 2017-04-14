//
// Particle Properties
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "particlesystem.h"
#include "ui_particleproperties.h"
#include <QDockWidget>

//-------------------------- ParticleProperties -----------------------------
//---------------------------------------------------------------------------

class ParticleProperties : public QDockWidget
{
  Q_OBJECT

  public:
    ParticleProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

    void set_selection(int index);

  signals:

    void selection_changed(int index);

  protected slots:

    void refresh();

    void on_MaxParticles_valueChanged(int value);
    void on_SpriteSheet_itemDropped(QString const &path);

    void on_Bound1_valueChanged(double value);
    void on_Bound2_valueChanged(double value);
    void on_Bound3_valueChanged(double value);
    void on_Bound4_valueChanged(double value);
    void on_Bound5_valueChanged(double value);
    void on_Bound6_valueChanged(double value);

  private:

    Ui::Properties ui;

    ParticleSystemDocument m_document;
};
