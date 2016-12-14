//
// Pack API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "documentapi.h"
#include <fstream>

#if defined(PACKPLUGIN)
# define PACKPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define PACKPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{
  struct PackerState
  {
    uint32_t id;
    QString name;

    QString type;
    unique_document document;

    uint32_t index;
    std::string buildpath;

    virtual uint32_t add_dependant(Studio::Document *document, QString type) = 0;
    virtual uint32_t add_dependant(Studio::Document *document, uint32_t index, QString type) = 0;
  };

  //-------------------------- PackManager ------------------------------------
  //---------------------------------------------------------------------------

  class PACKPLUGIN_EXPORT PackManager : public QObject
  {
    Q_OBJECT

    public:

      virtual void register_packer(QString const &type, QObject *packer) = 0;

    protected:
      virtual ~PackManager() { }
  };
}
