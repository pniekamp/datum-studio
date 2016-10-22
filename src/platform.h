//
// Datum Platform
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "datum.h"
#include "datum/renderer.h"

namespace Studio
{
  class STUDIO_EXPORT Platform : public QObject
  {
    Q_OBJECT

    public:

      static DatumPlatform::PlatformInterface *instance();

    public:

      virtual AssetManager *assets() = 0;
      virtual ResourceManager *resources() = 0;

      virtual Vulkan::Surface create_surface(WId wid) = 0;
  };
}

void initialise_platform(QWindow *window);
