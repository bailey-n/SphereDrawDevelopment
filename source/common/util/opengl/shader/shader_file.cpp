#include "shader_file.h"
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

shaderFile::shaderFile(const char *shader_path, int shader_type) :
        shader_file(shader_path), shader_type(shader_type), compile_status(-1) {
    // Validate shader type
    switch (shader_type) {
        case (GL_VERTEX_SHADER):
        case (GL_TESS_CONTROL_SHADER):
        case (GL_TESS_EVALUATION_SHADER):
        case (GL_FRAGMENT_SHADER):
            break;
        default:
            std::cout << "Attempted to create shader with an invalid type (" << shader_type << ")\n";
            throw std::exception();
    }

    // Open shader file
    gl_shader_id = glCreateShader(shader_type);
    std::ifstream shader_handle(shader_path, std::ios::in);
    if (!shader_handle.is_open()) {
        std::cout << "Could find/open shader file \"" << shader_file << "\"\n";
        throw std::exception();
    }

    // Read code into string
    std::stringstream code_stream;
    code_stream << shader_handle.rdbuf();
    shader_code = code_stream.str();
    shader_handle.close();

    // Compile shader
    const char * code_ptr = shader_code.c_str();
    glShaderSource(gl_shader_id, 1, &code_ptr, nullptr);
    glCompileShader(gl_shader_id);
    glGetShaderiv(gl_shader_id, GL_COMPILE_STATUS, &compile_status);

    // Store log and code
    int log_length;
    glGetShaderiv(gl_shader_id, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        // Note: info log stores a redundant null terminator
        info_log.resize(log_length + 1);
        glGetShaderInfoLog(gl_shader_id, log_length, nullptr, info_log.data());
    } else {
        info_log = "";
    }

    if (!compile_okay()) {
        std::cout << "Warning: Failed to compile shader \"" << shader_path << "\". Reason: " << info_log << std::endl;
    }
}

const std::string& shaderFile::get_info_log() const {
    return info_log;
}

int shaderFile::get_compile_status() const {
    return compile_status;
}

const std::string& shaderFile::get_shader_code() const {
    return shader_code;
}

bool shaderFile::compile_okay() const {
    return (compile_status == GL_TRUE);
}

void shaderFile::delete_shader() {
    glDeleteShader(this->gl_shader_id);
    shader_file = "";
    shader_code = "";
    shader_type = -1;
    gl_shader_id = -1;
    compile_status = -1;
    info_log = "";
}

