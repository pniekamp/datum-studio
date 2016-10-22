//
// Build Status
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "buildstatus.h"
#include "buildmanager.h"
#include <QHBoxLayout>

#include <QDebug>

using namespace std;

//|---------------------- BuildStatus ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// BuildStatus::Constructor //////////////////////////
BuildStatus::BuildStatus()
{
  m_building = 0;

  m_text = new QLabel(this);
  m_loader = new QLabel(this);
  m_loader->setMovie(new QMovie(":/images/loader.gif"));

  setLayout(new QHBoxLayout);
  layout()->setContentsMargins(4, 0, 4, 0);
  layout()->addWidget(m_text);
  layout()->addWidget(m_loader);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  connect(buildmanager, &Studio::BuildManager::build_started, this, &BuildStatus::on_build_started);
  connect(buildmanager, &Studio::BuildManager::build_completed, this, &BuildStatus::on_build_completed);
}



///////////////////////// BuildStatus::build_started ////////////////////////
void BuildStatus::on_build_started(Studio::Document *document)
{
  ++m_building;

  m_text->setText(QString("Building (%1)").arg(m_building));

  m_loader->movie()->start();
  m_loader->movie()->setSpeed(75);

  m_text->show();
  m_loader->show();
}


///////////////////////// BuildStatus::build_completed //////////////////////
void BuildStatus::on_build_completed(Studio::Document *document)
{
  --m_building;

  m_text->setText(QString("Building (%1)").arg(m_building));

  if (m_building == 0)
  {
    m_loader->movie()->stop();

    m_text->hide();
    m_loader->hide();
  }
}
