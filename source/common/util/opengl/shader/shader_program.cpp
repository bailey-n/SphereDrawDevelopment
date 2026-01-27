#include "shader_program.h"
#include <iostream>

shaderProgram::shaderProgram(std::vector<shaderFile*> &shaders, const bool delete_shader) {
    program_id = glCreateProgram();
    for (const auto shader: shaders) {
        glAttachShader(program_id, shader->gl_shader_id);
    }
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
    int log_length;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
    info_log.resize(log_length + 1);
    glGetProgramInfoLog(program_id, log_length, nullptr, info_log.data());

    if (!build_okay()) {
        std::cout << "Warning: Failed to link shaders";
        for (auto shader: shaders) {
            std::cout << " \"" << shader->shader_file << "\"";
        }
        std::cout << ". \nStatus: " << link_status << "\n";
        std::cout << "Reason: " << info_log << std::endl;
    }

    for (const auto shader: shaders) {
        glDetachShader(program_id, shader->gl_shader_id);
    }

    if (delete_shader) {
        for (auto shader: shaders) {
            shader->delete_shader();
        }
    }
}

const std::string& shaderProgram::get_info_log() const {
    return info_log;
}

int shaderProgram::get_link_status() const {
    return link_status;
}

bool shaderProgram::build_okay() const {
    return (link_status == GL_TRUE);
}

GLuint shaderProgram::get_program_id() const {
    return program_id;
}

