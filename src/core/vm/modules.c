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

#include "modules.h"

#include <core/vm/modules/bank.h>
#include <core/vm/modules/canvas.h>
#include <core/vm/modules/class.h>
#include <core/vm/modules/grid.h>
#include <core/vm/modules/font.h>
#include <core/vm/modules/input.h>
#include <core/vm/modules/file.h>
#include <core/vm/modules/math.h>
#include <core/vm/modules/system.h>
#include <core/vm/modules/surface.h>
#include <core/vm/modules/timer.h>
#include <libs/log.h>
#include <libs/luax.h>

#define LOG_CONTEXT "modules"

// TODO: http://www.ilikebigbits.com/2017_06_01_float_or_double.html

static int create_module(lua_State *L, const luaL_Reg *entries)
{
    lua_newtable(L);
    for (int i = 0; entries[i].func; ++i) {
        if (entries[i].func(L) != 1) {
            Log_write(LOG_LEVELS_ERROR, LOG_CONTEXT, "can't initialize class `%s`", entries[i].name);
            return 0;
        }
        lua_setfield(L, -2, entries[i].name);
    }
    return 1;
}

static int collections_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Grid", grid_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}

static int core_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Math", math_loader },
        { "System", system_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}

static int events_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Input", input_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}

static int graphics_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Bank", bank_loader },
        { "Canvas", canvas_loader },
        { "Font", font_loader },
        { "Surface", surface_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}
/*
static int audio_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Sound", sound_loader },
        { "Wave", wave_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}
*/
static int io_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "File", file_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}

static int util_loader(lua_State *L)
{
    static const luaL_Reg classes[] = {
        { "Class", class_loader },
        { "Timer", timer_loader },
        { NULL, NULL }
    };
    return create_module(L, classes);
}

void modules_initialize(lua_State *L, int nup)
{
    static const luaL_Reg modules[] = {
        { "tofu.collections", collections_loader },
        { "tofu.core", core_loader },
        { "tofu.events", events_loader },
        { "tofu.graphics", graphics_loader },
/*
        { "tofu.audio", audio_loader },
*/
        { "tofu.io", io_loader },
        { "tofu.util", util_loader },
        { NULL, NULL }
    };

#ifdef INSIST
    luaX_insisttable(L, "tofu");
    for (const luaL_Reg *module = modules; module->func; ++module) {
        luaX_pushvalues(L, nup);
        luaX_require(L, module->name, module->func, nup, 1);
        lua_setfield(L, -1, module->name);
    }
    lua_pop(L, nup);
#else
    for (const luaL_Reg *module = modules; module->func; ++module) {
        luaX_pushvalues(L, nup);
        luaX_preload(L, module->name, module->func, nup);
    }
    lua_pop(L, nup);
#endif
}
