// sokol implementation library on non-Apple platforms
#define SOKOL_IMPL
#if defined(__MINGW32__)
#define SOKOL_GLCORE33
#elif defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES3
#elif defined(__APPLE__)
// NOTE: on macOS, sokol.c is compiled explicitely as ObjC
#define SOKOL_METAL
#else
#define SOKOL_GLCORE33
#endif
#include "../lib/sokol/sokol_app.h"
#include "../lib/sokol/sokol_gfx.h"
#include "../lib/sokol/sokol_log.h"
#include "../lib/sokol/sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../lib/cimgui/cimgui.h"
#define SOKOL_IMGUI_IMPL
#include "../lib/sokol/util/sokol_imgui.h"
