#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "arena_base.h"
#include "page_arena.h"
#include "scratch_arena.h"
#include "zeta.h"
#include <stdio.h>
#include <stddef.h>
#include "shaders.h"
#include "linmath.h"
#include "camera.h"

//#ifndef SCRATCH_ARENA_IMPLEMENTED
//#errror "scratch_arena.c did not get linked"
//#endif

#define PAGE_SPACE_SIZE MiB(10)
#define SCRATCH_SIZE 1024 * 128
#define ARENA_SIZE MiB(1)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960
#define TRUE 1
#define FALSE 0

#define ZETA_INDEX_COUNT(w, h) (((h) - 1) * ((w) - 1) * 6)

f32 deltaTime;
f32 lastFrame;
f32 currentFrame;
f32 lastPress;
f32 lastPressWire;
u32 mouseFirst;
Camera *cam;
u8 isLine;
u8 isPoints;
u32 grid_w;
u32 grid_h;
u32 indexCount;

typedef void (*RenderFunc)(void);
RenderFunc renderFunc;

void drawAsPoints(void) {
    glDrawArrays(GL_POINTS, 0, grid_w * grid_h);
}

void drawAsSurface(void) {
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void scroll_callback(GLFWwindow *window, f64 xOffset, f64 yOffset) {
    ProcessMouseScroll(cam, yOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        ProcessKeyboard(cam, FORWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        ProcessKeyboard(cam, BACKWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        ProcessKeyboard(cam, LEFT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        ProcessKeyboard(cam, RIGHT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (lastPressWire < 1) {
            return;
        }
        lastPressWire = 0.0f;
        if(isLine) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            isLine = !isLine;
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            isLine = !isLine;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (lastPress < 1) {
            return;
        }
        lastPress = 0.0f;
        if(isPoints) {
            renderFunc = drawAsSurface;
        } else {
            renderFunc = drawAsPoints;
        }
        isPoints = !isPoints;
    }

}

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    if(mouseFirst) {
        cam->lastX = xpos;
        cam->lastY = ypos;
        mouseFirst = FALSE;
    }

    f32 xOffset = xpos - cam->lastX;
    f32 yOffset = cam->lastY - ypos;
    cam->lastX = xpos;
    cam->lastY = ypos;
    ProcessMouseMovement(cam, xOffset, yOffset, TRUE);
}

void error_callback(int error, const char *description) {
    fprintf(stderr, "ERROR: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    mouseFirst = TRUE;
    isLine = TRUE;
    isPoints = TRUE;

    memMap *map = initMemMap(PAGE_SPACE_SIZE);
    PageArena *scratch = createPageArena(map, SCRATCH_SIZE);
    PageArena *arena = createPageArena(map, ARENA_SIZE);
   
    cam = arenaPageAlloc(scratch, sizeof(Camera), ALIGN_4);
    CameraInit(cam, (vec3){1.f, 1.f, 5.f}, (vec3){0.f, 1.f, 0.f}, YAW, PITCH);
    
    if(!cam) {
        fprintf(stderr, "ERROR: Failed to initialize camera.\n");
        glfwTerminate();
        arenaPagePop(map);
        arenaPagePop(map);
        releasePages(map);
        exit(EXIT_FAILURE);
    }

    if(!glfwInit()) {
        fprintf(stderr, "ERROR: Failed to initialize GLFW. \n");
        glfwTerminate();
        arenaPagePop(map);
        arenaPagePop(map);
        releasePages(map);
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL Lighting 3", NULL, NULL);
    if(!window) {
        fprintf(stderr, "ERROR: Failed to initialize window.\n");
        glfwTerminate();
        arenaPagePop(map);
        arenaPagePop(map);
        releasePages(map);
        exit(EXIT_FAILURE);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    f32 sigma_max = 1.0f;
    f32 sigma_min = 0.5f;
    f32 t_min = 5;
    f32 t_max = 15;
    grid_w = 100;
    grid_h = 100;
    ZetaPoint *zetaPoints = arenaPageAlloc(arena, grid_w * grid_h * sizeof(ZetaPoint),  ALIGN_16);
    ZetaVertex* zetaVertices = arenaPageAlloc(arena, grid_w * grid_h * sizeof(ZetaVertex), ALIGN_16);
    populateMesh(zetaPoints, zetaVertices, grid_w, grid_h, sigma_min, sigma_max, t_min, t_max, zetaApprox);
    
    u32* indices = arenaPageAlloc(arena, (grid_h - 1) * (grid_w - 1) * 6 * sizeof(u32), ALIGN_4);
    generateMesh(indices, grid_w, grid_h);
    indexCount = ZETA_INDEX_COUNT(grid_w, grid_h);
    renderFunc = drawAsPoints;
    
    ScratchArena tmp = createScratchArena(SCRATCH_SIZE);
    Shader shader = loadGlShaders(&tmp, "shaders/mathModel.vs", "shaders/mathModel.fs");

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, grid_w * grid_h * sizeof(ZetaVertex), zetaVertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ZetaVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(f32) * 3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(u32), indices, GL_STATIC_DRAW);
    
    mat4x4 projection;
    mat4x4_identity(projection);
    f32 fovRad;
    mat4x4 model;
    mat4x4_identity(model);
    mat4x4 view;

    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint projLoc = glGetUniformLocation(shader.ID, "projection");
    GLuint viewLoc = glGetUniformLocation(shader.ID, "view");

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        lastPress += deltaTime;
        lastPressWire += deltaTime;

        fovRad = DEG2RAD(cam->Zoom);
        processInput(window);

        glClearColor(0.4f, 0.4f, 0.4f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader.ID);
        
        mat4x4_perspective(projection, fovRad, (f32)width / height, 0.01f, 500.f);

        vec3 tmp;
        vec3_add(tmp, cam->Position, cam->Front);
        mat4x4_look_at(view, cam->Position, tmp, cam->Up);

        mat4x4_identity(model);
        
        glUseProgram(shader.ID);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const f32*)projection);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const f32*)view);
        mat4x4_identity(model);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const f32*)model);

        glBindVertexArray(VAO);
        renderFunc();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    arenaPagePop(map);
    arenaPagePop(map);
    releasePages(map);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
