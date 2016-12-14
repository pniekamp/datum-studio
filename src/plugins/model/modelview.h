//
// Model View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "model.h"
#include "skybox.h"
#include "viewport.h"

//-------------------------- ModelView --------------------------------------
//---------------------------------------------------------------------------

class ModelView : public Viewport
{
  Q_OBJECT

  public:
    ModelView(QWidget *parent = nullptr);

  public slots:

    void view(ModelDocument *document);
    void edit(ModelDocument *document);

    void set_selection(int index);

    void set_skybox(QString const &path);

    void set_exposure(float value);

  signals:

    void exposure_changed(float value);

    void selection_changed(int index);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void on_skybox_build_complete(Studio::Document *document, QString const &path);

    void on_material_build_complete(Studio::Document *document, QString const &path);

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    SkyboxDocument m_skyboxdocument;

    unique_resource<SkyBox> m_skybox;

  public:

    struct MeshData
    {
      size_t hash = 0;

      unique_resource<Mesh> mesh;
    };

    std::map<std::pair<Studio::Document*, size_t>, MeshData> m_meshes;

    Mesh const *find_or_create_mesh(Studio::Document *document, size_t index);

    struct MaterialData
    {
      size_t hash = 0;

      unique_resource<Texture> albedomap;
      unique_resource<Texture> specularmap;
      unique_resource<Texture> normalmap;

      struct TintData
      {
        lml::Color3 tint;

        unique_resource<Material> material;
      };

      std::vector<TintData> tints;
    };

    std::map<Studio::Document*, MaterialData> m_materials;

    Material const *find_or_create_material(Studio::Document *document, lml::Color3 const &tint);

    struct Instance
    {
      int index;
      Mesh const *mesh;
      Material const *material;
      lml::Transform transform;
      lml::Bound3 bound;
    };

    std::vector<Instance> m_instances;

    std::vector<Instance> m_transparencies;

    unique_resource<Mesh> m_homocube;

    QTimer *m_updatetimer;

  private:

    enum class ModeType
    {
      None,
      Translating,
      Rotating,
    };

    ModeType m_mode;

    float m_yawsign;

    lml::Vec3 m_focuspoint;

    QPoint m_keypresspos;
    QPoint m_mousepresspos, m_mousemovepos;

    int m_selection;
    size_t m_hitrotation;

    lml::Transform m_selectedtransform;

    bool m_readonly;

    ModelDocument *m_document = nullptr;
};
