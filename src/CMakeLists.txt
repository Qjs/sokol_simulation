set(EXEC_NAME "q_wasm")


add_executable(${EXEC_NAME}
    main.c
    simulations/none.c
    simulations/pendulum.c
    simulations/mcpi.c
    simulations/gol.c
    simulations/simulations.c
)

target_include_directories(${EXEC_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/simulations
)

target_link_libraries(${EXEC_NAME} PRIVATE cimgui cimplot sokol)

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${EXEC_NAME} PRIVATE --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
    target_link_options(${EXEC_NAME} PRIVATE -sUSE_WEBGL2=1)
    target_link_options(${EXEC_NAME} PRIVATE -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()