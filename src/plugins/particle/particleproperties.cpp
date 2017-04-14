//
// Particle Properties
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "particleproperties.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ParticleProperties --------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ParticleProperties::Constructor ///////////////////
ParticleProperties::ParticleProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);

  ui.SpriteSheet->set_droptype("SpriteSheet");

  ui.Emitter->setVisible(false);

  connect(ui.EmitterList, &EmitterListWidget::selection_changed, this, &ParticleProperties::set_selection);
}


///////////////////////// ParticleProperties::edit //////////////////////////
void ParticleProperties::edit(Studio::Document *document)
{
  m_document = document;

  ui.EmitterList->edit(document);
  ui.Emitter->edit(document);

  connect(&m_document, &ParticleSystemDocument::document_changed, this, &ParticleProperties::refresh);
  connect(&m_document, &ParticleSystemDocument::dependant_changed, this, &ParticleProperties::refresh);

  refresh();
}


///////////////////////// ParticleProperties::set_selection /////////////////
void ParticleProperties::set_selection(int index)
{
  ui.EmitterList->set_selection(index);
  ui.Emitter->set_emitter(index);

  ui.Emitter->setVisible(index != -1);

  emit selection_changed(index);
}


///////////////////////// ParticleProperties::refresh ///////////////////////
void ParticleProperties::refresh()
{ 
  ui.MaxParticles->updateValue(m_document.maxparticles());

  ui.SpriteSheet->setPixmap(m_document.spritesheet());

  ui.Bound1->updateValue(m_document.bound().max.x);
  ui.Bound2->updateValue(m_document.bound().max.y);
  ui.Bound3->updateValue(m_document.bound().max.z);
  ui.Bound4->updateValue(m_document.bound().min.x);
  ui.Bound5->updateValue(m_document.bound().min.y);
  ui.Bound6->updateValue(m_document.bound().min.z);
}


///////////////////////// ParticleProperties::MaxParticles //////////////////
void ParticleProperties::on_MaxParticles_valueChanged(int value)
{
  m_document.set_maxparticles(value);
}


///////////////////////// ParticleProperties::SpriteSheet ///////////////////
void ParticleProperties::on_SpriteSheet_itemDropped(QString const &path)
{
  m_document.set_spritesheet(path);
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound1_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(m_document.bound().min.x, m_document.bound().min.y, m_document.bound().min.z), Vec3(value, m_document.bound().max.y, m_document.bound().max.z)));
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound2_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(m_document.bound().min.x, m_document.bound().min.y, m_document.bound().min.z), Vec3(m_document.bound().max.x, value, m_document.bound().max.z)));
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound3_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(m_document.bound().min.x, m_document.bound().min.y, m_document.bound().min.z), Vec3(m_document.bound().max.x, m_document.bound().max.y, value)));
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound4_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(value, m_document.bound().min.y, m_document.bound().min.z), Vec3(m_document.bound().max.x, m_document.bound().max.y, m_document.bound().max.z)));
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound5_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(m_document.bound().min.x, value, m_document.bound().min.z), Vec3(m_document.bound().max.x, m_document.bound().max.y, m_document.bound().max.z)));
}


///////////////////////// ParticleProperties::Bound /////////////////////////
void ParticleProperties::on_Bound6_valueChanged(double value)
{
  m_document.set_bound(Bound3(Vec3(m_document.bound().min.x, m_document.bound().min.y, value), Vec3(m_document.bound().max.x, m_document.bound().max.y, m_document.bound().max.z)));
}
