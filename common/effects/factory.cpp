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

#include "factory.h"
#include <render_util/gl_binding/gl_functions.h>

#include <iostream>

using namespace il2ge;
using namespace std;
using Factory = Effect3DParametersFactory;
using namespace render_util::gl_binding;


namespace
{


unordered_map<string, const Factory*> g_factories;


class DummyEffect : public il2ge::Effect3D
{
public:
  DummyEffect(const il2ge::Effect3DParameters &params) : Effect3D(params) {}

  void render() override
  {
    gl::Color4f(1, 0, 0, 0.5);
    gl::Begin(GL_POINTS);
    gl::Vertex3f(getPos().x, getPos().y, getPos().z);
    gl::End();
  }
};


class DummyParameters : public Effect3DParameters
{
  const char *getJavaClassName() const override
  {
    return "com/maddox/il2/engine/EffParticles";
  }

  std::unique_ptr<il2ge::Effect3D> createEffect() const override
  {
    return std::make_unique<DummyEffect>(*this);
  }
};


void registerClass(const char *name, const Factory *factory)
{
  g_factories[name] = factory;
}


void init()
{
  static bool initialized = false;
  if (initialized)
    return;

  registerClass("TParticlesSystemParams", create_TParticlesSystemParams);

  initialized = true;
}


} // namespace


namespace il2ge
{


std::unique_ptr<Effect3DParameters> createEffect3DParameters(const std::string &class_name)
{
  init();

  auto factory = g_factories[class_name];

  if (factory)
    return std::move(factory());
  else
    return std::make_unique<DummyParameters>();
}


} // namespace il2ge
