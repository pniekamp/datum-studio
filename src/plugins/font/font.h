//
// Font Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <string>
#include <QFont>

//-------------------------- FontDocument -----------------------------------
//---------------------------------------------------------------------------

class FontDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path, std::string const &name, int size, int weight);

    static void hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

  public:
    FontDocument() = default;
    FontDocument(QString const &path);
    FontDocument(Studio::Document *document);
    FontDocument(FontDocument const &document);
    FontDocument operator =(FontDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    QString name() const { return m_definition["name"].toString(); }

    int size() const { return m_definition["size"].toInt(); }
    int weight() const { return m_definition["weight"].toInt(); }
    bool italic() const { return m_definition["italic"].toBool(); }

    QFont font() const { return QFont(name(), size(), weight(), italic()); }

    int atlaswidth() const { return m_definition["atlaswidth"].toInt(); }
    int atlasheight() const { return m_definition["atlasheight"].toInt(); }

  public:

    void set_name(QString const &name);

    void set_size(int size);
    void set_weight(int weight);
    void set_italic(bool italic);

    void set_font(QFont const &font);

    void set_atlaswidth(int atlaswidth);
    void set_atlasheight(int atlasheight);

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    void refresh();

    void update();

    QJsonObject m_definition;

    unique_document m_document;
};
