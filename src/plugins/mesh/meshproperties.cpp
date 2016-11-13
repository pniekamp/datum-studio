//
// Mesh Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshproperties.h"
#include "contentapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MeshProperties ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshProperties::Constructor ///////////////////////
MeshProperties::MeshProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);

  ui.ImportSrc->set_browsetype(QcFileLineEdit::BrowseType::OpenFile);

  ui.ImportScale->setValidator(new QDoubleValidator(0.001, 1000.0, 3, this));

  connect(ui.ImportScaleReset, &QToolButton::clicked, this, [=]() { ui.ImportScale->setText(""); });
}


///////////////////////// MeshProperties::edit //////////////////////////////
void MeshProperties::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &MeshDocument::document_changed, this, &MeshProperties::refresh);

  refresh();
}


///////////////////////// MeshProperties::refresh ///////////////////////////
void MeshProperties::refresh()
{
  int meshes = 0;
  int materials = 0;
  int vertices = 0;
  int triangles = 0;

  m_document->lock();

  PackModelHeader modl;

  if (read_asset_header(m_document, 1, &modl))
  {
    vector<char> payload(pack_payload_size(modl));

    read_asset_payload(m_document, modl.dataoffset, payload.data(), payload.size());

    meshes += modl.meshcount;
    materials += modl.materialcount;

    auto meshtable = PackModelPayload::meshtable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);

    for(size_t i = 0; i < modl.meshcount; ++i)
    {
      PackMeshHeader mesh;

      if (read_asset_header(m_document, 1 + meshtable[i].mesh, &mesh))
      {
        vertices += mesh.vertexcount;
        triangles += mesh.indexcount / 3;
      }
    }
  }

  ui.Meshes->setText(QString::number(meshes));
  ui.Materials->setText(QString::number(materials));
  ui.Vertices->setText(QLocale(QLocale::English).toString(vertices));
  ui.Triangles->setText(QLocale(QLocale::English).toString(triangles));

  ui.ImportSrc->setText(m_document->metadata("src").toString());
  ui.ImportScale->setText(m_document->metadata("importscale").toString());

  m_document->unlock();
}


///////////////////////// MeshProperties::Reimport //////////////////////////
void MeshProperties::on_Reimport_clicked()
{
  if (ui.ImportSrc->text() == "")
    return;

  m_document->lock_exclusive();

  m_document->set_metadata("src", ui.ImportSrc->text());
  m_document->set_metadata("importscale", (ui.ImportScale->text() != "") ? ui.ImportScale->text().toDouble() : QVariant());

  m_document->save();

  m_document->unlock_exclusive(false);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  contentmanager->reimport(documentmanager->path(m_document));
}
