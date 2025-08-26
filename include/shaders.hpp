#pragma once

#include <string>
#include <GL/gl.h>
#include <fstream>
#include <sstream>

struct Shaders
{
    std::string vert_raw, frag_raw;
    GLuint vert_id, frag_id, shader_id;

    Shaders(std::string vert_path, std::string frag_path);
    void use();
};
