#pragma once
#ifndef _TEXTREN_H_
#define _TEXTREN_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "MatrixStack.h"
#include "Shape.h"
#include "Multishape.h"

#include "GLSL.h"
#include "Program.h"
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    GLuint     TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint     Advance;
};

class Program;

class TextRenderer
{
public:
	TextRenderer(std::string font_location);
	virtual ~TextRenderer();
    void RenderText(std::shared_ptr<Program> textprog, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, float win_w, float win_h);
	GLuint VAO, VBO;
    std::map<GLchar, Character> Characters;
};


#endif
