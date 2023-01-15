#include "viewer.h"

#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "default.frag.h"
#include "default.vert.h"
#include "file-reader.h"

#pragma pack(1)
struct StlTriangle {
  float n[3];
  float points[3][3];
  uint16_t unused;
};
#pragma pack()

Viewer::Viewer()
    : system_(this),
      bounded_(&system_, this),
      pool_(&system_),
      rendering_unit_(&system_),
      base_technique_(&system_),
      gl_vertex_buffer_(&system_),
      vertex_array_(&system_),
      vertex_shader_(&system_),
      fragment_shader_(&system_),
      program_(&system_),
      foundation_(&system_, &bounded_)
{}

Viewer::~Viewer()
{
  if (fd_ != 0) {
    close(fd_);
  }
}

bool
Viewer::Init(std::string &title, float radius, glm::mat4 transform)
{
  radius_ = radius;
  transform_ = transform;

  if (!system_.Init()) return false;
  if (!bounded_.Init(glm::vec3(radius))) return false;

  bounded_.SetTitle(title);

  return true;
}

bool
Viewer::Load(std::string path)
{
  char unused[80];
  uint32_t triangle_count;
  StlTriangle buffer;
  FileReader *file_reader = new FileReader();

  if (!file_reader->Open(path)) {
    std::cerr << "opened " << path << " failed." << std::endl;
    return false;
  }

  if (!file_reader->Read(unused, 80)) {
    std::cerr << "file read failed." << std::endl;
    return false;
  }

  if (std::string(unused, 5) == "solid") {
    std::cerr << path
              << ": this file seems \"ASCII\" STL which is not "
                 "supported yet"
              << std::endl;
    return false;
  }

  if (!file_reader->Read(
          reinterpret_cast<char *>(&triangle_count), sizeof(triangle_count))) {
    std::cerr << "file broken." << std::endl;
    return false;
  }

  if (triangle_count <= 0) {
    std::cerr << "triangle count error." << std::endl;
    return false;
  }

  vertices_.reserve(triangle_count * 3);
  for (uint32_t i = 0; i < triangle_count; i++) {
    if (!file_reader->Read(
            reinterpret_cast<char *>(&buffer), sizeof(StlTriangle))) {
      std::cerr << "file broken." << std::endl;
      return false;
    }

    float *n = buffer.n;
    for (int i = 0; i < 3; i++) {
      float *p = buffer.points[i];
      auto point = glm::vec3(p[0], p[2], -p[1]);

      min_vec_.x = std::min(min_vec_.x, point[0]);
      min_vec_.y = std::min(min_vec_.y, point[1]);
      min_vec_.z = std::min(min_vec_.z, point[2]);
      max_vec_.x = std::max(max_vec_.x, point[0]);
      max_vec_.y = std::max(max_vec_.y, point[1]);
      max_vec_.z = std::max(max_vec_.z, point[2]);
      vertices_.push_back(Vertex(point, glm::vec3(n[0], n[2], -n[1])));
    }
  }

  return true;
}

void
Viewer::Configure(glm::vec3 half_size, uint32_t serial)
{
  radius_ = std::min(std::min(half_size[0], half_size[1]), half_size[2]);

  if (!foundation_.Render(radius_,
          glm::translate(glm::mat4(1), glm::vec3(.0f, -radius_, .0f)))) {
    std::cerr << "Failed to render foundation." << std::endl;
    return;
  }

  glm::vec3 delta(0);
  float zoom = FLT_MAX;
  for (int i = 0; i < 3; i++) {
    delta[i] = (max_vec_[i] + min_vec_[i]) / 2;

    float l = max_vec_[i] - min_vec_[i];
    float r = half_size[i] * 2 / l;
    zoom = std::min(zoom, r);
  }

  for (auto &vertex : vertices_) {
    vertex.point = (vertex.point - delta) * zoom;
  }

  if (!this->Render()) {
    std::cerr << "Failed to render viewer." << std::endl;
    return;
  }

  base_technique_.Uniform(0, "local_model", transform_);
  base_technique_.Uniform(0, "focus_color_diff", glm::vec3(0));

  zukou::Region region(&system_);
  if (!region.Init()) {
    std::cerr << "region init failed!" << std::endl;
    return;
  }

  region.AddCuboid(half_size, glm::vec3(0), glm::quat(glm::vec3(0)));

  bounded_.SetRegion(&region);

  bounded_.AckConfigure(serial);
  bounded_.Commit();
}

void
Viewer::RayEnter(uint32_t /*serial*/, zukou::VirtualObject * /*virtual_object*/,
    glm::vec3 /*origin*/, glm::vec3 /*direction*/)
{
  base_technique_.Uniform(0, "focus_color_diff", glm::vec3(0.1));
  bounded_.Commit();
};

void
Viewer::RayLeave(uint32_t /*serial*/, zukou::VirtualObject * /*virtual_object*/)
{
  base_technique_.Uniform(0, "focus_color_diff", glm::vec3(0));
  bounded_.Commit();
};

void
Viewer::RayAxisFrame(const zukou::RayAxisEvent &event)
{
  float diff = event.vertical / 90.0f;
  transform_ *= glm::rotate(glm::mat4(1), diff, glm::vec3(0, 1, 0));
  base_technique_.Uniform(0, "local_model", transform_);
  bounded_.Commit();
}

bool
Viewer::Run()
{
  return system_.Run();
}

bool
Viewer::Render()
{
  size_t vertex_buffer_size =
      sizeof(decltype(vertices_)::value_type) * vertices_.size();
  size_t pool_size = vertex_buffer_size;

  fd_ = zukou::Util::CreateAnonymousFile(pool_size);

  if (!pool_.Init(fd_, pool_size)) return false;

  if (!vertex_buffer_.Init(&pool_, 0, vertex_buffer_size)) return false;

  if (!gl_vertex_buffer_.Init()) return false;
  if (!vertex_array_.Init()) return false;
  if (!vertex_shader_.Init(GL_VERTEX_SHADER, default_vert_shader_source))
    return false;
  if (!fragment_shader_.Init(GL_FRAGMENT_SHADER, default_frag_shader_source))
    return false;
  if (!program_.Init()) return false;

  if (!rendering_unit_.Init(&bounded_)) return false;
  if (!base_technique_.Init(&rendering_unit_)) return false;

  {
    auto vertex_buffer_data = static_cast<char *>(
        mmap(nullptr, pool_size, PROT_WRITE, MAP_SHARED, fd_, 0));

    std::memcpy(vertex_buffer_data, vertices_.data(), vertex_buffer_size);

    munmap(vertex_buffer_data, pool_size);
  }

  gl_vertex_buffer_.Data(GL_ARRAY_BUFFER, &vertex_buffer_, GL_STATIC_DRAW);

  program_.AttachShader(&vertex_shader_);
  program_.AttachShader(&fragment_shader_);
  program_.Link();

  vertex_array_.Enable(0);
  vertex_array_.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      offsetof(Vertex, point), &gl_vertex_buffer_);
  vertex_array_.Enable(1);
  vertex_array_.VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      offsetof(Vertex, norm), &gl_vertex_buffer_);

  base_technique_.Bind(&vertex_array_);
  base_technique_.Bind(&program_);

  base_technique_.DrawArrays(GL_TRIANGLES, 0, vertices_.size());

  return true;
}
