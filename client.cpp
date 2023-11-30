#include <stdio.h>
#include <thread>
#include <mutex>
#include <random>

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
#include "EasyLibs/EasyImgui.h"

std::mutex mtx;

int main(int argc, char** argv)
{
    windowTitle = (char*)"Client";
    windowWidth  = 310;
    windowHeight = 120;

    initKeyMapping();
    initEasySocket();
    initEasyImgui();

    EasyServer server;
    char code[5] = "0000";
    char port[6] = "3402";
    char host[16] = "127.0.0.1";

    srand(time(NULL));

    std::thread socketThread;

    bool quit = false;
    bool wait = false;
    int conn = 0;

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

        if (!wait && !conn) {
            // Open UDP socket to wait for connect from server
            ImGui::Text("Open a port to wait for a connection");
            ImGui::PushItemWidth(200);
            ImGui::InputText("##port", (char*)port, 6);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                // Wait for a connection
                server.elisten(port, "UDP");
                server.setService(
                    [&host, &wait, &conn, &code](SOCKET sock, char data[], int size, char ipv4[]) {
                        // printf("ipv4 = %s", ipv4);
                        if (!strcmp(data, code)) {
                            strcpy(host, ipv4);
                            wait = false;
                            conn = 1;
                        }
                    }
                );
                socketThread = std::thread([&server, &quit, &wait](){
                    while (!quit && wait) {
                        server.UDPReceive();
                    }
                });
                for (int i = 0; i < 4; i++) {
                    code[i] = rand() % 10 + '0';
                }
                wait = true;
            }
        }
        else if (wait) {
            // Waiting for a connection from server
            ImGui::PushItemWidth(-1);
            ImGui::ProgressBar(ImGui::GetTime() * -0.2f, ImVec2(0, 0), "Waiting for a connection");
            ImGui::PopItemWidth();

            ImGui::SetCursorPos(ImVec2(9, 53));
            ImGui::Text("Code: %s", code);
            ImGui::SetCursorPos(ImVec2(211, 50));

            if (ImGui::Button("Cancel")) {
                wait = false;
                server.eclose();
                socketThread.join();
            }
        }
        else if (conn) {
            // When client is connected from server
            if (conn == 1) {
                server.eclose();
                socketThread.join();

                socketThread = std::thread([&host, &quit](){

                    char port2[] = "3403";
                    EasyEvent easy_event;

                    EasyClient client;
                    client.econnect(host, port2, "UDP");

                    int id = 0;

                    while (!quit)
                    {
                        cv::Mat mat = easy_event.captureScreen();
                        resize(mat, mat, cv::Size(), 0.7, 0.7);
                        printf("size = %d\n", mat.rows * mat.cols * 3);

                        std::vector<uchar> buf;
                        compressImage(mat, buf, 70);

                        PacketBox box;
                        BufToPacketBox(buf, box, ++id, 'I', 256);

                        for (int i = 0; i < (int) box.packets.size(); i++) {
                            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                        }
                        cv::waitKey(20);
                    }
        
                });

            }
            conn = 2;
            ImGui::Text("%s", host);
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

    socketThread.join();

    cleanEasyImgui();
    cleanEasySocket();
    cleanKeyMapping();

    return 0;
}
