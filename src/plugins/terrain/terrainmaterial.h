//
// Terrain Material Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- TerrainMaterialDocument ------------------------
//---------------------------------------------------------------------------

class TerrainMaterialDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path, lml::Color4 const &color);

    static void hash(Studio::Document *document, size_t *key);

    static void build_hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    TerrainMaterialDocument();
    TerrainMaterialDocument(QString const &path);
    TerrainMaterialDocument(Studio::Document *document);
    TerrainMaterialDocument(TerrainMaterialDocument const &document);
    TerrainMaterialDocument operator =(TerrainMaterialDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    lml::Color4 color() const { return lml::Color4(m_definition["color.r"].toDouble(), m_definition["color.g"].toDouble(), m_definition["color.b"].toDouble(), m_definition["color.a"].toDouble(1)); }

    float metalness() const { return m_definition["metalness"].toDouble(0); }
    float roughness() const { return m_definition["roughness"].toDouble(1); }
    float reflectivity() const { return m_definition["reflectivity"].toDouble(0.5); }
    float emissive() const { return m_definition["emissive"].toDouble(0); }

    int layers() const { return m_materials.size(); }

    Studio::Document *layer(int index) const { return m_materials[index]; }

  public:

    void set_color(lml::Color4 const &color);

    void set_metalness(float metalness);
    void set_roughness(float roughness);
    void set_reflectivity(float reflectivity);
    void set_emissive(float emissive);

    void add_layer(int position, QString const &path);

    void move_layer(int index, int position);

    void erase_layer(int index);

  signals:

    void document_changed();
    void dependant_changed();

  private:

    void attach(Studio::Document *document);

    void touch(Studio::Document *document, QString const &path);

    void refresh();

    void update();

    QJsonObject m_definition;

    std::vector<unique_document> m_materials;

    unique_document m_document;
};
