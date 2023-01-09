#include "foundation.h"

#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "floor.frag.h"
#include "floor.vert.h"

Foundation::Foundation(
    zukou::System* system, zukou::VirtualObject* virtual_object)
    : virtual_object_(virtual_object),
      pool_(system),
      gl_vertex_buffer_(system),
      gl_element_array_buffer_(system),
      vertex_array_(system),
      vertex_shader_(system),
      fragment_shader_(system),
      program_(system),
      rendering_unit_(system),
      base_technique_(system)
{
  ConstructVertices();
  ConstructElements();
}

Foundation::~Foundation()
{
  if (fd_ != 0) {
    close(fd_);
  }
}

Foundation::Vertex::Vertex(float x, float y, float z) : x(x), y(y), z(z) {}

bool
Foundation::Render(float radius, glm::mat4 transform)
{
  if (!initialized_ && Init() == false) {
    return false;
  }

  radius_ = radius;

  auto local_model = transform * glm::scale(glm::mat4(1), glm::vec3(radius));
  base_technique_.Uniform(0, "local_model", local_model);

  return true;
}

bool
Foundation::Init()
{
  fd_ = zukou::Util::CreateAnonymousFile(pool_size());
  if (!pool_.Init(fd_, pool_size())) return false;
  if (!vertex_buffer_.Init(&pool_, 0, vertex_buffer_size())) return false;
  if (!element_array_buffer_.Init(
          &pool_, vertex_buffer_size(), element_array_buffer_size()))
    return false;

  if (!gl_vertex_buffer_.Init()) return false;
  if (!gl_element_array_buffer_.Init()) return false;
  if (!vertex_array_.Init()) return false;

  if (!vertex_shader_.Init(GL_VERTEX_SHADER, floor_vert_shader_source))
    return false;
  if (!fragment_shader_.Init(GL_FRAGMENT_SHADER, floor_frag_shader_source))
    return false;
  if (!program_.Init()) return false;

  if (!rendering_unit_.Init(virtual_object_)) return false;
  if (!base_technique_.Init(&rendering_unit_)) return false;

  {
    auto vertex_buffer_data = static_cast<char*>(
        mmap(nullptr, pool_size(), PROT_WRITE, MAP_SHARED, fd_, 0));
    auto element_array_buffer_data = vertex_buffer_data + vertex_buffer_size();

    std::memcpy(vertex_buffer_data, vertices_.data(), vertex_buffer_size());
    std::memcpy(element_array_buffer_data, elements_.data(),
        element_array_buffer_size());

    munmap(vertex_buffer_data, pool_size());
  }

  gl_vertex_buffer_.Data(GL_ARRAY_BUFFER, &vertex_buffer_, GL_STATIC_DRAW);
  gl_element_array_buffer_.Data(
      GL_ELEMENT_ARRAY_BUFFER, &element_array_buffer_, GL_STATIC_DRAW);

  program_.AttachShader(&vertex_shader_);
  program_.AttachShader(&fragment_shader_);
  program_.Link();

  vertex_array_.Enable(0);
  vertex_array_.VertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0, &gl_vertex_buffer_);

  base_technique_.Bind(&vertex_array_);
  base_technique_.Bind(&program_);

  base_technique_.DrawElements(GL_TRIANGLES, elements_.size(),
      GL_UNSIGNED_SHORT, 0, &gl_element_array_buffer_);

  initialized_ = true;

  return true;
}

void
Foundation::ConstructVertices()
{
  vertices_.emplace_back(0, 0, 0);
  vertices_.emplace_back(0, height_, 0);
  for (float r = 0; r <= resolution_; r++) {
    float theta = M_PI * 2.f * r / float(resolution_);

    float x = radius_ * cosf(theta);
    float y0 = 0;
    float z = radius_ * sinf(theta);
    vertices_.emplace_back(x, y0, z);

    float y1 = height_;
    vertices_.emplace_back(x, y1, z);
  }
}

void
Foundation::ConstructElements()
{
  ushort O0 = 0;
  ushort O1 = 1;

  for (int32_t i = 0; i <= resolution_; i++) {
    ushort A = 2 * i;
    ushort B = 2 * i + 1;
    ushort C = 2 * (i + 1);
    ushort D = 2 * (i + 1) + 1;

    // upper circle
    elements_.push_back(A);
    elements_.push_back(C);
    elements_.push_back(O0);

    // lower circle
    elements_.push_back(B);
    elements_.push_back(D);
    elements_.push_back(O1);

    // side 1
    elements_.push_back(A);
    elements_.push_back(B);
    elements_.push_back(D);

    // side 2
    elements_.push_back(C);
    elements_.push_back(A);
    elements_.push_back(D);
  }
}
