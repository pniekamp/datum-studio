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
  m_mesh = -1;
  m_slot = -1;

  ui.setupUi(this);

  ui.Material->set_droptype("Material");
}


///////////////////////// MaterialWidget::edit //////////////////////////////
void MaterialWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ModelDocument::document_changed, this, &MaterialWidget::refresh);
  connect(&m_document, &ModelDocument::dependant_changed, this, &MaterialWidget::refresh);

  refresh();
}


///////////////////////// MaterialWidget::set_mesh //////////////////////////
void MaterialWidget::set_mesh(int index)
{
  if (m_mesh != index)
  {
    m_mesh = index;

    refresh();
  }
}


///////////////////////// MaterialWidget::set_slot //////////////////////////
void MaterialWidget::set_slot(int index)
{
  if (m_slot != index)
  {
    m_slot = index;

    refresh();
  }
}


///////////////////////// MaterialWidget::refresh ///////////////////////////
void MaterialWidget::refresh()
{
  ui.Name->setText("");
  ui.Material->setPixmap(nullptr);

  if (0 <= m_mesh && m_mesh < m_document.meshes())
  {
    auto &mesh = m_document.mesh(m_mesh);

    if (0 <= m_slot && m_slot < (int)mesh.materials.size())
    {
      auto &material = mesh.materials[m_slot];

      ui.Name->setText(material.name);

      ui.Material->setPixmap(material.document);

      ui.TintRedSlider->updateValue(material.tint.r);
      ui.TintRedSpinner->updateValue(material.tint.r);
      ui.TintGreenSlider->updateValue(material.tint.g);
      ui.TintGreenSpinner->updateValue(material.tint.g);
      ui.TintBlueSlider->updateValue(material.tint.b);
      ui.TintBlueSpinner->updateValue(material.tint.b);
    }
  }

  ui.Material->setEnabled(m_mesh != -1 && m_slot != -1);
  ui.TintGroup->setEnabled(m_mesh != -1 && m_slot != -1);

  update();
}


///////////////////////// MaterialWidget::Material //////////////////////////
void MaterialWidget::on_Material_itemDropped(QString const &path)
{
  m_document.set_mesh_material(m_mesh, m_slot, path);
}


///////////////////////// MaterialWidget::TintRedSlider /////////////////////
void MaterialWidget::on_TintRedSlider_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(value, ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value()));
}


///////////////////////// MaterialWidget::TintRedSpinner ////////////////////
void MaterialWidget::on_TintRedSpinner_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(value, ui.TintGreenSpinner->value(), ui.TintBlueSpinner->value()));
}


///////////////////////// MaterialWidget::TintGreenSlider ///////////////////
void MaterialWidget::on_TintGreenSlider_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(ui.TintRedSpinner->value(), value, ui.TintBlueSpinner->value()));
}


///////////////////////// MaterialWidget::TintGreenSpinner //////////////////
void MaterialWidget::on_TintGreenSpinner_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(ui.TintRedSpinner->value(), value, ui.TintBlueSpinner->value()));
}


///////////////////////// MaterialWidget::TintBlueSlider ////////////////////
void MaterialWidget::on_TintBlueSlider_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), value));
}


///////////////////////// MaterialWidget::TintBlueSpinner ///////////////////
void MaterialWidget::on_TintBlueSpinner_valueChanged(double value)
{
  m_document.set_mesh_material_tint(m_mesh, m_slot, Color3(ui.TintRedSpinner->value(), ui.TintGreenSpinner->value(), value));
}


///////////////////////// MaterialWidget::Reset /////////////////////////////
void MaterialWidget::on_Reset_clicked()
{
  m_document.reset_mesh_material(m_mesh, m_slot);
}
