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

#include "jni_wrapper.h"
#include "meta_class_registrators.h"

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/il2.engine.LightPoint_definitions>

Interface import;

void JNICALL setPos(JNIEnv *env, jobject obj,
		jint arg0,
		jfloat arg1,
		jfloat arg2,
		jfloat arg3)
{
	return import.setPos(env, obj, arg0, arg1, arg2, arg3);
}

void JNICALL setColor(JNIEnv *env, jobject obj,
		jint arg0,
		jfloat arg1,
		jfloat arg2,
		jfloat arg3)
{
	return import.setColor(env, obj, arg0, arg1, arg2, arg3);
}

void JNICALL setEmit(JNIEnv *env, jobject obj,
		jint arg0,
		jfloat arg1,
		jfloat arg2)
{
	return import.setEmit(env, obj, arg0, arg1, arg2);
}

void JNICALL getPos(JNIEnv *env, jobject obj,
		jint arg0,
		jfloatArray arg1)
{
	return import.getPos(env, obj, arg0, arg1);
}

void JNICALL getColor(JNIEnv *env, jobject obj,
		jint arg0,
		jfloatArray arg1)
{
	return import.getColor(env, obj, arg0, arg1);
}

void JNICALL getEmit(JNIEnv *env, jobject obj,
		jint arg0,
		jfloatArray arg1)
{
	return import.getEmit(env, obj, arg0, arg1);
}

void JNICALL setOffset(JNIEnv *env, jobject obj,
		jfloat arg0,
		jfloat arg1,
		jfloat arg2)
{
	return import.setOffset(env, obj, arg0, arg1, arg2);
}

void JNICALL addToRender(JNIEnv *env, jobject obj,
		jint arg0)
{
// 	return import.addToRender(env, obj, arg0);
}

void JNICALL clearRender(JNIEnv *env, jobject obj)
{
// 	return import.clearRender(env, obj);
}

void JNICALL Finalize(JNIEnv *env, jobject obj,
		jint arg0)
{
	return import.Finalize(env, obj, arg0);
}

jint JNICALL New(JNIEnv *env, jobject obj)
{
	return import.New(env, obj);
//   retur 0x1;
}


} // namespace


#include <_generated/jni_wrapper/il2.engine.LightPoint_registration>
