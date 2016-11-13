//
// Pack
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "packapi.h"
#include "packmodel.h"
#include "ui_build.h"

//-------------------------- PackManager ------------------------------------
//---------------------------------------------------------------------------

class PackManager : public Studio::PackManager
{
  Q_OBJECT

  public:
    PackManager();

    void register_packer(QString const &type, QObject *packer);

    void build(PackModel const *model, QString const &filename, Ui::Build *dlg);

  private:

    QMap<QString, QObject*> m_packers;
};


