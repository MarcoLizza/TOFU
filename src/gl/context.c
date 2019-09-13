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

#include "context.h"

#include "surface.h"

#include "../log.h"

#include <memory.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

bool GL_context_initialize(GL_Context_t *context, size_t width, size_t height)
{
    void *vram = malloc(width * height * sizeof(GL_Color_t));
    if (!vram) {
        Log_write(LOG_LEVELS_ERROR, "<GL> can't allocate VRAM buffer");
        return false;
    }

    void **vram_rows = malloc(height * sizeof(void *));
    if (!vram_rows) {
        Log_write(LOG_LEVELS_ERROR, "<GL> can't allocate VRAM LUT");
        free(vram);
        return false;
    }
    for (size_t i = 0; i < height; ++i) {
        vram_rows[i] = (GL_Color_t *)vram + (width * i);
    }

    Log_write(LOG_LEVELS_DEBUG, "<GL> VRAM allocated at #%p (%dx%d)", vram, width, height);

    *context = (GL_Context_t){
            .width = width,
            .height = height,
            .stride = width * sizeof(GL_Color_t),
            .vram = vram,
            .vram_rows = vram_rows,
            .vram_size = width * height,
        };

    context->background = 0;
    for (size_t i = 0; i < GL_MAX_PALETTE_COLORS; ++i) {
        context->shifting[i] = i;
        context->transparent[i] = GL_BOOL_FALSE;
    }
    context->transparent[0] = GL_BOOL_TRUE;

    GL_palette_greyscale(&context->palette, GL_MAX_PALETTE_COLORS);
    Log_write(LOG_LEVELS_DEBUG, "<GL> calculating greyscale palette of #%d entries", GL_MAX_PALETTE_COLORS);

    return true;
}

void GL_context_terminate(GL_Context_t *context)
{
    free(context->vram);
    free(context->vram_rows);
    Log_write(LOG_LEVELS_DEBUG, "<GL> context deallocated");

    *context = (GL_Context_t){};
}

void GL_context_push(const GL_Context_t *context)
{
}

void GL_context_pop(const GL_Context_t *context)
{
}

void GL_context_clear(const GL_Context_t *context)
{
    const GL_Color_t color = context->palette.colors[context->background];
    GL_Color_t *dst = (GL_Color_t *)context->vram;
    for (size_t i = context->vram_size; i > 0; --i) {
        *(dst++) = color;
    }
}

void GL_context_screenshot(const GL_Context_t *context, const char *pathfile)
{
    int result = stbi_write_png(pathfile, context->width, context->height, sizeof(GL_Color_t), context->vram, context->stride);
    if (!result) {
        Log_write(LOG_LEVELS_WARNING, "<GL> can't save screenshot to '%s'", pathfile);
    }
}

// TODO: specifies `const` always? Is pedantic or useful?
// https://dev.to/fenbf/please-declare-your-variables-as-const
void GL_context_blit(const GL_Context_t *context, const GL_Surface_t *surface, GL_Rectangle_t tile, GL_Point_t position)
{
    const GL_Pixel_t *shifting = context->shifting;
    const GL_Bool_t *transparent = context->transparent;
    const GL_Color_t *colors = context->palette.colors;

    const size_t width = (size_t)tile.width;
    const size_t height = (size_t)tile.height;

    const size_t src_skip = surface->width - width;
    const size_t dst_skip = context->width - width;

    GL_Color_t *dst = (GL_Color_t *)context->vram_rows[position.y] + position.x;
    const GL_Pixel_t *src = (const GL_Pixel_t *)surface->data_rows[tile.y] + tile.x;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            GL_Pixel_t index = shifting[*(src++)];
#if 1
            if (transparent[index]) {
                dst++;
            } else {
                *(dst++) = colors[index];
            }
#else
            *dst = transparent[index] ? *dst : colors[index];
            dst++;
#endif
        }
        src += src_skip;
        dst += dst_skip;
    }
}

