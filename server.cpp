#include <stdio.h>
#include <thread>
#include <mutex>

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
#include "EasyLibs/EasyImgui.h"

std::mutex mtx;

int main(int argc, char** argv)
{
    windowTitle = (char*)"Server";
    windowWidth = 1200;
    windowHeight = 650;

    initKeyMapping();
    initEasySocket();
    initEasyImgui();

    EasyClient client;
    char code[5] = "0000";
    char port[6] = "3402";
    char host[16] = "127.0.0.1";

    char port2[] = "3403";
    EasyEvent easy_event;

    EasyServer server;
    BoxManager boxman;
    cv::Mat image;
    GLuint image_texture;

    int loopCount = 0;

    boxman.setCompleteCallback(
        [&image, &loopCount](PacketBox& box) {
            std::vector<uchar> buf;
            PacketBoxToBuf(box, buf);
            if (box.type == 'I') {
                std::lock_guard<std::mutex> lock(mtx);
                decompressImage(buf, image);
                if (image.rows > 0 && image.cols > 0)
                {
                    printf("Image received!\n");
                    printf("loopCount = %d\n", loopCount);
                }
            }
        }
    );

    server.setService(
        [&boxman](SOCKET sock, char data[], int size, char host[]) {
            std::vector<uchar> buf(data, data + size);
            boxman.addPacketToBox(buf);
        }
    );

    std::thread socketThread;
    ImVec2 mousePosRelativeToImage;

    bool quit = false;

    while (!quit)
    {
        loopCount++;

        // SDL poll event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Start a new ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Code goes here
        ImGui::SetNextWindowPos(ImVec2(930, 20));
        ImGui::SetNextWindowSize(ImVec2(255, 100));
        ImGui::Begin("Connect to client");

        ImGui::Text("Host");
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputText("##host", (char*)host, 16);
        ImGui::PopItemWidth();

        ImGui::Text("Port");
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputText("##port", (char*)port, 6);
        ImGui::PopItemWidth();

        ImGui::Text("Code");
        ImGui::SameLine();
        ImGui::PushItemWidth(135);
        ImGui::InputText("##code", (char*)code, 5);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Connect")) {
            client.econnect(host, port, "UDP");
            client.sendData(code, 5);
            client.eclose();
            server.elisten(port2, "UDP");
            socketThread = std::thread([&server, &quit](){
                while (!quit) {
                    server.UDPReceive();
                }
            });
        }

        ImGui::End();

        if (true) {
            std::lock_guard<std::mutex> lock(mtx);
            GLuint oldTexture = image_texture;
            image_texture = MatToTexture(image);
            glDeleteTextures(1, &oldTexture);

            ImGui::SetNextWindowPos(ImVec2(20, 20));
            ImGui::SetNextWindowSize(ImVec2(image.size().width + 10, image.size().height + 40));
            ImGui::Begin("OpenCV Image", NULL, ImGuiWindowFlags_NoMove);
            ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.size().width, image.size().height));
            if (ImGui::IsItemClicked(0)) {
                mousePosRelativeToImage = ImGui::GetMousePos() - ImGui::GetItemRectMin();
            }
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(930, 125));
            ImGui::SetNextWindowSize(ImVec2(255, 50)); 
            ImGui::Begin("Text Window", NULL, ImGuiWindowFlags_NoMove);
            ImGui::Text("Mouse Click Position: (%.2f, %.2f)", mousePosRelativeToImage.x, mousePosRelativeToImage.y);
            ImGui::End();
        }

        // Rendering
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    socketThread.join();

    cleanEasyImgui();
    cleanEasySocket();
    cleanKeyMapping();

    return 0;
}
