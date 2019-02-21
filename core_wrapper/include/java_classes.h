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


namespace il2::engine
{

  class ClassRenders : public java::Class
  {
    jfieldID m_currentRender_id =
        getEnv()->GetStaticFieldID(getID(), "currentRender", "Lcom/maddox/il2/engine/Render;");

  public:
    ClassRenders(JNIEnv *env) : java::Class("com/maddox/il2/engine/Renders", env)
    {
      assert(m_currentRender_id);
    }

    jobject current()
    {
      return getEnv()->GetStaticObjectField(getID(), m_currentRender_id);
    }
  };


  class Renders : public Object<ClassRenders> {};


  class ClassActor : public java::Class
  {
    friend class Actor;

    jfieldID m_name_id =
      getEnv()->GetFieldID(getID(), "name", "Ljava/lang/String;");
    jmethodID m_getByName_id =
      getStaticMethodID("getByName", "(Ljava/lang/String;)Lcom/maddox/il2/engine/Actor;");

  public:
    ClassActor(JNIEnv *env) : java::Class("com/maddox/il2/engine/Actor", env) {}

    jobject getByName(const std::string &name)
    {
      return getEnv()->CallStaticObjectMethod(getID(),
                                              m_getByName_id,
                                              getEnv()->NewStringUTF(name.c_str()));
    }

  };

  class Actor : public Object<ClassActor>
  {
  public:
    Actor(jobject id, JNIEnv *env = nullptr) : Object(id, env) {}

    std::string getName()
    {
      auto name = getEnv()->GetObjectField(getID(), getClass().m_name_id);
      return std::move(javaStringToStd(getEnv(), name));
    }
  };



//   class RendersMain : public Class
//   {
//     ClassMethod<jobject> method_glContext =
//         getMethod<jobject>("glContext", "()Lcom/maddox/opengl/GLContext;");
//
//   public:
//     RendersMain(JNIEnv *env) : Class("com/maddox/il2/engine/RendersMain", env) {}
//
//     java_object::opengl::GLContext glContext()
//     {
//       return java_object::opengl::GLContext(method_glContext());
//     }
//   };

}


// namespace il2::game
// {
//   class Main : public Class
//   {
//     ClassMethod<jobject> method_state;
//   };
// }

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
