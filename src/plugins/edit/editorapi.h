//
// Editor API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

#if defined(EDITPLUGIN)
# define EDITPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define EDITPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{

  //-------------------------- EditorManager ----------------------------------
  //---------------------------------------------------------------------------

  class EDITPLUGIN_EXPORT EditorManager : public QObject
  {
    Q_OBJECT

    public:

      virtual void open_editor(QString const &type, QString const &path) = 0;

      virtual int save_all() = 0;
      virtual int close_all() = 0;

    protected:
      virtual ~EditorManager() { }
  };

}
