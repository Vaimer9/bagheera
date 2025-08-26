#include "shaders.hpp"
#include "utils.hpp"
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <iostream>


Shaders::Shaders(std::string vert_path, std::string frag_path)
{
    GLuint vertid = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragid = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vert_code;
    std::string frag_code;

    std::ifstream vert_stream(vert_path, std::ios::in);
    if (vert_stream.is_open()) {
        std::stringstream str_stream;
        str_stream << vert_stream.rdbuf();
        vert_code = str_stream.str();
        vert_stream.close();
    } else {
        std::cerr << "Could not open " + vert_path << std::endl;
        return;
    }

    std::ifstream frag_stream(frag_path, std::ios::in);
    if (frag_stream.is_open()) {
        std::stringstream str_stream;
        str_stream << frag_stream.rdbuf();
        frag_code = str_stream.str();
        frag_stream.close();
    } else {
        std::cerr << "Could not open " + frag_path << std::endl;
        return;
    }

    GLint result = GL_FALSE;
    int log_len;

    // Compile Vertex Shader
    char const* vert_source_pointer = vert_code.c_str();
    glShaderSource(vert_id, 1, &vert_source_pointer, nullptr);
    glCompileShader(vert_id);

    // Check vertex shader
    glGetShaderiv(vert_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vert_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        std::vector<char> vert_err(log_len + 1);
        glGetShaderInfoLog(vert_id, log_len, nullptr, &vert_err[0]);
        std::cout << &vert_err[0] << std::endl;
    }


    // Compile Fragment Shader
    char const* frag_source_pointer = frag_code.c_str();
    glShaderSource(frag_id, 1, &frag_source_pointer, nullptr);
    glCompileShader(frag_id);

    // Check fragment shader
    glGetShaderiv(frag_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(frag_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        std::vector<char> frag_err(log_len + 1);
        glGetShaderInfoLog(frag_id, log_len, nullptr, &frag_err[0]);
        std::cout << &frag_err[0] << std::endl;
    }

    this->shader_id = glCreateProgram();
    glAttachShader(this->shader_id, vert_id);
    glAttachShader(this->shader_id, frag_id);
    glLinkProgram(this->shader_id);

    glGetProgramiv(this->shader_id, GL_LINK_STATUS, &result);
    glGetProgramiv(this->shader_id, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {
        std::vector<char> err_msg(log_len + 1);
        glGetProgramInfoLog(this->shader_id, log_len, nullptr, &err_msg[0]);
        std::cerr << &err_msg << std::endl;
    }

    glDetachShader(this->shader_id, vert_id);
    glDetachShader(this->shader_id, frag_id);

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}
