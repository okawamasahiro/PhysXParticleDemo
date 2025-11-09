#include "Shader.h"

// ファイルを文字列として読み込む
std::string Shader::LoadShaderSource(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
GLuint Shader::compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        printf("Shader compile error: %s\n", log);
    }
    return shader;
}

GLuint Shader::createShaderProgram(const char* vsSource, const char* fsSource)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        printf("Program link error: %s\n", log);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}