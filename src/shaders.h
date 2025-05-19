#ifndef m_SHADER_H
#define m_SHADER_H

#include "arena_base.h"
#include "scratch_arena.h"

typedef struct Shader {
    GLuint ID;
} Shader;

Shader loadGlShaders(ScratchArena* arena, const char* vertexPath, const char* fragmentPath);

#endif

#define STRING_BUFFER 512

Shader loadGlShaders(ScratchArena* arena, const char* vertexPath, const char* fragmentPath) {
    arenaScratchPush(arena);

    FILE *vertexFile = fopen(vertexPath, "rb");
    if(!vertexFile) {
        fprintf(stderr, "ERROR: Failed to load vertex file.\n");
        arenaScratchPop(arena);
        exit(-1);
    }

    fseek(vertexFile, 0, SEEK_END);
    long fileSize = ftell(vertexFile);
    rewind(vertexFile);

    char* vertSource = arenaScratchAlloc(arena, fileSize + 1, 1);
    fread(vertSource, 1, fileSize, vertexFile);
    vertSource[fileSize] = '\0';
    fclose(vertexFile);

    FILE *fragmentFile = fopen(fragmentPath, "rb");
    if(!fragmentFile) {
        fprintf(stderr, "ERROR: Failed to load fragment file.\n");
        arenaScratchPop(arena);
        exit(-1);
    }
   
    fseek(fragmentFile, 0, SEEK_END);
    fileSize = ftell(fragmentFile);
    rewind(fragmentFile);

    char* fragSource = arenaScratchAlloc(arena, fileSize + 1, 1);
    fread(fragSource, 1, fileSize, fragmentFile);
    fragSource[fileSize] = '\0';
    fclose(fragmentFile);

    GLuint vertex, fragment;
    int success;
    char infoLog[STRING_BUFFER];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertSource, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertex, STRING_BUFFER, NULL, infoLog);
        fprintf(stderr, "ERROR: Vertex Shader: %s\n", infoLog);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragSource, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragment, STRING_BUFFER, NULL, infoLog);
        fprintf(stderr, "ERROR: Fragment Shader: %s\n", infoLog);
    }

    Shader temp;
    temp.ID = glCreateProgram();
    glAttachShader(temp.ID, vertex);
    glAttachShader(temp.ID, fragment);
    glLinkProgram(temp.ID);

    glGetProgramiv(temp.ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(temp.ID, STRING_BUFFER, NULL, infoLog);
        fprintf(stderr, "ERROR: Program Link: %s\n", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    arenaScratchPop(arena);

    return temp;
}

