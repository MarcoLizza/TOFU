/*
 * Copyright (c) 2019 Marco Lizza (marco.lizza@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#ifndef __GL_FONT_H__
#define __GL_FONT_H__

#include <stdbool.h>

#include "common.h"
#include "texture.h"

#define GL_MAX_FONT_ALPHABET    256

typedef struct _GL_Font_t {
    GL_Texture_t atlas;
    GLuint glyph_width, glyph_height;
    GLchar alphabet[GL_MAX_FONT_ALPHABET];
} GL_Font_t;

// TODO: rename to `GL_font_load()` and add proper `GL_font_create()` from memory (also for textures)
extern bool GL_font_create(GL_Font_t *font, const char *pathfile, GLuint glyph_width, GLuint glyph_height, const char *alphabet);
extern void GL_font_delete(GL_Font_t *font);
extern GL_Rectangle_t GL_font_measure(const GL_Font_t *font, const char *text, const GLfloat size);
extern void GL_font_write(const GL_Font_t *font, const char *text, const GL_Point_t position, const GLfloat size, const GL_Color_t color);

#endif  /* __GL_FONT_H__ */