// Simple implementation of nearest-neighbour scaling.
// See `http://tech-algorithm.com/articles/nearest-neighbor-image-scaling/` for a reference code.
void GL_context_blit_s(const GL_Context_t *context, const GL_Surface_t *surface, GL_Rectangle_t tile, GL_Point_t position, float sx, float sy)
{
    const GL_Pixel_t *shifting = context->shifting;
    const GL_Bool_t *transparent = context->transparent;
    const GL_Color_t *colors = context->palette.colors;

    const size_t width = (size_t)(sx * (float)tile.width); // To avoid empty pixels we scan the destination area and calculate the source pixel.
    const size_t height = (size_t)(sy * (float)tile.height);

    const float du = 1.0f / sx;
    const float dv = 1.0f / sy;

    const size_t skip = context->width - width;

    GL_Color_t *dst = (GL_Color_t *)context->vram_rows[position.y] + position.x;

    float v = (float)tile.y;
    for (size_t i = 0; i < height; ++i) {
        const GL_Pixel_t *src = (const GL_Pixel_t *)surface->data_rows[(int)v];

        float u = (float)tile.x;
        for (size_t j = 0; j < width; ++j) {
            GL_Pixel_t index = shifting[src[(int)u]];
#if 1
            if (transparent[index]) {
                dst++;
            } else {
                *(dst++) = colors[index];
            }
#else
            *dst = transparent[index] ? *dst : colors[index];
            dst++;
#endif
            u += du;
        }

        v += dv;
        dst += skip;
    }
}

void GL_context_blit_r(const GL_Context_t *context, const GL_Surface_t *surface, GL_Rectangle_t tile, GL_Point_t position, float rotation)
{
    const GL_Pixel_t *shifting = context->shifting;
    const GL_Bool_t *transparent = context->transparent;
    const GL_Color_t *colors = context->palette.colors;

    const size_t width = tile.width; // To avoid empty pixels we scan the destination area and calculate the source pixel.
    const size_t height = tile.height;

    const size_t hw = width / 2;
    const size_t hh = height / 2;

    const float c = cosf(rotation);
    const float s = sinf(rotation);

    // The counter-clockwise 2D rotation matrix is
    //
    //      |  c  -s |
    //  R = |        |
    //      |  s   c |
    //
    // In order to calculate the clockwise rotation matrix one can use the
    // similarities `cos(-a) = cos(a)` and `sin(-a) = -sin(a)` and get
    //
    //      |  c   s |
    //  R = |        |
    //      | -s   c |
    //
    const size_t skip = context->width - width;

    GL_Color_t *dst = (GL_Color_t *)context->vram_rows[position.y - hh] + position.x - hw;

    float y = -(float)hh;
    for (size_t i = 0; i < height; ++i) {

        float x = -(float)hw;
        for (size_t j = 0; j < width; ++j) {

            float u = (x * c + y * s) + hw;
            float v = (y * c - x * s) + hh;

            int sx = (int)((float)tile.x + u);
            int sy = (int)((float)tile.y + v);

            if (sx >= tile.x && sy >= tile.y && sx < (int)(tile.x + tile.width) && sy < (int)(tile.y + tile.height)) {
                const GL_Pixel_t *src = (const GL_Pixel_t *)surface->data_rows[sy];
                GL_Pixel_t index = shifting[src[sx]];

                if (!transparent[index]) {
                    *dst = colors[index];
                }
            }

            x += 1.0f;
            dst++;
        }

        y += 1.0f;
        dst += skip;
    }
}

// https://www.lexaloffle.com/bbs/?pid=52525
// https://github.com/morgan3d/misc/tree/master/p8particle
// https://web.archive.org/web/20190305223938/http://www.drdobbs.com/architecture-and-design/fast-bitmap-rotation-and-scaling/184416337
void GL_context_blit_sr(const GL_Context_t *context, const GL_Surface_t *surface, GL_Rectangle_t tile, GL_Point_t position, float sx, float sy, float rotation)
{
    GL_context_blit(context, surface, tile, position);
}

void GL_context_palette(GL_Context_t *context, const GL_Palette_t *palette)
{
    context->palette = *palette;
    Log_write(LOG_LEVELS_DEBUG, "<GL> palette updated");
}

void GL_context_shifting(GL_Context_t *context, const size_t *from, const size_t *to, size_t count)
{
    if (from == NULL) {
        for (size_t i = 0; i < GL_MAX_PALETTE_COLORS; ++i) {
            context->shifting[i] = i;
        }
    } else {
        for (size_t i = 0; i < count; ++i) {
            context->shifting[from[i]] = to[i];
        }
    }
}

void GL_context_transparent(GL_Context_t *context, const GL_Pixel_t *indexes, const GL_Bool_t *transparent, size_t count)
{
    if (indexes == NULL) {
        for (size_t i = 0; i < GL_MAX_PALETTE_COLORS; ++i) {
            context->transparent[i] = GL_BOOL_FALSE;
        }
        context->transparent[0] = GL_BOOL_TRUE;
    } else {
        for (size_t i = 0; i < count; ++i) {
            context->transparent[indexes[i]] = transparent[i];
        }
    }
}

void GL_context_background(GL_Context_t *context, const GL_Pixel_t index)
{
    if (index >= context->palette.count) {
        Log_write(LOG_LEVELS_WARNING, "<GL> color index #%d not available in current palette", index);
        return;
    }
    context->background = index;
}
