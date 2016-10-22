//
// Datum Studio
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "datumstudio.h"
#include "dialogfactory.h"
#include "ui_about.h"
#include <leap/pathstring.h>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QScreen>
#include <QFile>

#include <QtDebug>

using namespace std;
using namespace leap;

const char *VersionString = "0.0.1";

//|---------------------- DatumStudio ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// DatumStudio::Constructor //////////////////////////
DatumStudio::DatumStudio()
{
  ui.setupUi(this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  actionmanager->register_container("Studio.Menu", ui.MenuBar);
  actionmanager->register_container("Studio.Menu.File", ui.FileMenu);
  actionmanager->register_container("Studio.Menu.Help", ui.HelpMenu);

  Studio::Core::instance()->add_object(new MainWindow(this));

  actionmanager->register_container("Studio.Meta.Bar", ui.SideMetaBar);
  actionmanager->register_container("Studio.Meta.Box", ui.SideMetaBox);
  actionmanager->register_container("Studio.Main.StatusBar", ui.MainStatusBar);
  actionmanager->register_container("Studio.Main.StatusBox", ui.MainStatusBox);
  actionmanager->register_container("Studio.Main.StatusReport", ui.MainStatusReport);

  Studio::Core::instance()->add_object(new ModeManager(ui.SideMetaBar, ui.SideMetaBox, ui.Container));

  Studio::Core::instance()->add_object(new StatusManager(ui.MainStatusBar, ui.MainStatusBox, ui.MainStatusReport, ui.MainStatusView));

  Studio::Core::instance()->add_object(new ViewFactory);

  QFile theme(pathstring("theme.css").c_str());

  if (theme.open(QIODevice::ReadOnly))
  {
    setStyleSheet(theme.readAll());
  }

  QSettings settings;
  move(settings.value("mainwindow/pos", pos()).toPoint());
  resize(settings.value("mainwindow/size", size()).toSize());
  restoreState(settings.value("mainwindow/state", QByteArray()).toByteArray());
  ui.Splitter->restoreState(settings.value("mainwindow/splitter", QByteArray()).toByteArray());
}


///////////////////////// DatumStudio::Destructor ///////////////////////////
DatumStudio::~DatumStudio()
{
  QSettings settings;

  if (!(windowFlags() & Qt::FramelessWindowHint) && !isMaximized())
  {
    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/size", size());
  }

  settings.setValue("mainwindow/state", saveState());

  settings.setValue("mainwindow/splitter", ui.Splitter->saveState());
}


///////////////////////// DatumStudio::set_screen_geometry //////////////////
// WIDTHxHEIGHT+XOFF+YOFF
void DatumStudio::set_screen_geometry(std::string const &geometry)
{
  QScreen *screen = QGuiApplication::primaryScreen();

  if (geometry.substr(0, 3) == "fs=")
  {
    int screenid = atoi(geometry.substr(3).c_str());

    if (screenid >= 0 && screenid < QGuiApplication::screens().size())
      screen = QGuiApplication::screens()[screenid];
  }

  int x = screen->geometry().left();
  int y = screen->geometry().top();
  int w = screen->geometry().width();
  int h = screen->geometry().height();

  QRegExp rx("(\\d+)x(\\d+)([+-]\\d+)*([+-]\\d+)*");

  if (rx.indexIn(geometry.c_str()) == 0)
  {
    x = rx.cap(3).toInt();
    y = rx.cap(4).toInt();
    w = rx.cap(1).toInt();
    h = rx.cap(2).toInt();

    if (x < 0)
      x += screen->geometry().width();

    if (y < 0)
      y += screen->geometry().height();
  }

//  setWindowFlags(Qt::FramelessWindowHint);
//  setGeometry(x, y, w, h);

  // QTBUG-41883
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setGeometry(x, y, w, h+1);
}


///////////////////////// DatumStudio::closeEvent ///////////////////////////
void DatumStudio::closeEvent(QCloseEvent *event)
{
  auto mainwindow = Studio::Core::instance()->find_object<MainWindow>();

  if (!mainwindow->close())
  {
    event->ignore();
  }
}


///////////////////////// DatumStudio::on_About_triggered ///////////////////
void DatumStudio::on_About_triggered()
{
  DialogFactory<Ui::About> dlg(this);

  dlg.ui.Version->setText(QString("Version %1").arg(VersionString));

  dlg.exec();
}
