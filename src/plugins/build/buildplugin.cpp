//
// Build Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "buildplugin.h"
#include "buildmanager.h"
#include "buildstatus.h"
#include <QWidgetAction>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- BuildPlugin ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// BuildPlugin::Constructor //////////////////////////
BuildPlugin::BuildPlugin()
{
}


///////////////////////// BuildPlugin::Destructor ///////////////////////////
BuildPlugin::~BuildPlugin()
{
  shutdown();
}


///////////////////////// BuildPlugin::initialise ///////////////////////////
bool BuildPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  Studio::Core::instance()->add_object(new BuildManager);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto report = new QWidgetAction(this);
  report->setDefaultWidget(new BuildStatus);
  actionmanager->container("Studio.Main.StatusReport")->add_back(report);

  return true;
}


///////////////////////// BuildPlugin::shutdown /////////////////////////////
void BuildPlugin::shutdown()
{
}
