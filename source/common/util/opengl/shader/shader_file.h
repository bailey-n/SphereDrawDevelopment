#ifndef OGL_PROJECT_SHADER_FILE_H
#define OGL_PROJECT_SHADER_FILE_H

#include "opengl_include.h"
#include <map>
#include <string>
#include <memory>
#include "vector"

struct shaderFile {
    // Char buffers
    std::string shader_file;
    std::string shader_code;

    // Shader type and ID
    int shader_type;
    GLuint gl_shader_id;

    // Debug
    GLint compile_status;
    std::string info_log;

    shaderFile(const char * shader_path, int shader_type);
    [[nodiscard]] const std::string& get_info_log() const;
    [[nodiscard]] int get_compile_status() const;
    [[nodiscard]] const std::string& get_shader_code() const;
    [[nodiscard]] bool compile_okay() const;
    void delete_shader();
};


#endif //OGL_PROJECT_SHADER_FILE_H
