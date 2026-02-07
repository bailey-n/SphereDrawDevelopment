# CMake entry point
cmake_minimum_required (VERSION 3.10.0)
project (SphereDraw)
set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)


# Compile external dependencies
add_subdirectory (external)

include_directories(
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glew-1.13.0/include/
        external/assimp-3.0.1270/include/
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        GLEW_1130
)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
        -DGLFW_EXPOSE_NATIVE_WGL
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)

add_executable(SphereDraw
        # EXTERNAL DIRECT INCLUDES
        # imgui includes
        external/imgui/backends/imgui_impl_opengl3.cpp
        external/imgui/backends/imgui_impl_opengl3.h
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_glfw.h
        external/imgui/imconfig.h
        external/imgui/imgui.cpp
        external/imgui/imgui.h
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_internal.h
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imstb_rectpack.h
        external/imgui/imstb_textedit.h
        external/imgui/imstb_truetype.h
        # stb_image include
        external/stb_image.h

        # SOURCE CODE INCLUDES
        # Main includes
        source/main.cpp
        source/application.h
        source/application.cpp
        source/application_action.h
        source/planet.cpp
        source/planet.h
        source/gui.h
        source/gui.cpp

        # Includes for rendered shapes
        # Meshes for OpenGL
        source/common/geometry/drawable/drawable.cpp
        source/common/geometry/drawable/drawable.h
        source/common/geometry/meshes/mesh_util.cpp
        source/common/geometry/meshes/mesh_util.h
        source/common/geometry/meshes/sphere_mesh.cpp
        source/common/geometry/meshes/sphere_mesh.h
        source/common/geometry/quadmap/quadmap.cpp
        source/common/geometry/quadmap/quadmap.h
        source/common/geometry/sphere_point.cpp
        source/common/geometry/sphere_point.h
        # Higher-level shape object classes

        # General utility includes
        source/common/util/camera/camera.cpp
        source/common/util/camera/camera.h
        source/common/util/cubemap/cubemap_util.cpp
        source/common/util/cubemap/cubemap_util.h
        source/common/util/math/shapes.cpp
        source/common/util/math/shapes.h
        source/common/util/timer/timer.cpp
        source/common/util/timer/timer.h
        # OpenGL abstraction includes
        # vertex buffer/element buffer
        source/common/util/opengl/buffer/vertex_attribute.h
        source/common/util/opengl/buffer/vertex_attribute.cpp
        source/common/util/opengl/buffer/element_buffer.h
        source/common/util/opengl/buffer/element_buffer.cpp
        # shader files management
        source/common/util/opengl/shader/shader_manager.cpp
        source/common/util/opengl/shader/shader_manager.h
        source/common/util/opengl/shader/shader_file.h
        source/common/util/opengl/shader/shader_file.cpp
        source/common/util/opengl/shader/shader_program.cpp
        source/common/util/opengl/shader/shader_program.h
        source/common/geometry/drawing/primitive_id_manager.h
        source/common/geometry/drawing/primitive_id_manager.cpp

        # NEW INCLUDES (Organize later)
)
target_link_libraries(SphereDraw
        ${ALL_LIBS}
)
target_include_directories(SphereDraw PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/geometry
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/geometry/drawable
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/geometry/meshes
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/geometry/quadmap
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/camera
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/cubemap
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/math
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/opengl
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/opengl/buffer
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/opengl/shader
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common/util/timer
)

add_subdirectory(test)
