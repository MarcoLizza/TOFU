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

#include "graphics.h"

#include "../config.h"
#include "../environment.h"
#include "../log.h"
#include "graphics/palettes.h"

#include <raylib/raylib.h>
#include <math.h>
#include <string.h>

#ifdef __EXPLICIT_SIGNUM__
static inline float fsgnf(float value)
{
    return (value < 0.0f) ? -1.0f : ((value > 0.0f) ? 1.0f : value); // On -0.0, +NaN, -NaN, it returns -0.0, +NaN, -NaN
}
#else
static inline float fsgnf(float value)
{
    return (float)((0.0f < value) - (value < 0.0f)); // No cache miss due to branches.
}
#endif

#define DEFAULT_FONT_SIZE   10

const char graphics_wren[] =
    "foreign class Bank {\n"
    "\n"
    "    construct new(file, cell_width, cell_height) {}\n"
    "\n"
    "    sprite(id, x, y) {\n"
    "        sprite(id, x, y, 0.0)\n"
    "    }\n"
    "    sprite(id, x, y, r) {\n"
    "        sprite(id, x, y, r, 1.0, 1.0)\n"
    "    }\n"
    "    foreign sprite(id, x, y, r, sx, sy)\n"
    "\n"
    "}\n"
    "\n"
    "foreign class Font {\n"
    "\n"
    "    construct new(file) {}\n"
    "\n"
    "    static default { Font.new(\"default\") }\n"
    "\n"
    "    foreign text(text, x, y, color, size, align)\n"
    "\n"
    "}\n"
    "\n"
    "foreign class Canvas {\n"
    "\n"
    "    foreign static width\n"
    "    foreign static height\n"
    "    foreign static palette(colors)\n"
    "\n"
    "    foreign static point(x, y, color)\n"
    "    foreign static polygon(mode, vertices, color)\n"
    "    foreign static circle(mode, x, y, radius, color)\n"
    "\n"
    "    static line(x0, y0, x1, y1, color) {\n"
    "        polygon(\"line\", [ x0, y0, x1, y1 ], color)\n"
    "    }\n"
    "    static triangle(mode, x0, y0, x1, y1, x2, y2, color) {\n"
    "        polygon(mode, [ x0, y0, x1, y1, x2, y2, x0, y0 ], color)\n"
    "    }\n"
    "    static rectangle(mode, x, y, width, height, color) {\n"
    "        var offset = mode == \"line\" ? 1 : 0\n"
    "        var left = x\n"
    "        var top = y\n"
    "        var right = left + width - offset\n"
    "        var bottom = top + height - offset\n"
    "        polygon(mode, [ left, top, left, bottom, right, bottom, right, top, left, top ], color)\n"
    "    }\n"
    "    static square(mode, x, y, size, color) {\n"
    "        rectangle(mode, x, y, size, size, color)\n"
    "    }\n"
    "}\n"
    "\n"
;

void graphics_bank_allocate(WrenVM *vm)
{
    Environment_t *environment = (Environment_t *)wrenGetUserData(vm);

    const char *file = wrenGetSlotString(vm, 1);
    int cell_width = (int)wrenGetSlotDouble(vm, 2);
    int cell_height = (int)wrenGetSlotDouble(vm, 3);
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Bank.new() -> %s, %d, %d", file, cell_width, cell_height);
#endif

    Bank_t *bank = (Bank_t *)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Bank_t)); // `0, 0` since we are in the allocate callback.

    char pathfile[PATH_FILE_MAX] = {};
    strcpy(pathfile, environment->base_path);
    strcat(pathfile, file + 2);

    *bank = load_bank(pathfile, cell_width, cell_height, environment->display->palette, MAX_PALETTE_COLORS);
}

void graphics_bank_finalize(void *data)
{
    Bank_t *bank = (Bank_t *)data;
    unload_bank(bank);
}

