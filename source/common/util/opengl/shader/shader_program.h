#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "opengl_include.h"
#include "shader_file.h"

struct shaderProgram {
    GLuint program_id = -1;
    int link_status = -1;
    std::string info_log;

    explicit shaderProgram(std::vector<shaderFile*>& shaders, bool delete_shader = false);
    [[nodiscard]] const std::string& get_info_log() const;
    [[nodiscard]] int get_link_status() const;
    [[nodiscard]] bool build_okay() const;
    [[nodiscard]] GLuint get_program_id() const;
};



#endif //SHADER_PROGRAM_H
