#include "jni_wrapper.h"
#include "meta_class_registrators.h"
#include <core.h>

#include <cassert>

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/rts.KeyboardDX_definitions>

Interface import;

jboolean JNICALL nGetMsg(JNIEnv *env, jobject obj,
		jintArray arg0)
{
	auto ret = import.nGetMsg(env, obj, arg0);
  return core::isMenuShown() ? false : ret;
}


} // namespace


#include <_generated/jni_wrapper/rts.KeyboardDX_registration>
