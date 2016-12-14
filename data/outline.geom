#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout(std430, set=0, binding=0, row_major) buffer SceneSet 
{
  mat4 proj;
  mat4 invproj;
  mat4 view;
  mat4 invview;
  mat4 worldview;
  mat4 prevview;
  mat4 skyview;
  vec4 viewport;
  
} scene;

layout(location=0) in vec2 texcoords[];

layout(location=0) noperspective out vec4 fbocoord;

const float Overhang = 0.005f;
const float HalfWidth = 0.005f;

void EmitPt(vec3 pt)
{
  fbocoord = vec4(0.5 * pt.xy + 0.5, pt.z, 1);
  gl_Position = vec4(pt.x * scene.viewport.z / (scene.viewport.z + 2*scene.viewport.x), pt.y * scene.viewport.w / (scene.viewport.w + 2*scene.viewport.y), pt.z, 1);
  EmitVertex();
}

void EmitEdge(vec3 p0, vec3 p1)
{
  vec3 e = Overhang * vec3(p1.xy - p0.xy, 0);
  vec2 v = normalize(p1.xy - p0.xy);
  vec3 n = vec3(-v.y, v.x, 0) * HalfWidth;

  EmitPt(p0 + n - e); 
  EmitPt(p0 - n - e); 
  EmitPt(p1 + n + e); 
  EmitPt(p1 - n + e); 
  EndPrimitive();
}

///////////////////////// main //////////////////////////////////////////////
void main()
{
  if (gl_in[0].gl_Position.w > 0.1 && gl_in[1].gl_Position.w > 0.1 && gl_in[2].gl_Position.w > 0.1)
  {
    vec3 p0 = gl_in[0].gl_Position.xyz/gl_in[0].gl_Position.w;
    vec3 p1 = gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w;
    vec3 p2 = gl_in[2].gl_Position.xyz/gl_in[2].gl_Position.w;
    
    float area = (p2.x - p0.x) * (p1.y - p0.y) - (p2.y - p0.y) * (p1.x - p0.x);

    if (area > 0)
    {
      EmitEdge(p0, p1);
      EmitEdge(p1, p2);
      EmitEdge(p2, p0);
    }
  }
}  
