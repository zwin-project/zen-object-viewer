#pragma once

#include <zukou.h>

#include <float.h>

#include <vector>

#include "foundation.h"

struct Vertex {
  Vertex(glm::vec3 point, glm::vec3 norm)
  {
    this->point = point;
    this->norm = norm;
  };

  glm::vec3 point;
  glm::vec3 norm;
};

class Viewer : public zukou::IBoundedDelegate, public zukou::ISystemDelegate
{
 public:
  DISABLE_MOVE_AND_COPY(Viewer);
  Viewer();
  ~Viewer();

  bool Init(std::string& title, float radius, glm::mat4 transform);

  bool Load(std::string path);

  void Configure(glm::vec3 half_size, uint32_t serial) override;

  void RayEnter(uint32_t /*serial*/, zukou::VirtualObject* /*virtual_object*/,
      glm::vec3 /*origin*/, glm::vec3 /*direction*/) override;

  void RayLeave(
      uint32_t /*serial*/, zukou::VirtualObject* /*virtual_object*/) override;

  void RayAxisFrame(const zukou::RayAxisEvent& /*event*/) override;

  bool Run();

 private:
  float radius_ = 0.25f;
  glm::mat4 transform_ = glm::mat4(1);
  glm::vec3 min_vec_ = glm::vec3(FLT_MAX);
  glm::vec3 max_vec_ = glm::vec3(FLT_MIN);

  zukou::System system_;
  zukou::Bounded bounded_;

  int fd_ = 0;
  zukou::ShmPool pool_;
  zukou::RenderingUnit rendering_unit_;
  zukou::GlBaseTechnique base_technique_;

  zukou::Buffer vertex_buffer_;

  zukou::GlBuffer gl_vertex_buffer_;

  zukou::GlVertexArray vertex_array_;

  zukou::GlShader vertex_shader_;
  zukou::GlShader fragment_shader_;
  zukou::GlProgram program_;

  std::vector<Vertex> vertices_;

  Foundation foundation_;

  bool Render();
};
