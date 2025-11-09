#pragma once
#define GLEW_STATIC        // (静的リンクの場合のみ必要)
#include <GL/glew.h>       // ★ 必ず最初に！
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class Shader {
public:
	static std::string LoadShaderSource(const char* filePath);
	static GLuint compileShader(GLenum type, const char* source);
	static GLuint createShaderProgram(const char* vsSource, const char* fsSource);
};