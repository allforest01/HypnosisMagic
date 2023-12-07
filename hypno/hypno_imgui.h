#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl2.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

extern SDL_Window* window;
extern SDL_GLContext glContext;
extern char* window_title;
extern int window_width;
extern int window_height;

void initHypnoImgui();
void cleanHypnoImgui();