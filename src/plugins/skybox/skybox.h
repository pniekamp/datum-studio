//
// Skybox Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "packapi.h"
#include <string>

//-------------------------- SkyboxDocument ---------------------------------
//---------------------------------------------------------------------------

class SkyboxDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path);

    static void hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    SkyboxDocument();
    SkyboxDocument(QString const &path);
    SkyboxDocument(Studio::Document *document);
    SkyboxDocument(SkyboxDocument const &document);
    SkyboxDocument operator =(SkyboxDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    enum class Type
    {
      FaceImages,
      SphericalMap
    };

    enum class Image
    {
      Front,
      Left,
      Right,
      Back,
      Top,
      Bottom,
      EnvMap
    };

    Type type() const { return static_cast<Type>(m_definition["type"].toInt()); }

    int width() const { return m_definition["width"].toInt(); }
    int height() const { return m_definition["height"].toInt(); }

    Studio::Document const *image(int image) const { return m_images[image]; }
    Studio::Document const *image(Image image) const { return m_images[static_cast<int>(image)]; }

  public:

    void set_type(Type type);

    void set_width(int width);
    void set_height(int height);

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
