//
// Model Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- ModelDocument ----------------------------
//---------------------------------------------------------------------------

class ModelDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path);

    static void hash(Studio::Document *document, size_t *key);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    ModelDocument();
    ModelDocument(QString const &path);
    ModelDocument(Studio::Document *document);
    ModelDocument(ModelDocument const &document);
    ModelDocument operator =(ModelDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    struct Mesh
    {
      QString name;
      unique_document document;

      struct Material
      {
        QString name;
        lml::Color3 tint;
        unique_document document;
      };

      std::vector<Material> materials;

      lml::Transform transform;
    };

    int meshes() const { return m_meshes.size(); }

    Mesh const &mesh(int index) const { return m_meshes[index]; }

  public:

    struct Instance
    {
      Studio::Document *mesh;
      size_t index;

      Studio::Document *material;

      lml::Color3 tint;

      lml::Transform transform;
    };

    std::vector<Instance> instances() const;

  public:

    void add_mesh(int position, QString const &path);

    void move_mesh(int index, int position);

    void erase_mesh(int index);

    void set_mesh_name(int index, QString const &name);

    void set_mesh_transform(int index, lml::Transform const &transform);

    void set_mesh_material(int index, int slot, QString const &path);
    void set_mesh_material_tint(int index, int slot, lml::Color3 const &tint);

    void reset_mesh_material(int index, int slot);

  signals:

    void document_changed();
    void dependant_changed();

  private:

    void attach(Studio::Document *document);

    void touch(Studio::Document *document, QString const &path);

    void refresh();

    void update();

    QJsonObject m_definition;

    std::vector<Mesh> m_meshes;

    unique_document m_document;
};
