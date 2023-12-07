#include "../include/imgui_wrapper.h"

ImGuiWrapper::ImGuiWrapper(int width, int height, char* title) {
    this->window_width = width;
    this->window_height = height;
    strcpy(this->window_title, title);
}

void initImGui(ImGuiWrapper& imgui_wrapper) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    imgui_wrapper.window = SDL_CreateWindow(
        imgui_wrapper.window_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        imgui_wrapper.window_width, imgui_wrapper.window_height, SDL_WINDOW_OPENGL
    );

    if (imgui_wrapper.window == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // Create an OpenGL context
    imgui_wrapper.glContext = SDL_GL_CreateContext(imgui_wrapper.window);

    // Set up ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Init ImGui for SDL
    ImGui_ImplSDL2_InitForOpenGL(imgui_wrapper.window, imgui_wrapper.glContext);

    // Init ImGui for OpenGL
    ImGui_ImplOpenGL2_Init();
}

void cleanImGui(ImGuiWrapper& imgui_wrapper) {
    // Cleanup and shutdown
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(imgui_wrapper.glContext);
    SDL_DestroyWindow(imgui_wrapper.window);
    SDL_Quit();
}
