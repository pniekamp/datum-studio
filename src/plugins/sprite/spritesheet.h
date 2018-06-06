//
// SpriteSheet Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "packapi.h"
#include <string>

//-------------------------- SpriteSheetDocument ----------------------------
//---------------------------------------------------------------------------

class SpriteSheetDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path);

    static void hash(Studio::Document *document, size_t *key);

    static void build_hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    SpriteSheetDocument();
    SpriteSheetDocument(QString const &path);
    SpriteSheetDocument(Studio::Document *document);
    SpriteSheetDocument(SpriteSheetDocument const &document);
    SpriteSheetDocument operator =(SpriteSheetDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    int layers() const { return m_images.size(); }

    Studio::Document *layer(int index) const { return m_images[index]; }

  public:

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

    std::vector<unique_document> m_images;

    unique_document m_document;
};
