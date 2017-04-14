//
// Material View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "material.h"
#include "mesh.h"
#include "skybox.h"
#include "viewport.h"

//-------------------------- MaterialView -----------------------------------
//---------------------------------------------------------------------------

class MaterialView : public Viewport
{
  Q_OBJECT

  public:
    MaterialView(QWidget *parent = nullptr);

  public slots:

    void view(Studio::Document *document);

    void set_mesh(QString const &path);
    void set_skybox(QString const &path);

    void set_exposure(float value);

  signals:

    void exposure_changed(float value);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void on_material_build_complete(Studio::Document *document, QString const &path);

    void on_skybox_build_complete(Studio::Document *document, QString const &path);

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

  private:

    MeshDocument m_meshdocument;

    struct MeshInstance
    {
      lml::Transform transform;
      unique_resource<Mesh> mesh;
    };

    std::vector<MeshInstance> m_meshes;

    QTimer *m_updatetimer;

  private:

    float m_yawsign;

    lml::Vec3 m_focuspoint;

    QPoint m_mousepresspos, m_mousemovepos;

    size_t m_buildhash;
    QString m_buildpath;

    unique_resource<Texture> m_albedomap;
    unique_resource<Texture> m_specularmap;
    unique_resource<Texture> m_normalmap;
    unique_resource<Material> m_material;

    MaterialDocument m_document;
};
