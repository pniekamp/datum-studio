//
// Datum Viewport
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "datum.h"
#include "platform.h"
#include "documentapi.h"
#include <QWidget>

//-------------------------- Viewport ---------------------------------------
//---------------------------------------------------------------------------

class Viewport : public QWidget
{
  Q_OBJECT

  public:
    Viewport(size_t slabsize = 8*1024, size_t storagesize = 2*1024*1024, QWidget *parent = nullptr);
    virtual ~Viewport();

  public:

    class ResourceProxy
    {
      public:
        ResourceProxy(ResourceManager *manager) : m_manager(manager) { }

        template<typename Resource, typename ...Args>
        unique_resource<Resource> create(Args... args) { return { m_manager, m_manager->create<Resource>(std::forward<Args>(args)...) }; }

        template<typename Resource, typename ...Args>
        void update(Resource const *resource, Args... args) { m_manager->update(resource, std::forward<Args>(args)...); }

        template<typename Resource, typename ...Args>
        unique_resource<Resource> load(size_t asset, Args... args);

        template<typename Resource, typename ...Args>
        unique_resource<Resource> load(std::istream &fin, size_t index, Args... args);

        template<typename Resource, typename ...Args>
        unique_resource<Resource> load(Studio::Document *document, size_t index, Args... args);

      public:

        ResourceManager::TransferLump const *acquire_lump(size_t size) { return m_manager->acquire_lump(size); }

        void release_lump(ResourceManager::TransferLump const *lump) { m_manager->release_lump(lump); }

      private:
        ResourceManager *m_manager;
    };

    ResourceProxy resources;

  public:

    Camera camera;
    RenderParams renderparams;

    bool prepare();

    bool begin(SpriteList &sprites, SpriteList::BuildState &buildstate);

    void push_sprites(SpriteList const &sprites);

    bool begin(GeometryList &geometry, GeometryList::BuildState &buildstate);

    void push_geometry(GeometryList const &geometry);

    bool begin(ForwardList &objects, ForwardList::BuildState &buildstate);

    void push_objects(ForwardList const &objects);

    bool begin(OverlayList &overlays, OverlayList::BuildState &buildstate);

    void push_overlays(OverlayList const &overlays);

    void render();

  protected:

    void hideEvent(QHideEvent *event) override;

    QPaintEngine *paintEngine() const override;

    void paintEvent(QPaintEvent *event);

  private:

    DatumPlatform::GameMemory scratchmemory;

    PushBuffer m_pushbuffer;

    RenderContext m_rendercontext;

    Vulkan::Surface surface;

    Vulkan::Swapchain swapchain;
    Vulkan::Semaphore acquirecomplete;
    Vulkan::Semaphore rendercomplete;

    VkImage presentimages[2];

    size_t m_resourcetoken;
};
