/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2019 Jan Lepper
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IL2GE_CORE_WRAPPER_JAVA_UTIL_H
#define IL2GE_CORE_WRAPPER_JAVA_UTIL_H

#include <jni.h>

namespace il2ge::java
{


class ClassNotFoundException : public std::exception
{
public:
  const char *what() const noexcept override { return "class not found"; }
};

class MethodNotFoundException : public std::exception
{
  std::string m_what;
public:
  MethodNotFoundException(const std::string &name)
  {
    m_what = "method not found: " + name;
  }
  const char *what() const noexcept override { return m_what.c_str(); }
};

class FieldNotFoundException : public std::exception
{
public:
  const char *what() const noexcept override { return "field not found"; }
};

class NoJNIEnvException : public std::exception
{
public:
  const char *what() const noexcept override { return "no JNI environment"; }
};

class NullObjectException: public std::exception
{
public:
  const char *what() const noexcept override { return "null object passed"; }
};


inline std::string javaStringToStd(JNIEnv *env, jstring java_string)
{
  jsize len = env->GetStringUTFLength(java_string);
  char buf[len+1];
  env->GetStringUTFRegion(java_string, 0, len, buf);
  return std::move(std::string(buf, len));
}


inline std::string javaStringToStd(JNIEnv *env, jobject java_string)
{
  return std::move(javaStringToStd(env, (jstring)java_string));
}


class Class
{
  JNIEnv *m_env = nullptr;
  jclass m_id = 0;

public:
  Class &operator=(const &Class) = delete;
  Class(const &Class) = delete;
  Class(const char *name, JNIEnv *env = nullptr)
  {
    m_env = env;
    if (!m_env)
      m_env = il2ge::core_wrapper::getJNIEnv();
    if (!m_env)
      throw NoJNIEnvException();

    auto id = m_env->FindClass(name);
    if (!id)
      throw ClassNotFoundException();

    m_id = (jclass) m_env->NewGlobalRef((jobject)id);
    assert(m_id);
  }

  ~Class()
  {
    m_env->DeleteGlobalRef((jobject)m_id);
  }

  JNIEnv *getEnv() { return m_env; }
  jclass getID() { return m_id; }

  jmethodID getStaticMethodID(const char *name, const char *sig)
  {
    return getEnv()->GetStaticMethodID(getID(), name, sig);
  }

};


template <class T>
T &getSingleton()
{
  static T instance {getEnv()};
  return instance;
}


template <class T>
typename T::Class &getClass()
{
  using Class = typename T::Class;
  return getSingleton<Class>();
}


template <class T = java::Class>
class Object
{
  JNIEnv *m_env = nullptr;
  jobject m_id = 0;

public:
  using Class = T;

  Object &operator=(const &Object) = delete;
  Object(const &Object) = delete;
  Object(jobject id, JNIEnv *env = nullptr)
  {
    m_env = env;
//     if (!m_env)
//       m_env = il2ge::core_wrapper::getJNIEnv();
    if (!m_env)
      throw NoJNIEnvException();

    if (!id)
      throw NullObjectException();

    m_id = m_env->NewGlobalRef(id);
    assert(m_id);
  }

  ~Object()
  {
    m_env->DeleteGlobalRef(m_id);
  }

  jobject getID() { return m_id; }

  JNIEnv *getEnv() { return m_env; }

  static Class &getClass()
  {
    return java::getSingleton<Class>();
  }
};


} // namespace il2ge::java


#endif
