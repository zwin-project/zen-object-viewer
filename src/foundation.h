#pragma once

#include <zukou.h>

#include <vector>

#define INITIAL_RESOLUTION 64
#define INITIAL_RADIUS 1.0
#define INITIAL_HEIGHT 0.05

class Foundation
{
  struct Vertex {
    Vertex(float x, float y, float z);
    float x, y, z;
  };

 public:
  DISABLE_MOVE_AND_COPY(Foundation);
  Foundation() = delete;
  Foundation(zukou::System* system, zukou::VirtualObject* virtual_object);
  ~Foundation();

  bool Render(float scale, glm::mat4 transform);

 private:
  bool Init();

  void ConstructVertices();
  void ConstructElements();

  bool initialized_ = false;

  zukou::VirtualObject* virtual_object_;

  float radius_ = INITIAL_RADIUS;
  float height_ = INITIAL_HEIGHT;
  int resolution_ = INITIAL_RESOLUTION;

  int fd_ = 0;
  zukou::ShmPool pool_;
  zukou::Buffer vertex_buffer_;
  zukou::Buffer element_array_buffer_;

  zukou::GlBuffer gl_vertex_buffer_;
  zukou::GlBuffer gl_element_array_buffer_;
  zukou::GlVertexArray vertex_array_;
  zukou::GlShader vertex_shader_;
  zukou::GlShader fragment_shader_;
  zukou::GlProgram program_;
  zukou::RenderingUnit rendering_unit_;
  zukou::GlBaseTechnique base_technique_;

  std::vector<Vertex> vertices_;
  std::vector<ushort> elements_;

  inline size_t vertex_buffer_size();
  inline size_t element_array_buffer_size();
  inline size_t pool_size();
};

inline size_t
Foundation::vertex_buffer_size()
{
  return sizeof(decltype(vertices_)::value_type) * vertices_.size();
}

inline size_t
Foundation::element_array_buffer_size()
{
  return sizeof(decltype(elements_)::value_type) * elements_.size();
}

inline size_t
Foundation::pool_size()
{
  return vertex_buffer_size() + element_array_buffer_size();
}
