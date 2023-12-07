#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl2.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

struct ImGuiWrapper {
    SDL_Window* window;
    SDL_GLContext glContext;
    int window_width;
    int window_height;
    char* window_title;
    ImGuiWrapper() {}
    ImGuiWrapper(int width, int height, char* title);
};

void initImGui(ImGuiWrapper& imgui_wrapper);
void cleanImGui(ImGuiWrapper& imgui_wrapper);