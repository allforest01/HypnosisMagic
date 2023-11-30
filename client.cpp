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
    initKeyMapping();
    initEasySocket();

    EasyServer server;
    char port[5] = "3402";


    windowTitle = (char*)"Client";
    windowWidth  = 310;
    windowHeight = 120;
    
    initEasyImgui();

    bool quit = false;
    bool wait = false;

    while (!quit)
    {
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
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::SetNextWindowSize(ImVec2(270, 80));
        ImGui::Begin("Port");
        if (!wait) {
            // Open UDP socket to wait for connect from server
            ImGui::Text("Open a port to wait for a connection");
            ImGui::PushItemWidth(200);
            ImGui::InputText("##port", (char*)port, 6);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                // Wait for a connection
                server.connect(port, "UDP");
                server.setService(
                    [&server](SOCKET sock, char data[], int size, char ipv4[]) {
                        printf("ipv4 = %s", ipv4);
                        server.disconnect();
                        return 0;
                    }
                );
                wait = true;
            }
        }
        else {
            // Waiting for a connection from server
            ImGui::PushItemWidth(-1);
            ImGui::ProgressBar(ImGui::GetTime() * -0.2f, ImVec2(0, 0), "Waiting for a connection");
            ImGui::PopItemWidth();
            if (ImGui::Button("Cancel")) {
                server.disconnect();
                wait = false;
            }
            // server.UDPReceive();
        }
        ImGui::End();

        // Rendering
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    cleanEasyImgui();
    cleanEasySocket();

    // char port1[] = "3402";
    // char port2[] = "3403";
    // EasyEvent easy_event;

    // // char host[16] = "10.211.55.23";
    // // char host[16] = "10.37.129.2";

    // char host[16] = "127.0.0.1";

    // // printf("host = ");
    // // scanf("%s", host);

    // EasyClient client(host, port1, "UDP");
    // // EasyServer server(port2, "TCP");

    // int id = 0;

    // while (true)
    // {
    //     cv::Mat mat = easy_event.captureScreen();
    //     resize(mat, mat, cv::Size(), 0.7, 0.7);
    //     printf("size = %d\n", mat.rows * mat.cols * 3);

    //     std::vector<uchar> buf;
    //     compressImage(mat, buf, 70);

    //     PacketBox box;
    //     BufToPacketBox(buf, box, ++id, 'I', 128);

    //     for (int i = 0; i < (int) box.packets.size(); i++) {
    //         client.sendData((char*)box.packets[i].data(), box.packets[i].size());
    //     }
    //     cv::waitKey(20);
    // }

    cleanKeyMapping();

    return 0;
}
