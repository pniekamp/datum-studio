//
// Document Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "documentplugin.h"
#include "documentmanager.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- DocumentPlugin ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// DocumentPlugin::Constructor ///////////////////////
DocumentPlugin::DocumentPlugin()
{
}


///////////////////////// DocumentPlugin::Destructor ////////////////////////
DocumentPlugin::~DocumentPlugin()
{
  shutdown();
}


///////////////////////// DocumentPlugin::initialise ////////////////////////
bool DocumentPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  Studio::Core::instance()->add_object(new DocumentManager);

  return true;
}


///////////////////////// DocumentPlugin::shutdown //////////////////////////
void DocumentPlugin::shutdown()
{
}
