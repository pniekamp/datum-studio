//
// Material Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- MaterialDocument -------------------------------
//---------------------------------------------------------------------------

class MaterialDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path, lml::Color3 const &color, float metalness, float roughness);

    static void hash(Studio::Document *document, size_t *key);

    static void build_hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    MaterialDocument();
    MaterialDocument(QString const &path);
    MaterialDocument(Studio::Document *document);
    MaterialDocument(MaterialDocument const &document);
    MaterialDocument operator =(MaterialDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    enum class Shader
    {
      Deferred,
      Transparent
    };

    enum class Image
    {
      AlbedoMap,
      AlbedoMask,
      MetalnessMap,
      RoughnessMap,
      ReflectivityMap,
      NormalMap,
    };

    enum class AlbedoOutput
    {
      rgba,
    };

    enum class MetalnessOutput
    {
      r, g, b, a, invr, invg, invb, inva
    };

    enum class RoughnessOutput
    {
      r, g, b, a, invr, invg, invb, inva
    };

    enum class ReflectivityOutput
    {
      r, g, b, a, invr, invg, invb, inva
    };

    enum class NormalOutput
    {
      xyz, xinvyz, bump
    };

    Shader shader() const { return static_cast<Shader>(m_definition["shader"].toInt(0)); }

    lml::Color3 color() const { return lml::Color3(m_definition["color.r"].toDouble(), m_definition["color.g"].toDouble(), m_definition["color.b"].toDouble()); }

    float metalness() const { return m_definition["metalness"].toDouble(0); }
    float roughness() const { return m_definition["roughness"].toDouble(1); }
    float reflectivity() const { return m_definition["reflectivity"].toDouble(0.5); }
    float emissive() const { return m_definition["emissive"].toDouble(0); }

    Studio::Document const *image(int image) const { return m_images[image]; }
    Studio::Document const *image(Image image) const { return m_images[static_cast<int>(image)]; }

    AlbedoOutput albedooutput() const { return static_cast<AlbedoOutput>(m_definition["albedooutput"].toInt(0)); }
    MetalnessOutput metalnessoutput() const { return static_cast<MetalnessOutput>(m_definition["metalnessoutput"].toInt(0)); }
    RoughnessOutput roughnessoutput() const { return static_cast<RoughnessOutput>(m_definition["roughnessoutput"].toInt(3)); }
    ReflectivityOutput reflectivityoutput() const { return static_cast<ReflectivityOutput>(m_definition["reflectivityoutput"].toInt(1)); }
    NormalOutput normaloutput() const { return static_cast<NormalOutput>(m_definition["normaloutput"].toInt(0)); }

  public:

    void set_shader(Shader shader);

    void set_color(lml::Color3 const &color);

    void set_metalness(float metalness);
    void set_roughness(float roughness);
    void set_reflectivity(float reflectivity);
    void set_emissive(float emissive);

    void set_image(Image image, QString const &path);

    void set_albedooutput(AlbedoOutput output);
    void set_metalnessoutput(MetalnessOutput output);
    void set_roughnessoutput(RoughnessOutput output);
    void set_reflectivityoutput(ReflectivityOutput output);
    void set_normaloutput(NormalOutput output);

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
