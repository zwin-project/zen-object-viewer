#include <zukou.h>

#include <sys/mman.h>

#include <cstring>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <vector>

#include "default.fragment.h"
#include "default.vert.h"
#include "file-reader.h"

#pragma pack(1)
struct StlTriangle {
  float n[3];
  float points[3][3];
  uint16_t unused;
};
#pragma pack()

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
  Viewer(std::vector<StlTriangle> *triangles)
      : system_(this),
        bounded_(&system_, this),
        pool(&system_),
        rendering_unit(&system_),
        base_technique(&system_),
        gl_vertex_buffer(&system_),
        vertex_array(&system_),
        vertex_shader(&system_),
        fragment_shader(&system_),
        program(&system_),
        triangles_(triangles)
  {}

  bool Init(glm::vec3 half_size)
  {
    if (!system_.Init()) return false;
    if (!bounded_.Init(half_size)) return false;
    return true;
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    if (!this->Render()) {
      std::cerr << "rendering failed" << std::endl;
      return;
    }

    auto local_model = glm::scale(glm::mat4(1), half_size);
    glm::vec4 color = {0, 0, 0, 0};
    base_technique.Uniform(0, "local_model", local_model);
    base_technique.Uniform(0, "color", color);

    bounded_.AckConfigure(serial);
    bounded_.Commit();
  }

  bool Run() { return system_.Run(); }

 private:
  zukou::System system_;
  zukou::Bounded bounded_;

 private:
  zukou::ShmPool pool;
  zukou::RenderingUnit rendering_unit;
  zukou::GlBaseTechnique base_technique;

  zukou::Buffer vertex_buffer;

  zukou::GlBuffer gl_vertex_buffer;

  zukou::GlVertexArray vertex_array;

  zukou::GlShader vertex_shader;
  zukou::GlShader fragment_shader;
  zukou::GlProgram program;

  std::vector<StlTriangle> *triangles_;

 private:
  bool Render()
  {
    std::vector<Vertex> vertices;
    vertices.reserve(triangles_->size() * 3);
    {
      float max_norm = 0;
      for (auto triangle : *triangles_) {
        for (int i = 0; i < 3; i++) {
          float *p = triangle.points[i];
          max_norm = std::max(
              max_norm, std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]));
        }
      }

      for (auto triangle : *triangles_) {
        float *n = triangle.n;
        for (int i = 0; i < 3; i++) {
          float *p = triangle.points[i];
          vertices.push_back(Vertex(
              glm::vec3(p[0] / max_norm, p[2] / max_norm, -p[1] / max_norm),
              glm::vec3(n[0], n[2], -n[1])));
        }
      }
    }

    size_t vertex_buffer_size =
        sizeof(decltype(vertices)::value_type) * vertices.size();
    size_t pool_size = vertex_buffer_size;

    int fd = 0;
    fd = zukou::Util::CreateAnonymousFile(pool_size);

    if (!pool.Init(fd, pool_size)) return false;

    if (!vertex_buffer.Init(&pool, 0, vertex_buffer_size)) return false;

    if (!gl_vertex_buffer.Init()) return false;
    if (!vertex_array.Init()) return false;
    if (!vertex_shader.Init(GL_VERTEX_SHADER, default_vertex_shader_source))
      return false;
    if (!fragment_shader.Init(
            GL_FRAGMENT_SHADER, default_fragment_shader_source))
      return false;
    if (!program.Init()) return false;

    if (!rendering_unit.Init(&bounded_)) return false;
    if (!base_technique.Init(&rendering_unit)) return false;

    {
      auto vertex_buffer_data = static_cast<char *>(
          mmap(nullptr, pool_size, PROT_WRITE, MAP_SHARED, fd, 0));

      std::memcpy(vertex_buffer_data, vertices.data(), vertex_buffer_size);

      munmap(vertex_buffer_data, pool_size);
    }

    gl_vertex_buffer.Data(GL_ARRAY_BUFFER, &vertex_buffer, GL_STATIC_DRAW);

    program.AttachShader(&vertex_shader);
    program.AttachShader(&fragment_shader);
    program.Link();

    vertex_array.Enable(0);
    vertex_array.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        offsetof(Vertex, point), &gl_vertex_buffer);
    vertex_array.Enable(1);
    vertex_array.VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        offsetof(Vertex, norm), &gl_vertex_buffer);

    base_technique.Bind(&vertex_array);
    base_technique.Bind(&program);

    base_technique.DrawArrays(GL_TRIANGLES, 0, vertices.size());

    return true;
  }
};

int
main(int argc, char const *argv[])
{
  if (argc != 2) {
    std::cerr << "argument number error" << std::endl;
    std::cerr << "Usage:\t" << argv[0] << " [FILE]" << std::endl;
    return EXIT_FAILURE;
  }

  std::string path(argv[1]);

  std::vector<StlTriangle> triangles;
  {
    char unused[80];
    uint32_t triangle_count;
    StlTriangle buffer;
    FileReader *file_reader = new FileReader();

    if (!file_reader->Open(path)) {
      std::cerr << "opened " << path << " failed." << std::endl;
      return EXIT_FAILURE;
    }

    if (!file_reader->Read(unused, 80)) {
      std::cerr << "file read failed." << std::endl;
      return EXIT_FAILURE;
    }

    if (std::string(unused, 5) == "solid") {
      std::cerr << path
                << ": this file seems \"ASCII\" STL which is not "
                   "supported yet"
                << std::endl;
      return EXIT_FAILURE;
    }

    if (!file_reader->Read(reinterpret_cast<char *>(&triangle_count),
            sizeof(triangle_count))) {
      std::cerr << "file broken." << std::endl;
      return EXIT_FAILURE;
    }

    if (triangle_count <= 0) {
      std::cerr << "triangle count error." << std::endl;
      return EXIT_FAILURE;
    }

    triangles.reserve(triangle_count);

    for (uint32_t i = 0; i < triangle_count; i++) {
      if (!file_reader->Read(
              reinterpret_cast<char *>(&buffer), sizeof(StlTriangle))) {
        std::cerr << "file broken." << std::endl;
        return EXIT_FAILURE;
      }
      triangles.push_back(buffer);
    }
  }

  Viewer viewer(&triangles);

  glm::vec3 half_size(0.25, 0.25, 0.25);

  if (!viewer.Init(half_size)) {
    std::cerr << "system init error" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "start running" << std::endl;
  return viewer.Run();
}
