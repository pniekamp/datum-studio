//
// Asset File
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "assetpacker.h"
#include "documentapi.h"
#include <QIcon>
#include <QJsonObject>
#include <fstream>

//
// Asset File Functions
//

QString encode_icon(QIcon const &icon);
QIcon decode_icon(QString const &str);

uint64_t read_asset_header(std::istream &fin, uint32_t id, PackTextHeader *text);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackFontHeader *font);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackImageHeader *imag);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackMeshHeader *mesh);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackMaterialHeader *matl);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackAnimationHeader *anim);
uint64_t read_asset_header(std::istream &fin, uint32_t id, PackModelHeader *modl);
uint64_t read_asset_payload(std::istream &fin, uint64_t offset, void *data, uint32_t size);

QImage read_asset_image(std::istream &fin, uint32_t id, int layer);
QByteArray read_asset_text(std::istream &fin, uint32_t id);
QJsonObject read_asset_json(std::istream &fin, uint32_t id);

void write_asset_header(std::ostream &fout, QJsonObject const &metadata);
void write_asset_text(std::ostream &fout, uint32_t id, uint32_t length, void const *data);
void write_asset_image(std::ostream &fout, uint32_t id, std::vector<QImage> const &images, uint32_t format);
void write_asset_json(std::ostream &fout, uint32_t id, QJsonObject const &json);
void write_asset_footer(std::ostream &fout);

//
// Asset Document Functions
//

uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackTextHeader *text);
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackImageHeader *imag);
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackMeshHeader *mesh);
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackMaterialHeader *matl);
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackAnimationHeader *anim);
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackModelHeader *modl);
uint64_t read_asset_payload(Studio::Document *document, uint64_t offset, void *data, uint32_t size);

uint64_t write_text_asset(Studio::Document *document, uint64_t position, uint32_t id, uint32_t length, void const *data);
uint64_t write_footer(Studio::Document *document, uint64_t position);

//
// Misc Functions
//

double buildtime();

QString relpath(Studio::Document *document, QString const &file);
QString fullpath(Studio::Document *document, QString const &file);
