#ifndef IL2GE_VERSION_H
#define IL2GE_VERSION_H

#include <string>

namespace il2ge::version
{

  const std::string &getBuildJobID();
  const std::string &getCommitSHA();
  bool isDebugBuild();

}

#endif
