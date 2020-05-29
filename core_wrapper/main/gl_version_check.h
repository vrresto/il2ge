#ifndef IL2GE_GL_VERSION_CHECK_H
#define IL2GE_GL_VERSION_CHECK_H

#include <functional>

namespace il2ge
{
  void checkGLVersion(std::function<void*(const char*)> get_proc_address);
}

#endif