void graphics_bank_sprite(WrenVM *vm)
{
    int sprite_id = (int)wrenGetSlotDouble(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    double rotation = wrenGetSlotDouble(vm, 4);
    double scale_x = wrenGetSlotDouble(vm, 5);
    double scale_y = wrenGetSlotDouble(vm, 6);
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Bank.sprite() -> %d, %d, %d, %.3f, %.3f, %.3f", sprite_id, x, y, rotation, scale_x, scale_y);
#endif

    const Bank_t *bank = (const Bank_t *)wrenGetSlotForeign(vm, 0);

    if (!bank->loaded) {
        Log_write(LOG_ERROR, "[TOFU] Bank now loaded, can't draw sprite");
        return;
    }

    int bank_position = sprite_id * bank->cell_width;
    int bank_x = bank_position % bank->atlas.width;
    int bank_y = (bank_position / bank->atlas.width) * bank->cell_height;

    Rectangle sourceRec = { (float)bank_x, (float)bank_y, (float)bank->cell_width * fsgnf(scale_x), (float)bank->cell_height * fsgnf(scale_y) };
    Rectangle destRec = { x, y, (float)bank->cell_width * fabsf(scale_x), (float)bank->cell_height * fabsf(scale_y) };
    Vector2 origin = { bank->cell_width * 0.5f, bank->cell_height * 0.5f}; // Rotate along center.

    DrawTexturePro(bank->atlas, sourceRec, destRec, origin, (float)rotation, (Color){ 255, 255, 255, 255 });
}

void graphics_font_allocate(WrenVM *vm)
{
    Environment_t *environment = (Environment_t *)wrenGetUserData(vm);

    const char *file = wrenGetSlotString(vm, 1);
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Font.new() -> %s", file);
#endif

    Font_t *font = (Font_t *)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Font_t)); // `0, 0` since we are in the allocate callback.

    if (strcmp(file, "default") == 0) {
        *font = (Font_t){
                .loaded = true,
                .is_default = true,
                .font = GetFontDefault()
            };
        return;
    }

    char pathfile[PATH_FILE_MAX] = {};
    strcpy(pathfile, environment->base_path);
    strcat(pathfile, file + 2);

    *font = load_font(pathfile);
}

void graphics_font_finalize(void *data)
{
    Font_t *font = (Font_t *)data;
    if (!font->is_default) {
        unload_font(font);
    }
}

void graphics_font_text(WrenVM *vm) // foreign text(text, color, size, align)
{
    const char *text = wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    int color = (int)wrenGetSlotDouble(vm, 4);
    int size = (int)wrenGetSlotDouble(vm, 5);
    const char *align = wrenGetSlotString(vm, 6);
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Font.text() -> %s, %d, %d, %d, %d, %s", text, x, y, color, size, align);
#endif

    const Font_t *font = (const Font_t *)wrenGetSlotForeign(vm, 0);

    int width = MeasureText(text, size);

    int dx = x, dy = y;
    if (strcmp(align, "left") == 0) {
        dx = x;
        dy = y;
    } else
    if (strcmp(align, "center") == 0) {
        dx = x - (width / 2);
        dy = y;
    } else
    if (strcmp(align, "right") == 0) {
        dx = x - width;
        dy = y;
    }
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Canvas.text() -> %d, %d, %d", width, dx, dy);
#endif

    if (!font->loaded) {
        return;
    }

    // Spacing is proportional to default font size.
    if (size < DEFAULT_FONT_SIZE) {
        size = DEFAULT_FONT_SIZE;
    }
    int spacing = size / DEFAULT_FONT_SIZE;

    DrawTextEx(font->font, text, (Vector2){ dx, dy }, size, (float)spacing, (Color){ color, color, color, 255 });
}

void graphics_canvas_width(WrenVM *vm)
{
    Environment_t *environment = (Environment_t *)wrenGetUserData(vm);

    wrenSetSlotDouble(vm, 0, environment->display->configuration.width);
}

void graphics_canvas_height(WrenVM *vm)
{
    Environment_t *environment = (Environment_t *)wrenGetUserData(vm);

    wrenSetSlotDouble(vm, 0, environment->display->configuration.height);
}

