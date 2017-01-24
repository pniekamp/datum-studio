//
// Model Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "modelproperties.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

const double rad2deg = 180.0 / pi<double>();
const double deg2rad = pi<double>() / 180.0;

//|---------------------- ModelProperties -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelProperties::Constructor //////////////////////
ModelProperties::ModelProperties(QWidget *parent)
  : QDockWidget(parent)
{
  m_mesh = -1;

  ui.setupUi(this);

  connect(ui.MeshList, &MeshListWidget::selection_changed, this, &ModelProperties::set_selection);
  connect(ui.SlotList, &SlotListWidget::selection_changed, ui.Material, &MaterialWidget::set_slot);
}


///////////////////////// ModelProperties::edit /////////////////////////////
void ModelProperties::edit(ModelDocument *document)
{
  m_document = document;

  ui.MeshList->edit(document);
  ui.SlotList->edit(document);
  ui.Material->edit(document);

  connect(m_document, &ModelDocument::document_changed, this, &ModelProperties::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &ModelProperties::refresh);

  refresh();
}


///////////////////////// ModelProperties::set_selection ////////////////////
void ModelProperties::set_selection(int index)
{
  m_mesh = index;

  ui.MeshList->set_selection(m_mesh);
  ui.SlotList->set_mesh(m_mesh);
  ui.Material->set_mesh(m_mesh);

  emit selection_changed(m_mesh);

  refresh();
}


///////////////////////// ModelProperties::refresh /////////////////////////
void ModelProperties::refresh()
{
  if (0 <= m_mesh && m_mesh < m_document->meshes())
  {
    auto &mesh = m_document->mesh(m_mesh);

    ui.dx->updateValue(mesh.transform.translation().x);
    ui.dy->updateValue(mesh.transform.translation().y);
    ui.dz->updateValue(mesh.transform.translation().z);
    ui.ax->updateValue(mesh.transform.rotation().ax() * rad2deg);
    ui.ay->updateValue(mesh.transform.rotation().ay() * rad2deg);
    ui.az->updateValue(mesh.transform.rotation().az() * rad2deg);
  }

  ui.dx->setEnabled(m_mesh != -1);
  ui.dy->setEnabled(m_mesh != -1);
  ui.dz->setEnabled(m_mesh != -1);
  ui.ax->setEnabled(m_mesh != -1);
  ui.ay->setEnabled(m_mesh != -1);
  ui.az->setEnabled(m_mesh != -1);
}


///////////////////////// ModelProperties::dx ///////////////////////////////
void ModelProperties::on_dx_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(value, mesh.transform.translation().y, mesh.transform.translation().z) * Transform::rotation(mesh.transform.rotation()));
}


///////////////////////// ModelProperties::dy ///////////////////////////////
void ModelProperties::on_dy_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(mesh.transform.translation().x, value, mesh.transform.translation().z) * Transform::rotation(mesh.transform.rotation()));
}


///////////////////////// ModelProperties::dz ///////////////////////////////
void ModelProperties::on_dz_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(mesh.transform.translation().x, mesh.transform.translation().y, value) * Transform::rotation(mesh.transform.rotation()));
}


///////////////////////// ModelProperties::ax ///////////////////////////////
void ModelProperties::on_ax_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(mesh.transform.translation()) * Transform::rotation(Vec3(0, 0, 1), mesh.transform.rotation().az()) * Transform::rotation(Vec3(0, 1, 0), mesh.transform.rotation().ay()) * Transform::rotation(Vec3(1, 0, 0), value * deg2rad));
}


///////////////////////// ModelProperties::ay ///////////////////////////////
void ModelProperties::on_ay_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(mesh.transform.translation()) * Transform::rotation(Vec3(0, 0, 1), mesh.transform.rotation().az()) * Transform::rotation(Vec3(0, 1, 0), value * deg2rad) * Transform::rotation(Vec3(1, 0, 0), mesh.transform.rotation().ax()));
}


///////////////////////// ModelProperties::az ///////////////////////////////
void ModelProperties::on_az_valueChanged(double value)
{
  auto &mesh = m_document->mesh(m_mesh);

  m_document->set_mesh_transform(m_mesh, Transform::translation(mesh.transform.translation()) * Transform::rotation(Vec3(0, 0, 1), value * deg2rad) * Transform::rotation(Vec3(0, 1, 0), mesh.transform.rotation().ay()) * Transform::rotation(Vec3(1, 0, 0), mesh.transform.rotation().ax()));
}
