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

#ifndef __LIBS_LUAX_H__
#define __LIBS_LUAX_H__

#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

typedef enum _luaX_Const_Type {
    LUA_CT_BOOLEAN,
    LUA_CT_INTEGER,
    LUA_CT_NUMBER,
    LUA_CT_STRING,
} luaX_Const_Type;

typedef struct _luaX_Const {
    const char *name;
    luaX_Const_Type type;
    union {
        int b;
        lua_Integer i;
        lua_Number n;
        const char *sz;
    } value;
} luaX_Const;

typedef struct _luaX_Script {
    const char *buffer;
    size_t size;
    const char *name;
} luaX_Script;

typedef int luaX_Reference;

#if DEBUG
    #define LUAX_SIGNATURE_BEGIN(l, n) \
        do { \
            lua_State *_L = (l); \
            int _n = (n); \
            int _argc = lua_gettop(L); \
            if (_argc != _n) { \
                luaL_error(_L, "[%s:%d] wrong number of arguments (need %d, got %d)", __FILE__, __LINE__, _n, _argc); \
            } \
            int _index = 1;
    #define LUAX_SIGNATURE_ARGUMENT(...) \
            luaX_checkargument(_L, _index++, __FILE__, __LINE__, __VA_ARGS__, LUA_TNONE);
    #define LUAX_SIGNATURE_END \
            (void)_index; \
        } while (0);
#else
    #define LUAX_SIGNATURE_BEGIN(l, n)
    #define LUAX_SIGNATURE_ARGUMENT(...)
    #define LUAX_SIGNATURE_END
#endif

#define LUAX_OVERLOAD_BEGIN(l) \
    do { \
        lua_State *_L = (l); \
        int _argc = lua_gettop(_L); \
        switch (_argc) {
#define LUAX_OVERLOAD_ARITY(n, f) \
            case (n): { return (f)(_L); }
#define LUAX_OVERLOAD_END \
            default: { return luaL_error(L, "[%s:%d] wrong number of arguments (got %d)", __FILE__, __LINE__, _argc); } \
        } \
    } while (0);

#define luaX_dump(L)                luaX_stackdump(L, __FILE__, __LINE__)

#define luaX_tofunction(L, arg)     luaX_ref(L, arg)

extern void luaX_stackdump(lua_State *L, const char *file, int line);
extern void luaX_overridesearchers(lua_State *L, lua_CFunction searcher, int nup);
extern int luaX_insisttable(lua_State *L, const char *name);
extern int luaX_newmodule(lua_State *L, const luaX_Script *script, const luaL_Reg *f, const luaX_Const *c, int nup, const char *name);
extern void luaX_openlibs(lua_State *L);
extern void luaX_preload(lua_State *L, const char *modname, lua_CFunction openf, int nup);
extern void luaX_requiref(lua_State *L, const char *modname, lua_CFunction openf, int nup, int glb);

extern luaX_Reference luaX_ref(lua_State *L, int idx);
extern void luaX_unref(lua_State *L, luaX_Reference ref);

extern void luaX_checkargument(lua_State *L, int idx, const char *file, int line, ...);

extern void luaX_pushvalues(lua_State *L, int nup);
extern int luaX_pushupvalues(lua_State *L);
extern int luaX_upvaluescount(lua_State *L); // UNUSED

#endif  /* __LIBS_LUAX_H__ */