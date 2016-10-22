//
// Mesh View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "mesh.h"
#include "viewport.h"

//-------------------------- MeshView ---------------------------------------
//---------------------------------------------------------------------------

class MeshView : public Viewport
{
  Q_OBJECT

  public:
    MeshView(QWidget *parent = nullptr);

  public slots:

    void view(Studio::Document *document);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    float m_yawsign;

    lml::Vec3 m_focuspoint;

    QPoint m_mousepresspos, m_mousemovepos;

    unique_resource<Material> m_material;

    struct MeshInstance
    {
      lml::Transform transform;
      unique_resource<Mesh> mesh;
    };

    std::vector<MeshInstance> m_meshes;

    MeshDocument m_document;
};
