#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <vector>
#include <map>
#include <string>
#include "shader_file.h"
#include "shader_program.h"
#include <array>

class shaderManager {
    static std::string shader_root;
    static std::map<std::string, shaderFile> shader_files;
    static std::map<std::string, shaderProgram> shader_programs;
    static unsigned int refcount;

    template <unsigned int ShaderType>
    static bool load_shader_if_not_loaded(const std::string& shader_path) {
        if (!shader_files.contains(shader_path)) {
            shader_files.emplace(shader_path, shaderFile(shader_path.c_str(), ShaderType));
            return shader_files.at(shader_path).compile_okay();
        }
        return shader_files.at(shader_path).compile_okay();
    }

    static bool load_vs_if_not_loaded(const std::string& vertex_path);
    static bool load_fs_if_not_loaded(const std::string& fragment_path);
    static bool load_tcs_if_not_loaded(const std::string& tessellation_control_path);
    static bool load_tes_if_not_loaded(const std::string& tessellation_evaluation_path);
    static bool load_program_if_not_loaded(const std::vector<std::string>& paths);

public:
    static void set_shader_root_directory(std::string directory);
    shaderManager();
    shaderManager(std::string vertex_path, std::string fragment_path);
    shaderManager(std::string vertex_path, std::string fragment_path,
        std::string tessellation_control_path, std::string tessellation_evaluation_path);
    ~shaderManager();
    GLuint operator[](std::vector<std::string> shader_paths);
};



#endif //SHADERMANAGER_H
