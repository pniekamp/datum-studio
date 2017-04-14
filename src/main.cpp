//
// Datum Studio
//

//
// Copyright (c) 2016 Peter Niekamp
//

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QPluginLoader>
#include <leap/pathstring.h>
#include "datumstudio.h"
#include "platform.h"
#include "core.h"

#include <QtDebug>

using namespace std;

class QcApplication : public QApplication
{
  public:
    QcApplication(int &argc, char **argv)
      : QApplication(argc, argv)
    {
    }

    bool notify(QObject *receiver, QEvent *event)
    {
      try
      {
        return QApplication::notify(receiver, event);
      }
      catch(exception &e)
      {
        qDebug() << "Exception:" << e.what();
      }
      catch(...)
      {
        qDebug() << "Exception: Unknown";
      }

      return false;
    }
};

void usage()
{
  QString msg;

  msg = "Usage: datumstudio [options] project\n\n";
  msg += "    -fs[=n]\t\tSet fullscreen\n";
  msg += "    -ss=geometry\tSet geometry WIDTHxHEIGHT+XOFF+YOFF\n";

  QMessageBox::information(NULL, "Usage", msg);
}

QStringList select_plugins(QDir pluginpath)
{
  QStringList plugins;
  QVector<int> ordering;

  for(auto &plugin : pluginpath.entryList(QDir::Files))
  {
    QPluginLoader loader(pluginpath.absoluteFilePath(plugin));

    auto order = loader.metaData().value("MetaData").toObject().value("LoadOrder").toInt(99);

    int i = 0;
    while (i < plugins.size() && ordering[i] < order)
      ++i;

    plugins.insert(i, plugin);
    ordering.insert(i, order);
  }

  return plugins;
}


//|---------------------- main ----------------------------------------------
//|--------------------------------------------------------------------------

int main(int argc, char **argv)
{
  QcApplication app(argc, argv);

  if (app.arguments().contains("-h", Qt::CaseInsensitive))
  {
    usage();
    exit(1);
  }

  QApplication::setStyle("fusion");

  app.setOrganizationName("pniekamp");
  app.setOrganizationDomain("au");
  app.setApplicationName("datumstudio");

  Studio::Core::instance()->add_object(new ActionManager);

  try
  {
    string project = "";

    QStringList args = app.arguments();

    for(int i = 0, j = 0; i < args.size(); ++i)
    {
      if (args[i].startsWith("-"))
        continue;

      if (j == 1)
        project = args[i].toStdString();

      ++j;
    }

    DatumStudio datumstudio;

    for(int i = 0; i < args.size(); ++i)
    {
      if (args[i].startsWith("-fs"))
        datumstudio.set_screen_geometry(args[i].mid(1).toStdString());

      if (args[i].startsWith("-ss="))
        datumstudio.set_screen_geometry(args[i].split('=')[1].toStdString());
    }

    datumstudio.show();

    initialise_platform(datumstudio.windowHandle());

    QDir plugins(leap::pathstring("plugins/datumstudio").c_str());

    for(auto &plugin :select_plugins(plugins))
    {
      QPluginLoader loader(plugins.absoluteFilePath(plugin));

      Studio::Plugin *instance = qobject_cast<Studio::Plugin*>(loader.instance());

      if (instance && strcmp(instance->build(), Studio::ApiBuild) == 0)
      {
        QString errormsg;

        if (instance->initialise(args, &errormsg))
        {
          qInfo().noquote() << "Loaded Plugin (" + plugin + ")";

          Studio::Core::instance()->add_object(instance);
        }
        else
          qWarning().noquote() << "Error Initialising Plugin (" + plugin + ") : " + errormsg;
      }
      else
        qWarning().noquote() << "Invalid Plugin (" + plugin + ") : " + loader.errorString();
    }

    app.exec();

    for(auto &plugin : Studio::Core::instance()->find_objects<Studio::Plugin>())
    {
      plugin->shutdown();

      delete plugin;
    }
  }
  catch(exception &e)
  {
    QMessageBox::critical(NULL, "Error", QString("Critical Error: %1").arg(e.what()));
  }

  return 0;
}
