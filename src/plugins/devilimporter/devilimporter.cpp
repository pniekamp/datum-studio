//
// DevIL Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "devilimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include "IL/il.h"
#include <QImage>
#include <QPainter>
#include <QtPlugin>

#include <QDebug>

using namespace std;

namespace
{
  QIcon generate_icon(QImage const &image)
  {
    QImage icon(48, 48, QImage::Format_ARGB32);

    QPainter painter(&icon);

    QImage scaled = image.scaled(icon.width(), icon.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    painter.fillRect(icon.rect(), Qt::white);
    painter.drawImage((icon.width() - scaled.width())/2, (icon.height() - scaled.height())/2, scaled);

    painter.setPen(Qt::darkGray);
    painter.drawRect(icon.rect().adjusted(0, 0, -1, -1));

    painter.end();

    return QPixmap::fromImage(icon);
  }
}


//|---------------------- DevILImporter -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// DevILImporter::Constructor ////////////////////////
DevILImporter::DevILImporter()
{
}


///////////////////////// DevILImporter::Destructor /////////////////////////
DevILImporter::~DevILImporter()
{
  shutdown();
}


///////////////////////// DevILImporter::initialise /////////////////////////
bool DevILImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Image", this);

  return true;
}


///////////////////////// DevILImporter::shutdown ///////////////////////////
void DevILImporter::shutdown()
{
}


///////////////////////// DevILImporter::try_import /////////////////////////
bool DevILImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  ilInit();
  ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
  ilEnable(IL_ORIGIN_SET);

  if (!ilLoadImage(src.toUtf8()))
    return false;

  QImage image(ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), QImage::Format_ARGB32);

  ilCopyPixels(0, 0, 0, image.width(), image.height(), 1, IL_BGRA, IL_UNSIGNED_BYTE, image.bits());

  int width = metadata["importwidth"].toInt(image.width());
  int height = metadata["importheight"].toInt(image.height());

  if (width != image.width() || height != image.height())
  {
    image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }

  if (image.format() != QImage::Format_ARGB32)
  {
    image = image.convertToFormat(QImage::Format_ARGB32);
  }

  metadata["src"] = src;
  metadata["type"] = "Image";
  metadata["icon"] = encode_icon(generate_icon(image));
  metadata["build"] = buildtime();

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_image(fout, 1, { image }, PackImageHeader::rgba);

  write_asset_footer(fout);

  fout.close();

  return true;
}
