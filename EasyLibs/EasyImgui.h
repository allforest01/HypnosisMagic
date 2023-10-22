#pragma once

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl2.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

extern SDL_Window* window;
extern SDL_GLContext glContext;
extern int windowWidth;
extern int windowHeight;

void initEasyImgui();
void cleanEasyImgui();