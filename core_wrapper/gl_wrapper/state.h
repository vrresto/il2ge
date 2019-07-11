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


#include <render_util/gl_binding/gl_functions.h>

#include <array>

#ifndef IL2GE_CORE_GL_WRAPPER_STATE_H
#define IL2GE_CORE_GL_WRAPPER_STATE_H


namespace core_gl_wrapper
{


struct State
{
  struct AttributeIndex
  {
    enum Enum : unsigned int
    {
      FRONT_FACE = 0,
      CULL_FACE ,
      DEPTH_FUNC,
      BLEND_SRC,
      BLEND_DST,
      DEPTH_MASK,
      MAX,
    };
  };

  struct EnableIndex
  {
    enum Enum : unsigned int
    {
      CULL_FACE = 0,
      BLEND,
      DEPTH_TEST,
      STENCIL_TEST,
      MAX,
    };
  };

  static GLenum getEnableNameFromIndex(unsigned int index)
  {
    switch (index)
    {
      case EnableIndex::CULL_FACE:
        return GL_CULL_FACE;
      case EnableIndex::BLEND:
        return GL_BLEND;
      case EnableIndex::DEPTH_TEST:
        return GL_DEPTH_TEST;
      case EnableIndex::STENCIL_TEST:
        return GL_STENCIL_TEST;
      default:
        assert(0);
        abort();
    }
  }

  static GLenum getAttributeNameFromIndex(unsigned int index)
  {
    switch (index)
    {
      case AttributeIndex::FRONT_FACE:
        return GL_FRONT_FACE;
      case AttributeIndex::CULL_FACE:
        return GL_CULL_FACE_MODE;
      case AttributeIndex::DEPTH_FUNC:
        return GL_DEPTH_FUNC;
      case AttributeIndex::DEPTH_MASK:
        return GL_DEPTH_WRITEMASK;
      case AttributeIndex::BLEND_SRC:
        return GL_BLEND_SRC;
      case AttributeIndex::BLEND_DST:
        return GL_BLEND_DST;
      default:
        assert(0);
        abort();
    }
  }

  static const State &defaults();
  static State fromCurrent();

  std::array<int, AttributeIndex::MAX> attributes;
  std::array<bool, EnableIndex::MAX> enables;

private:
  State() {}
};


struct StateModifier
{
  using AttributeIndex = State::AttributeIndex;
  using EnableIndex = State::EnableIndex;

  StateModifier(const State &original_state) :
    original_state(original_state),
    current_state(original_state)
  {
  }

  ~StateModifier();

  void setDefaults();

  void setFrontFace(GLenum value)
  {
    setAttribute<AttributeIndex::FRONT_FACE>(value);
  }

  void setCullFace(GLenum value)
  {
    setAttribute<AttributeIndex::CULL_FACE>(value);
  }

  void setDepthFunc(GLenum value)
  {
    setAttribute<AttributeIndex::DEPTH_FUNC>(value);
  }

  void setDepthMask(bool value)
  {
    setAttribute<AttributeIndex::DEPTH_MASK>(value);
  }

  void enableCullFace(bool value)
  {
    enable(EnableIndex::CULL_FACE, value);
  }

  void enableBlend(bool value)
  {
    enable(EnableIndex::BLEND, value);
  }

  void enableDepthTest(bool value)
  {
    enable(EnableIndex::DEPTH_TEST, value);
  }

  void enableStencilTest(bool value)
  {
    enable(EnableIndex::STENCIL_TEST, value);
  }

private:
  template <AttributeIndex::Enum T>
  struct Attribute
  {
  };

  template <AttributeIndex::Enum Index, typename T>
  void setAttribute(T value)
  {
    if (current_state.attributes.at(Index) != value)
    {
      current_state.attributes.at(Index) = value;
      Attribute<Index>::set(value);
    }
  }

  template<AttributeIndex::Enum T>
  void restoreAttribute()
  {
    auto orig = original_state.attributes.at(T);
    auto curr = current_state.attributes.at(T);

    if (curr != orig)
    {
      Attribute<T>::set(orig);
    }
  }

  void enable(EnableIndex::Enum index, bool value)
  {
    using namespace render_util::gl_binding;

    if (current_state.enables.at(index) != value)
    {
      current_state.enables.at(index) = value;
      auto name = State::getEnableNameFromIndex(index);

      if (value)
        gl::Enable(name);
      else
        gl::Disable(name);
    }
  }

  void restoreEnable(EnableIndex::Enum index)
  {
    using namespace render_util::gl_binding;

    if (original_state.enables.at(index) != current_state.enables.at(index))
    {
      auto value = original_state.enables.at(index);
      auto name = State::getEnableNameFromIndex(index);

      if (value)
        gl::Enable(name);
      else
        gl::Disable(name);
    }
  }

  const State &original_state;
  State current_state;
};



template <>
struct StateModifier::Attribute<StateModifier::AttributeIndex::FRONT_FACE>
{
  static constexpr auto &set = render_util::gl_binding::gl::FrontFace;
};

template <>
struct StateModifier::Attribute<StateModifier::AttributeIndex::CULL_FACE>
{
  static constexpr auto &set = render_util::gl_binding::gl::CullFace;
};

template <>
struct StateModifier::Attribute<StateModifier::AttributeIndex::DEPTH_FUNC>
{
  static constexpr auto &set = render_util::gl_binding::gl::DepthFunc;
};

template <>
struct StateModifier::Attribute<StateModifier::AttributeIndex::DEPTH_MASK>
{
  static constexpr auto &set = render_util::gl_binding::gl::DepthMask;
};


}

#endif
