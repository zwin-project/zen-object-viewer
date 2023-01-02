#pragma once

#include <fstream>
#include <string>

class FileReader
{
 public:
  FileReader();
  bool Open(std::string path);
  bool Read(char *buffer, size_t n);
  bool End();
  bool GetLine(std::string *str);

 private:
  std::ifstream ifs_;
};
