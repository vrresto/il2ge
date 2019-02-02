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


template <typename ReturnType, typename ... ArgTypes>
struct ClassMethodCaller {};


template <typename ... ArgTypes>
struct ClassMethodCaller<jobject, ArgTypes...>
{
  static jobject call(JNIEnv *env, jclass java_class, jmethodID id, ArgTypes... args)
  {
    return env->CallStaticObjectMethod(java_class, id, args...);
  }
};


template <typename ... ArgTypes>
struct ClassMethodCaller<int, ArgTypes...>
{
  static int call(JNIEnv *env, jclass java_class, jmethodID id, ArgTypes... args)
  {
    return env->CallStaticIntMethod(java_class, id, args...);
  }
};


template <typename ReturnType, typename  ... ArgTypes>
class ClassMethod
{
public:
  JNIEnv *m_env = nullptr;
  jclass m_class = 0;
  jmethodID m_id = 0;

  ClassMethod(JNIEnv *env, jclass java_class, jmethodID id)
  {
    m_env = env;
    m_class = java_class;
    m_id = id;
  }

  ReturnType call(ArgTypes... args)
  {
    return ClassMethodCaller<ReturnType, ArgTypes...>::call(m_env, m_class, m_id, args...);
  }

};


template <typename ReturnType, typename  ... ArgTypes>
class ObjectMethodCaller {};


template<typename ... ArgTypes>
class ObjectMethodCaller<int, ArgTypes...>
{
  static int call(JNIEnv *env, jobject *obj, jmethodID id, ArgTypes... args)
  {
    return env->CallIntMethod(obj, id, args...);
  }
};

template<typename ... ArgTypes>
class ObjectMethodCaller<jobject, ArgTypes...>
{
  static jobject call(JNIEnv *env, jobject *obj, jmethodID id, ArgTypes... args)
  {
    return env->CallObjectMethod(obj, id, args...);
  }
};


template <typename ReturnType, typename  ... ArgTypes>
class ObjectMethod
{
public:
  JNIEnv *m_env = nullptr;
  jobject m_java_object = 0;
  jmethodID m_id = 0;

  ObjectMethod(JNIEnv *env, jobject java_object, jmethodID id)
  {
    m_env = env;
    m_java_object = java_object;
    m_id = id;
  }

  ReturnType call(ArgTypes... args)
  {
    return ObjectMethodCaller<ReturnType, ArgTypes...>::call(m_env, m_java_object, m_id, args...);
  }

};


inline void getObjectField(JNIEnv *env, jobject obj, jfieldID id, jobject &value)
{
  value = env->GetObjectField(obj, id);
}

inline void getObjectField(JNIEnv *env, jobject obj, jfieldID id, int &value)
{
  value = env->GetIntField(obj, id);
}


template <typename T>
class ObjectField
{
  JNIEnv *m_env = nullptr;
  jobject m_obj = 0;
  jfieldID m_id = 0;

public:
  ObjectField(JNIEnv *env, jobject obj, jfieldID id) : m_env(env), m_obj(obj), m_id(id) {}

  T get()
  {
    T value;
    getObjectField(m_env, m_obj, m_id, value);
    return value;
  }
};


inline void getClassField(JNIEnv *env, jclass java_class, jfieldID id, jobject &value)
{
  value = env->GetStaticObjectField(java_class, id);
}


template <typename T>
class ClassField
{
  JNIEnv *m_env = nullptr;
  jclass m_class = 0;
  jfieldID m_id = 0;

public:
  ClassField(JNIEnv *env, jclass java_class, jfieldID id) :
    m_env(env), m_class(java_class), m_id(id) {}

  T get()
  {
    T value;
    getClassField(m_env, m_class, m_id, value);
    return value;
  }
};



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
//     if (!m_env)
//       m_env = (JNIEnv*) il2ge::core_wrapper::getJNIEnv();
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
    if (m_id)
      m_env->DeleteGlobalRef((jobject)m_id);
  }

  JNIEnv *getEnv() { return m_env; }
  jclass getID() { return m_id; }

  template <typename ReturnType, typename  ... ArgTypes>
  ClassMethod<ReturnType, ArgTypes...> getMethod(const char *name, const char *signature)
  {
    auto id = m_env->GetStaticMethodID(m_id, name, signature);
    if (!id)
      throw MethodNotFoundException(name);

    return ClassMethod<ReturnType, ArgTypes...>(m_env, m_id, id);
  }

  template <typename T>
  ClassField<T> getField(const char *name, const char *signature)
  {
    auto id = m_env->GetStaticFieldID(m_id, name, signature);
    if (!id)
      throw FieldNotFoundException();
    return ClassField<T>(m_env, m_id, id);
  }

};


class Object
{
  JNIEnv *m_env = nullptr;
  jclass m_class = 0;
  jobject m_id = 0;

public:
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

    m_class = (jclass) m_env->NewGlobalRef((jobject) m_env->GetObjectClass(m_id));
    assert(m_class);
  }

  ~Object()
  {
    if (m_id)
      m_env->DeleteGlobalRef(m_id);
    if (m_class)
      m_env->DeleteGlobalRef(m_class);
  }

  jobject getID() { return m_id; }

  JNIEnv *getEnv() { return m_env; }

  template <typename ReturnType, typename  ... ArgTypes>
  ObjectMethod<ReturnType, ArgTypes...> getMethod(const char *name, const char *signature)
  {
    auto id = m_env->GetMethodID(m_class, name, signature);
    if (!id)
      throw MethodNotFoundException();

    return ObjectMethod<ReturnType, ArgTypes...>(m_env, m_env, m_id, id);
  }

  template <typename T>
  ObjectField<T> getField(const char *name, const char *signature)
  {
    auto id = m_env->GetFieldID(m_class, name, signature);
    if (!id)
      throw FieldNotFoundException();
    return ObjectField<T>(m_env, m_id, id);
  }

};


} // namespace il2ge::java_util


#endif
