#include <zukou.h>

#include <iostream>

#include "viewer.h"

std::string
ExtractFilename(std::string &path)
{
  std::string extension = "";
  for (int i = path.size() - 2; i >= 0; --i) {
    if (path[i] == '/') {
      extension = path.substr(i + 1);
      break;
    }
  }

  assert(extension != "");

  return extension;
}

std::string
ExtractExtension(std::string &path)
{
  std::string extension = "";
  for (int i = path.size() - 2; i >= 0; --i) {
    if (path[i] == '.') {
      extension = path.substr(i + 1);
      break;
    }
  }

  assert(extension != "");

  return extension;
}

int
main(int argc, char const *argv[])
{
  Viewer viewer;
  std::string path = ZEN_OBJECT_VIEWER_ASSET_DIR "/default_text.stl";
  std::string name = "Zen Object Viewer";
  float radius = 0.125;
  glm::mat4 transform =
      glm::rotate(glm::mat4(1), -(float)M_PI / 2, glm::vec3(0, 1, 0));

  if (argc >= 2) {
    path = argv[1];
    transform = glm::mat4(1);
    name = ExtractFilename(path);
  }

  auto extension = ExtractExtension(path);
  if (extension != "stl" && extension != "STL") {
    std::cerr << "This file extension is not yet supported.:" << extension
              << " " << std::endl;
    return EXIT_FAILURE;
  }

  if (!viewer.Init(name, radius, transform)) {
    std::cerr << "Failed to initialize viewer." << std::endl;
    return EXIT_FAILURE;
  }

  if (!viewer.Load(path)) {
    std::cerr << "Failed to load path.: " << path << std::endl;
    return EXIT_FAILURE;
  }

  return viewer.Run();
}
