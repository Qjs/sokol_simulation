cmake_minimum_required(VERSION 3.5)
project(q_wasm_app)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")

#=== LIBRARY: cimgui + Dear ImGui
add_library(cimgui STATIC
    ${LIB_DIR}/cimgui/cimgui.cpp
    ${LIB_DIR}/cimgui/cimgui.h
    ${LIB_DIR}/cimgui/imgui/imgui.cpp
    ${LIB_DIR}/cimgui/imgui/imgui.h
    ${LIB_DIR}/cimgui/imgui/imgui_widgets.cpp
    ${LIB_DIR}/cimgui/imgui/imgui_draw.cpp
    ${LIB_DIR}/cimgui/imgui/imgui_tables.cpp
    ${LIB_DIR}/cimgui/imgui/imgui_demo.cpp
    ${LIB_DIR}/cimgui/imgui/imgui_internal.h
    ${LIB_DIR}/cimgui/imgui/imstb_rectpack.h
    ${LIB_DIR}/cimgui/imgui/imstb_textedit.h
    ${LIB_DIR}/cimgui/imgui/imstb_truetype.h
    ${LIB_DIR}/cimgui/imgui/imconfig.h
    )
target_include_directories(cimgui PUBLIC 
    ${LIB_DIR}/cimgui
    ${LIB_DIR}/cimgui/imgui)

set(SOKOL_HEADERS
    ${LIB_DIR}/sokol/sokol_gfx.h
    ${LIB_DIR}/sokol/sokol_app.h
    ${LIB_DIR}/sokol/util/sokol_imgui.h
    ${LIB_DIR}/sokol/util/sokol_gl.h
    ${LIB_DIR}/sokol/sokol_glue.h)


add_library(sokol STATIC ${CMAKE_SOURCE_DIR}/src/sokol.c ${SOKOL_HEADERS})
target_include_directories(sokol INTERFACE ${LIB_DIR}/sokol)

set(CIMPLOT_DIR "${LIB_DIR}/cimplot")

# Include Cimplot
add_library(cimplot STATIC
    ${CIMPLOT_DIR}/cimplot.cpp
    ${CIMPLOT_DIR}/cimplot.h
    ${CIMPLOT_DIR}/implot/implot.cpp
    ${CIMPLOT_DIR}/implot/implot.h
    ${CIMPLOT_DIR}/implot/implot_items.cpp
    ${CIMPLOT_DIR}/implot/implot_internal.h
    ${CIMPLOT_DIR}/implot/implot_demo.cpp
)

# Set the include directories for Cimplot
target_include_directories(cimplot PUBLIC
    ${CIMPLOT_DIR}
    ${CIMPLOT_DIR}/implot
    ${LIB_DIR}/cimgui
    ${LIB_DIR}/cimgui/imgui
)

# Link Cimplot with CImGui
target_link_libraries(cimplot PUBLIC cimgui)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL dl m)
    target_link_libraries(sokol PUBLIC Threads::Threads)
endif()
target_link_libraries(sokol PUBLIC cimgui)

add_subdirectory(src)