void graphics_canvas_palette(WrenVM *vm)
{
    Environment_t *environment = (Environment_t *)wrenGetUserData(vm);

    WrenType type = wrenGetSlotType(vm, 1);

    Color colors[MAX_PALETTE_COLORS];
    int count = 0;

    if (type == WREN_TYPE_STRING) { // Predefined palette!
        const char *id = wrenGetSlotString(vm, 1);
        const Palette_t *palette = graphics_palettes_find(id);
        if (palette != NULL) {
            count = palette->count;
            memcpy(colors, palette->colors, sizeof(Color) * count);

            Log_write(LOG_LEVELS_DEBUG, "[TOFU] Setting predefined palette '%s' w/ %d color(s)", id, count);
        } else {
            Log_write(LOG_LEVELS_WARNING, "[TOFU] Unknown predefined palette w/ id '%s'", id);
        }
    } else
    if (type == WREN_TYPE_LIST) { // User supplied palette.
        count = wrenGetListCount(vm, 1);
        Log_write(LOG_LEVELS_DEBUG, "Setting custom palette of #%d color(s)", count);

        if (count > MAX_PALETTE_COLORS) {
            Log_write(LOG_LEVELS_WARNING, "[TOFU] Palette has too many colors (%d) - clamping!", count);
            count = MAX_PALETTE_COLORS;
        }

        int slots = wrenGetSlotCount(vm);
#ifdef DEBUG
        Log_write(LOG_LEVELS_DEBUG, "Currently #%d slot(s) available, asking for additional slot", slots);
#endif
        const int aux_slot_id = slots;
        wrenEnsureSlots(vm, aux_slot_id + 1); // Ask for an additional temporary slot.

#ifdef DEBUG
        Log_write(LOG_LEVELS_DEBUG, "Canvas.palette() -> %d", count);
#endif
        for (int i = 0; i < count; ++i) {
            wrenGetListElement(vm, 1, i, aux_slot_id);

            Color *color = &colors[i];
            const char *argb = wrenGetSlotString(vm, aux_slot_id);
            char hex[3] = {};
            strncpy(hex, argb    , 2); color->a = strtol(hex, NULL, 16);
            strncpy(hex, argb + 2, 2); color->r = strtol(hex, NULL, 16);
            strncpy(hex, argb + 4, 2); color->g = strtol(hex, NULL, 16);
            strncpy(hex, argb + 6, 2); color->b = strtol(hex, NULL, 16);
        }
    } else { 
        Log_write(LOG_ERROR, "[TOFU] Wrong palette type, need to be string or list");
    }

    if (count > 0) {
        Display_palette(environment->display, colors, count);
    }
}

void graphics_canvas_point(WrenVM *vm)
{
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int color = (int)wrenGetSlotDouble(vm, 3);

    DrawPixel(x, y, (Color){ color, color, color, 255 });
}

void graphics_canvas_polygon(WrenVM *vm)
{
    const char *mode = wrenGetSlotString(vm, 1);
    int vertices = wrenGetListCount(vm, 2);
    int color = (int)wrenGetSlotDouble(vm, 3);

    int slots = wrenGetSlotCount(vm);
#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Currently #%d slot(s) available, asking for additional slot", slots);
#endif
    const int aux_slot_id = slots;
    wrenEnsureSlots(vm, aux_slot_id + 1); // Ask for an additional temporary slot.

#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Canvas.polygon(%d, %d, %d)", mode, color, vertices);
#endif

    const int count = vertices / 2;
    if (count == 0) {
        Log_write(LOG_LEVELS_INFO, "[TOFU] Polygon as no vertices");
        return;
    }

    // When drawing lines we need to ensure to be in mid-pixel coordinates. Also the length of lines are inclusive
    // (and this need to be taken into account for rectangles/squares). This is due to the "diamond exit rule" in
    // OpenGL rasterization.
    //
    // http://glprogramming.com/red/appendixg.html#name1
    float offset = (strcmp(mode, "line") == 0) ? 0.5f : 0.0f;

    Vector2 points[count];
    for (int i = 0; i < count; ++i) {
        wrenGetListElement(vm, 2, (i * 2), aux_slot_id);
        int x = (int)wrenGetSlotDouble(vm, aux_slot_id);
        wrenGetListElement(vm, 2, (i * 2) + 1, aux_slot_id);
        int y = (int)wrenGetSlotDouble(vm, aux_slot_id);

        points[i] = (Vector2){
                .x = (float)x + offset, .y = (float)y + offset
            };
    }

    if (strcmp(mode, "fill") == 0) {
        DrawPolyEx(points, count, (Color){ color, color, color, 255 });
    } else
    if (strcmp(mode, "line") == 0) {
        DrawPolyExLines(points, count, (Color){ color, color, color, 255 });
    } else {
        Log_write(LOG_LEVELS_WARNING, "[TOFU] Undefined drawing mode for polygon: '%s'", mode);
    }
}

void graphics_canvas_circle(WrenVM *vm)
{
    const char *mode = wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    float radius = (float)wrenGetSlotDouble(vm, 4);
    int color = (int)wrenGetSlotDouble(vm, 5);

#ifdef DEBUG
    Log_write(LOG_LEVELS_DEBUG, "Canvas.circle(%s, %d, %d, %d, %d)", mode, x, y, radius, color);
#endif

    if (strcmp(mode, "fill") == 0) {
        DrawCircle(x, y, radius, (Color){ color, color, color, 255 });
    } else
    if (strcmp(mode, "line") == 0) {
        DrawCircleLines(x, y, radius, (Color){ color, color, color, 255 });
//     } else
//     if (strcmp(mode, "sector") == 0) {
//         DrawCircleSector(x, y, radius, (Color){ color, color, color, 255 });
    } else {
        Log_write(LOG_LEVELS_WARNING, "[TOFU] Undefined drawing mode for polygon: '%s'", mode);
    }
}
