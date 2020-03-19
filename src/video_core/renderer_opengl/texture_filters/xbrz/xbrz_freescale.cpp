// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

// adapted from
// https://github.com/libretro/glsl-shaders/blob/d7a8b8eb2a61a5732da4cbe2e0f9ad30600c3f17/xbrz/shaders/xbrz-freescale.glsl

// xBRZ freescale
// based on :
// 4xBRZ shader - Copyright (C) 2014-2016 DeSmuME team
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the this software.  If not, see <http://www.gnu.org/licenses/>.

// Hyllian's xBR-vertex code and texel mapping
// Copyright (C) 2011/2016 Hyllian - sergiogdb@gmail.com
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "video_core/renderer_opengl/gl_rasterizer_cache.h"
#include "video_core/renderer_opengl/texture_filters/xbrz/xbrz_freescale.h"

#include "shaders/xbrz_freescale.frag"
#include "shaders/xbrz_freescale.vert"

namespace OpenGL {

XbrzFreescale::XbrzFreescale(u16 scale_factor) : TextureFilterInterface(scale_factor) {
    const OpenGLState cur_state = OpenGLState::GetCurState();

    program.Create(xbrz_freescale_vert.data(), xbrz_freescale_frag.data());
    vao.Create();
    draw_fbo.Create();
    src_sampler.Create();

    state.draw.shader_program = program.handle;
    state.Apply();

    glSamplerParameteri(src_sampler.handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(src_sampler.handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(src_sampler.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(src_sampler.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(glGetUniformLocation(program.handle, "scale"), static_cast<GLfloat>(scale_factor));

    cur_state.Apply();
    state.draw.vertex_array = vao.handle;
    state.draw.shader_program = program.handle;
    state.draw.draw_framebuffer = draw_fbo.handle;
    state.texture_units[0].sampler = src_sampler.handle;
}

void XbrzFreescale::scale(CachedSurface& surface, const Common::Rectangle<u32>& rect,
                          std::size_t buffer_offset) {
    const OpenGLState cur_state = OpenGLState::GetCurState();

    OGLTexture src_tex;
    src_tex.Create();
    state.texture_units[0].texture_2d = src_tex.handle;

    state.viewport = RectToViewport(rect);
    state.Apply();

    const FormatTuple tuple = GetFormatTuple(surface.pixel_format);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(surface.stride));
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, tuple.internal_format, rect.GetWidth(), rect.GetHeight(), 0,
                 tuple.format, tuple.type, &surface.gl_buffer[buffer_offset]);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           cur_state.texture_units[0].texture_2d, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

    cur_state.Apply();
}

} // namespace OpenGL
