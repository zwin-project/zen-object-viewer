#include <zukou.h>

#include <iostream>

#include "viewer.h"

std::string
ExtractExtension(std::string &filename)
{
  std::string extension = "";
  for (int i = filename.size() - 2; i >= 0; --i) {
    if (filename[i] == '.') {
      extension = filename.substr(i + 1);
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
  std::string path = "./assets/default_text.stl";
  float radius = 0.25;
  glm::mat4 transform =
      glm::rotate(glm::mat4(1), -(float)M_PI / 2, glm::vec3(0, 1, 0));

  if (argc >= 2) {
    path = argv[1];
    transform = glm::mat4(1);
  }

  auto extension = ExtractExtension(path);
  if (extension != "stl" && extension != "STL") {
    std::cerr << "This file extension is not yet supported.:" << extension
              << " " << std::endl;
    return EXIT_FAILURE;
  }

  if (!viewer.Init(radius, transform)) {
    std::cerr << "Failed to initialize viewer." << std::endl;
    return EXIT_FAILURE;
  }

  if (!viewer.Load(path)) {
    std::cerr << "Failed to load path.: " << path << std::endl;
    return EXIT_FAILURE;
  }

  return viewer.Run();
}
