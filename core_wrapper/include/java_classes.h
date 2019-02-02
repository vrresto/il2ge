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

#ifndef IL2GE_CORE_WRAPPER_JAVA_CLASSES
#define IL2GE_CORE_WRAPPER_JAVA_CLASSES

#include <java_util.h>


namespace il2ge::java
{


// namespace il2::game
// {
//   class Main : public Class
//   {
//     ClassMethod<jobject> method_state;
//   };
// }

namespace il2::engine
{

  class Renders : public Object
  {
  public:
    class Class;
  };

  class Renders::Class : public java::Class
  {
//     ClassField<jobject> m_currentRender =
//         getField<jobject>("currentRender", "Lcom/maddox/il2/engine/Render;");

    jfieldID m_currentRender_id =
        getEnv()->GetStaticFieldID(getID(), "currentRender", "Lcom/maddox/il2/engine/Render;");

  public:
    Class(JNIEnv *env) : java::Class("com/maddox/il2/engine/Renders", env)
    {
      assert(m_currentRender_id);
    }

    jobject current()
    {
//       return field_currentRender.get();
      return getEnv()->GetStaticObjectField(getID(), m_currentRender_id);
    }
  };


  class Actor : public Object
  {
//     ObjectField<jobject> field_name = getField<jobject>("name", "Ljava/lang/String;");
  public:
    class Class;
//     Actor(jobject id, JNIEnv *env) : Object(id, env) {}
//     std::string getName() {  return std::move(javaStringToStd(getEnv(), field_name.get())); }
  };


  class Actor::Class : public java::Class
  {
    jmethodID m_getByName_id =
        getEnv()->GetStaticMethodID(getID(),
                                    "getByName",
                                    "(Ljava/lang/String;)Lcom/maddox/il2/engine/Actor;");
//     ClassMethod<jobject, jstring> m_getByName =
//         getMethod<jobject, jstring>("getByName",
//                                     "(Ljava/lang/String;)Lcom/maddox/il2/engine/Actor;");

  public:
    Class(JNIEnv *env) : java::Class("com/maddox/il2/engine/Actor", env) {}

    jobject getByName(const std::string &name)
    {
      return getEnv()->CallStaticObjectMethod(getID(),
                                              m_getByName_id,
                                              getEnv()->NewStringUTF(name.c_str()));
//       return m_getByName.call(getEnv()->NewStringUTF(name.c_str()));
    }

  };


//   class RendersMain : public Class
//   {
// //     ClassMethod<jobject> method_glContext =
// //         getMethod<jobject>("glContext", "()Lcom/maddox/opengl/GLContext;");
//     ClassMethod<jobject, jint> method_get =
//         getMethod<jobject, jint>("get", "(I)Lcom/maddox/il2/engine/Render;");
//
//   public:
//     RendersMain(JNIEnv *env) : Class("com/maddox/il2/engine/RendersMain", env) {}
//
//     jobject get(jint i)
//     {
//       return method_get.call(i);
//     }
//
// //     java_object::opengl::GLContext glContext()
// //     {
// //       return java_object::opengl::GLContext(method_glContext());
// //     }
//   };

}



// namespace il2::game
// {
//   class GameState : public Object<java_class::il2::game::GameState>
//   {
//   public:
//     int id();
//   };
// }

// namespace opengl
// {
//   class GLContext : public Object
//   {
//     ObjectField<int> field_iRC;
//
//   public:
//     GLContext(jobject id) : Object(id),
//       field_iRC(getField<int>("iRC", "I"))
//     {
//     }
//
//     int getHandle() { return field_iRC.get(); }
//   };
// }


} // namespace il2ge::java


#endif
