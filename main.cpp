#define IMGUI_DEFINE_MATH_OPERATORS

#include <stdio.h>
#include <string>
#include <vector>

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyDataMan.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl2.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define MAX_BYTES 8192

#ifdef WINDOWS
    #define sleep(x) Sleep(x * 1000)
#endif

EasyEvent easy_event;
EasySocket easy_socket;

char port[] = "44444";

int main(int argc, char** argv)
{
    initKeyMapping();

    if (std::stoi(argv[1]) == 1)
    {
        easy_socket.CreateServer(port, "UDP");

        // // Initialize SDL
        // if (SDL_Init(SDL_INIT_VIDEO)) {
        //     printf("Error: %s\n", SDL_GetError());
        //     return -1;
        // }

        // const int windowWidth = 1100;
        // const int windowHeight = 700;

        // SDL_Window* window = SDL_CreateWindow(
        //     "ImGui with SDL 2 and OpenGL 2",
        //     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        //     windowWidth, windowHeight, SDL_WINDOW_OPENGL
        // );

        // if (window == NULL) {
        //     printf("Error: %s\n", SDL_GetError());
        //     return -1;
        // }

        // // Create an OpenGL context
        // SDL_GLContext glContext = SDL_GL_CreateContext(window);

        // // Set up ImGui
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO& io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // // Init ImGui for SDL
        // ImGui_ImplSDL2_InitForOpenGL(window, glContext);

        // // Init ImGui for OpenGL
        // ImGui_ImplOpenGL2_Init();

        // // Main loop
        // ImVec2 clickPosition, imagePos, mousePosRelativeToImage;

        bool quit = false;

        BoxManager boxman;

        while (!quit)
        {
            // SDL_Event event;
            // while (SDL_PollEvent(&event)) {
            //     ImGui_ImplSDL2_ProcessEvent(&event);
            //     if (event.type == SDL_QUIT) {
            //         quit = true;
            //     }
            // }

            // // Start a new ImGui frame
            // ImGui_ImplOpenGL2_NewFrame();
            // ImGui_ImplSDL2_NewFrame(window);
            // ImGui::NewFrame();

            // code here

            auto Services = [&boxman](SOCKET sock, char data[], int size)
            {
                // printf("received!\n");

                std::vector<uchar> buf(data, data + size);
                boxman.addPacketToBox(buf);
                // printf("id = %d\n", *(int*)buf.data());
                bool *isComplete = (bool*)buf.data() + 5;

                if (*isComplete) {
                    printf("OKE!\n");
                    std::vector<uchar> buff;
                    PacketBoxToBuf(boxman.boxs.back(), buff);

                    cv::Mat mat;
                    decompressImage(buff, mat);
                    resize(mat, mat, cv::Size(), 2, 2);

                    if (!mat.empty()) {
                        cv::imshow("screen", mat);
                        cv::waitKey(10);
                    }
                }
            };

            easy_socket.setServices(Services);
            easy_socket.UDPReceive();

            // ImGui::SetNextWindowSize(ImVec2(image.size().width + 10, image.size().height + 40)); // Adjust the height to accommodate the text
            // ImGui::SetNextWindowPos(ImVec2(60, 100));

            // ImGui::Begin("OpenCV Image", NULL, ImGuiWindowFlags_NoMove);

            // // Display the image as an OpenGL texture inside the ImGui window
            // GLuint image_texture = easy_toolkit.MatToTexture(image);
            // int width, height;
            // int bytesPerPixel; // 3 for RGB, 4 for RGBA
            // int numMipmapLevels; // Number of mipmap levels

            // // Get texture parameters
            // glBindTexture(GL_TEXTURE_2D, image_texture);
            // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &bytesPerPixel);
            // glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &numMipmapLevels);

            // // Calculate the size in bytes
            // int size = width * height * bytesPerPixel;

            // // The 'size' variable now contains the size of the texture in bytes.
            // printf("texture size = %u\n", size);

            // ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.size().width, image.size().height));

            // if (ImGui::IsItemClicked(0)) {
            //     // Get the mouse position relative to the top-left corner of the window
            //     clickPosition = ImGui::GetMousePos();

            //     // You can also get the mouse position relative to the image if needed
            //     imagePos = ImGui::GetItemRectMin();
            //     mousePosRelativeToImage = clickPosition - imagePos;
            // }

            // ImGui::End();

            // // Create a separate ImGui window for the text
            // ImGui::SetNextWindowSize(ImVec2(300, 50)); // Adjust the size as needed
            // ImGui::SetNextWindowPos(ImVec2(image.size().width + 100, 100)); // Position next to the image window
            // ImGui::Begin("Text Window", NULL, ImGuiWindowFlags_NoMove);

            // // Display the stored mouse position
            // ImGui::Text("Mouse Click Position: (%.2f, %.2f)", mousePosRelativeToImage.x, mousePosRelativeToImage.y);

            // ImGui::End();

            // // Rendering
            // glViewport(0, 0, windowWidth, windowHeight);
            // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            // glClear(GL_COLOR_BUFFER_BIT);
            // ImGui::Render();
            // ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            // // Swap buffers
            // SDL_GL_SwapWindow(window);
        }

        // // Cleanup and shutdown
        // ImGui_ImplOpenGL2_Shutdown();
        // ImGui_ImplSDL2_Shutdown();
        // ImGui::DestroyContext();

        // SDL_GL_DeleteContext(glContext);
        // SDL_DestroyWindow(window);
        // SDL_Quit();

        return 0;
    }
    else
    {
        char host[256] = "127.0.0.1";
        // printf("host = ");
        // scanf("%s", host);
        SOCKET ConnectSocket = easy_socket.ConnectTo(host, port, "UDP");
        if (!ConnectSocket) return 0;

        // auto KeyDownCallback = [&](int keyCode) {
        //     char buf[MAX_BYTES];
        //     snprintf(buf, sizeof(buf), "Press %d\n", keyCode);
        //     easy_socket.SendData(ConnectSocket, buf, strlen(buf));
        //     // printf("KeyDown %d\n", keyCode);
        // };

        // auto KeyUpCallback = [](int keyCode) {
        //     // printf("KeyUp %d\n", keyCode);
        // };

        // auto LDownCallback = [](int x, int y) {
        //     // printf("LDown %d %d\n", x, y);
        // };

        // auto LUpCallback = [](int x, int y) {
        //     // printf("LUp %d %d\n", x, y);
        // };

        // auto RDownCallback = [](int x, int y) {
        //     // printf("RDown %d %d\n", x, y);
        // };

        // auto RUpCallback = [](int x, int y) {
        //     // printf("RUp %d %d\n", x, y);
        // };

        // auto MoveCallback = [](int x, int y) {
        //     // printf("Move %d %d\n", x, y);
        // };

        // easy_event.setKeyDownCallback(KeyDownCallback);
        // easy_event.setKeyUpCallback(KeyUpCallback);
        // easy_event.setLDownCallback(LDownCallback);
        // easy_event.setLUpCallback(LUpCallback);
        // easy_event.setRDownCallback(RDownCallback);
        // easy_event.setRUpCallback(RUpCallback);
        // easy_event.setMoveCallback(MoveCallback);

        // easy_event.StartHook();
        // while (true) {
        //     easy_event.MsgLoop();
        // }
        // easy_event.Unhook();

        int idCnt = 0;

        while (true)
        {
            cv::Mat mat = easy_event.CaptureScreen();
            // resize(mat, mat, cv::Size(), 0.5, 0.5);
            std::vector<uchar> buff;
            compressImage(mat, buff);

            PacketBox box;
            BufToPacketBox(buff, box, ++idCnt, 'I', 100);

            for (int i = 0; i < box.packets.size(); i++) {
                easy_socket.SendData(ConnectSocket, (void*)box.packets[i].data(), box.packets[i].size());
                // cv::waitKey(1);
            }
            printf("send done!\n");
            // sleep(10);
        }
    }

    return 0;
}
