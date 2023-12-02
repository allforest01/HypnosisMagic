#include "EasyImgui.h"

SDL_Window* window;
SDL_GLContext glContext;
char* windowTitle = (char*)"ImGui with SDL 2 and OpenGL 2";
int windowWidth = 1100;
int windowHeight = 700;

void initEasyImgui() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(
        windowTitle,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL
    );

    if (window == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // Create an OpenGL context
    glContext = SDL_GL_CreateContext(window);

    // Set up ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Init ImGui for SDL
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);

    // Init ImGui for OpenGL
    ImGui_ImplOpenGL2_Init();
}

void cleanEasyImgui() {
    // Cleanup and shutdown
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
