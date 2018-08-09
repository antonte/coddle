#pragma once
#include <string>

class Config
{
public:
  Config(int argc, char **argv);
  std::string remoteRepository;
  std::string remoteVersion;
  std::string localRepository;
  std::string localVersion;
private:
  void loadConfig(const std::string& configFileName);
};

