//
// Ocean Material Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- OceanMaterialDocument --------------------------
//---------------------------------------------------------------------------

class OceanMaterialDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path, lml::Color4 const &color, float roughness);

    static void hash(Studio::Document *document, size_t *key);

    static void build_hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    OceanMaterialDocument();
    OceanMaterialDocument(QString const &path);
    OceanMaterialDocument(Studio::Document *document);
    OceanMaterialDocument(OceanMaterialDocument const &document);
    OceanMaterialDocument operator =(OceanMaterialDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    enum class Image
    {
      SurfaceMap,
      NormalMap,
    };

    lml::Color4 color() const { return lml::Color4(m_definition["color.r"].toDouble(), m_definition["color.g"].toDouble(), m_definition["color.b"].toDouble(), m_definition["color.a"].toDouble(1)); }

    lml::Color3 shallowcolor() const { return lml::Color3(m_definition["shallowcolor.r"].toDouble(1), m_definition["shallowcolor.g"].toDouble(1), m_definition["shallowcolor.b"].toDouble(1)); }
    lml::Color3 deepcolor() const { return lml::Color3(m_definition["deepcolor.r"].toDouble(1), m_definition["deepcolor.g"].toDouble(1), m_definition["deepcolor.b"].toDouble(1)); }
    lml::Color3 fresnelcolor() const { return lml::Color3(m_definition["fresnelcolor.r"].toDouble(1), m_definition["fresnelcolor.g"].toDouble(1), m_definition["fresnelcolor.b"].toDouble(1)); }

    float depthscale() const { return m_definition["depthscale"].toDouble(1); }

    float metalness() const { return m_definition["metalness"].toDouble(0); }
    float roughness() const { return m_definition["roughness"].toDouble(0.32f); }
    float reflectivity() const { return m_definition["reflectivity"].toDouble(0.02f); }
    float emissive() const { return m_definition["emissive"].toDouble(0); }

    Studio::Document *image(int image) const { return m_images[image]; }
    Studio::Document *image(Image image) const { return m_images[static_cast<int>(image)]; }

  public:

    void set_color(lml::Color4 const &color);

    void set_shallowcolor(lml::Color3 const &color);
    void set_deepcolor(lml::Color3 const &color);
    void set_fresnelcolor(lml::Color3 const &color);

    void set_depthscale(float depthscale);

    void set_metalness(float metalness);
    void set_roughness(float roughness);
    void set_reflectivity(float reflectivity);
    void set_emissive(float emissive);

    void set_image(Image image, QString const &path);

  signals:

    void document_changed();
    void dependant_changed();

  private:

    void attach(Studio::Document *document);

    void touch(Studio::Document *document, QString const &path);

    void refresh();

    void update();

    QJsonObject m_definition;

    std::vector<unique_document> m_images;

    unique_document m_document;
};
