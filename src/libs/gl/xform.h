/*
 * Copyright (c) 2019-2020 by Marco Lizza (marco.lizza@gmail.com)
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

#ifndef __GL_XFORM_H__
#define __GL_XFORM_H__

#include "common.h"

typedef enum _GL_XForm_Registers_t {
    GL_XForm_Registers_t_First,
    GL_XFORM_REGISTER_H = GL_XForm_Registers_t_First,
    GL_XFORM_REGISTER_V,
    GL_XFORM_REGISTER_A,
    GL_XFORM_REGISTER_B,
    GL_XFORM_REGISTER_C,
    GL_XFORM_REGISTER_D,
    GL_XFORM_REGISTER_X,
    GL_XFORM_REGISTER_Y,
    GL_XForm_Registers_t_Last = GL_XFORM_REGISTER_Y,
    GL_XForm_Registers_t_CountOf
} GL_XForm_Registers_t;

typedef struct _GL_XForm_State_Operation_t {
    GL_XForm_Registers_t id;
    float value;
} GL_XForm_State_Operation_t;

typedef struct _GL_XForm_Table_Entry_t {
    int scan_line;
    GL_XForm_State_Operation_t operations[GL_XForm_Registers_t_CountOf]; // At most, change all the registries.
    size_t count;
} GL_XForm_Table_Entry_t;

typedef enum _GL_XForm_Clamps_t {
    GL_XFORM_CLAMP_EDGE,
    GL_XFORM_CLAMP_BORDER,
    GL_XFORM_CLAMP_REPEAT
} GL_XForm_Clamps_t;

typedef struct _GL_XForm_t {
    float registers[GL_XForm_Registers_t_CountOf];
    GL_XForm_Clamps_t clamp;
    GL_XForm_Table_Entry_t *table;
} GL_XForm_t;

// TODO: add helper functions to generate common transformations?

#endif  /* __GL_XFORM_H__ */