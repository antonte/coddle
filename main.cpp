#include "coddle.hpp"
#include "config.hpp"
#include "file_exist.hpp"
#include "osal.hpp"
#include "make_path.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  if (isFileExist("coddle.cfg"))
  {
    Config config(argc, argv);
    std::cout << "coddle: Entering directory `coddle.cfg'" << std::endl;
    changeDir("coddle.cfg");
    config.multithread = true;
    ProjectConfig project;
    project.srcDirs.push_back(".");
    project.targetType = TargetType::SharedLib;
    config.projects.push_back(project);
    auto res = coddle(&config);
    std::cout << "coddle: Leaving directory `coddle.cfg'" << std::endl;
    changeDir("..");
    if (res != 0)
      return res;
  }
  Config config(argc, argv);
  if (isFileExist(makePath("coddle.cfg", "libcoddle.cfg.so")))
  {
    SharedLib lib(makePath("coddle.cfg", "libcoddle.cfg.so"));
    auto configure = (void (*)(Config &))lib.symbol("_Z9configureR6Config");
    if (!configure)
    {
      std::cerr << "coddle: undefined reference to `configure(Config&)'\n";
      return 2;
    }
    configure(config);
  }
  if (!config.gitLibs.empty())
  {
    makeDir(".gitlibs");
    changeDir(".gitlibs");
    for (const auto& lib: config.gitLibs)
    {
      auto p = lib.first.rfind("/");
      auto dirName = lib.first.substr(p + 1);
      dirName.resize(dirName.size() - 4);
      if (isDirExist(dirName))
        continue;
      execShowCmd("git clone --depth 1", lib.first, "-b", lib.second, dirName);
    }
    changeDir("..");
  }
  ProjectConfig project;
  project.srcDirs.push_back(".");
  config.projects.push_back(project);
  return coddle(&config);
}
