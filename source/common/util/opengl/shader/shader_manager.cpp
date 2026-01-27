#include "shader_manager.h"
#include <iostream>

std::string shaderManager::shader_root;
std::map<std::string, shaderFile> shaderManager::shader_files = {};
std::map<std::string, shaderProgram> shaderManager::shader_programs = {};
unsigned int shaderManager::refcount = 0;
shaderManager::shaderManager() {
    refcount += 1;
}

shaderManager::shaderManager(const std::string vertex_path, const std::string fragment_path) {
    refcount += 1;
    load_program_if_not_loaded({
        shader_root + vertex_path,
        shader_root + fragment_path
    });
}

shaderManager::shaderManager(
    const std::string vertex_path, const std::string fragment_path,
    const std::string tessellation_control_path, const std::string tessellation_evaluation_path
    ) {
    refcount += 1;
    load_program_if_not_loaded({
        shader_root + vertex_path,
        shader_root + fragment_path,
        shader_root + tessellation_control_path,
        shader_root + tessellation_evaluation_path
    });
}

shaderManager::~shaderManager() {
    refcount -= 1;
    if (!refcount) {
        for (const auto& program: shader_programs) {
            if (program.second.build_okay()) glDeleteProgram(program.second.program_id);
        }
        shader_programs.clear();
        for (auto& shader: shader_files) {
            if (shader.second.compile_okay()) shader.second.delete_shader();
        }
    }
}

bool shaderManager::load_vs_if_not_loaded(const std::string& vertex_path) { return load_shader_if_not_loaded<GL_VERTEX_SHADER>(vertex_path); }
bool shaderManager::load_fs_if_not_loaded(const std::string& fragment_path) { return load_shader_if_not_loaded<GL_FRAGMENT_SHADER>(fragment_path); }
bool shaderManager::load_tcs_if_not_loaded(const std::string& tessellation_control_path) { return load_shader_if_not_loaded<GL_TESS_CONTROL_SHADER>(tessellation_control_path); }
bool shaderManager::load_tes_if_not_loaded(const std::string& tessellation_evaluation_path) { return load_shader_if_not_loaded<GL_TESS_EVALUATION_SHADER>(tessellation_evaluation_path); }

bool shaderManager::load_program_if_not_loaded(const std::vector<std::string>& paths) {
    if (paths.size() == 2) {
        if (!load_vs_if_not_loaded(paths[0])) return false;
        if (!load_fs_if_not_loaded(paths[1])) return false;

        std::string name = "2:" + paths[0] + ":" + paths[1] + ":";
        if (!shader_programs.contains(name)) {
            std::vector<shaderFile*> pipeline = {
                &shader_files.at(paths[0]),
                &shader_files.at(paths[1])
            };
            shader_programs.emplace(name, shaderProgram(pipeline));
            return shader_programs.at(name).build_okay();
        }
        return shader_programs.at(name).build_okay();;
    }
    else if (paths.size() == 4) {
        if (!load_vs_if_not_loaded(paths[0])) return false;
        if (!load_fs_if_not_loaded(paths[1])) return false;
        if (!load_tcs_if_not_loaded(paths[2])) return false;
        if (!load_tes_if_not_loaded(paths[3])) return false;

        std::string name = "4:" + paths[0] + ":" + paths[1] + ":" + paths[2] + ":" + paths[3] + ":";
        if (!shader_programs.contains(name)) {
            std::vector<shaderFile*> pipeline = {
                &shader_files.at(paths[0]),
                &shader_files.at(paths[1]),
                &shader_files.at(paths[2]),
                &shader_files.at(paths[3])
            };
            shader_programs.emplace(name, shaderProgram(pipeline));
            return shader_programs.at(name).build_okay();
        }
        return shader_programs.at(name).build_okay();;
    }
    else return false;
}

GLuint shaderManager::operator[](std::vector<std::string> shader_paths) {
    for (auto& path: shader_paths) path = shader_root + path;
    if (!load_program_if_not_loaded(shader_paths)) {
        std::cout << "Warning: failed to generate shader program" << std::endl;
        return -1;
    };

    std::string prog_name(std::to_string(shader_paths.size()) + ":");
    for (const auto& path: shader_paths) prog_name += path + ":";

    if (!shader_programs.contains(prog_name)) {
        std::cout << "Warning: failed to access shader program" << std::endl;
        return -1;
    }
    return shader_programs.at(prog_name).program_id;
}

void shaderManager::set_shader_root_directory(const std::string directory) {
    shader_root = directory;
}
