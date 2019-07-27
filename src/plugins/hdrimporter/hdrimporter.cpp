//
// Hdr Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "hdrimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include "hdr.h"
#include <QImage>
#include <QFileInfo>
#include <QPainter>
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;

namespace
{
  QIcon generate_icon(QImage const &img)
  {
    QImage icon(48, 48, QImage::Format_ARGB32);

    QPainter painter(&icon);

    QImage scaled = img.scaled(icon.width(), icon.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    painter.fillRect(icon.rect(), Qt::white);
    painter.drawImage((icon.width() - scaled.width())/2, (icon.height() - scaled.height())/2, scaled);

    painter.setPen(Qt::darkGray);
    painter.drawRect(icon.rect().adjusted(0, 0, -1, -1));

    painter.end();

    return QPixmap::fromImage(icon);
  }

  QIcon generate_icon(HDRImage const &hdr)
  {
    QImage img(hdr.width, hdr.height, QImage::Format_ARGB32);

    uint32_t *dst = (uint32_t*)img.bits();

    for(int j = 0; j < img.height(); ++j)
    {
      for(int i = 0; i < img.width(); ++i)
      {
        *dst++ = srgba(clamp(hdr.sample(i, j), 0.0f, 1.0f));
      }
    }

    return generate_icon(img);
  }
}


//|---------------------- HdrImporter ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// HdrImporter::Constructor //////////////////////////
HdrImporter::HdrImporter()
{
}


///////////////////////// HdrImporter::Destructor ///////////////////////////
HdrImporter::~HdrImporter()
{
  shutdown();
}


///////////////////////// HdrImporter::initialise ///////////////////////////
bool HdrImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("HDR", this);

  return true;
}


///////////////////////// HdrImporter::shutdown /////////////////////////////
void HdrImporter::shutdown()
{
}


///////////////////////// HdrImporter::try_import ///////////////////////////
bool HdrImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  if (QFileInfo(src).suffix().toLower() != "hdr")
    return false;

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  QProgressDialog progress("Import HDR", "Abort", 0, 100, mainwindow->handle());

  HDRImage hdr = load_hdr(src.toStdString());

  progress.setValue(10);

  int width = metadata["importwidth"].toInt(hdr.width);
  int height = metadata["importheight"].toInt(hdr.height);

  QImage image(width, height, QImage::Format_ARGB32);

  uint32_t *bits = (uint32_t*)image.bits();

  for(int j = 0; j < image.height(); ++j)
  {
    for(int i = 0; i < image.width(); ++i)
    {
      *bits++ = rgbe(hdr.sample(Vec2((i + 0.5f)/width, (j + 0.5f)/height), Vec2(1.0f / width, 1.0f / height)));
    }
  }

  progress.setValue(50);

  metadata["src"] = src;
  metadata["type"] = "Image";
  metadata["icon"] = encode_icon(generate_icon(hdr));
  metadata["build"] = buildtime();

  progress.setValue(80);

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_image(fout, 1, { image }, PackImageHeader::rgbe);

  write_asset_footer(fout);

  progress.setValue(100);

  fout.close();

  return true;
}
