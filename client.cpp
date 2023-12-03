#include <stdio.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
#include "EasyLibs/EasyImgui.h"
#include "EasyLibs/EasyKeyCode.h"

int main(int argc, char** argv)
{
    windowTitle = (char*)"Client";

    windowWidth  = 310;
    windowHeight = 120;

    initEasySocket();
    initEasyImgui();

    EasyServer server;
    EasyClient client;
    EasyEvent easy_event;

    char passcode[5] = "0000";
    char port1[6] = "3402";
    char port2[6] = "3403";
    char host[16] = "127.0.0.1";

    std::thread socketThread1;
    std::thread socketThread2;
    BoxManager boxman;

    srand(time(NULL));

    bool quit = false;
    bool wait = false;
    int connection_phase = 0;

    char debug[256] = "Debug message";

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

        if (!wait && !connection_phase) {
            // Open UDP socket to wait for connect from server
            ImGui::Text("Open a port1 to wait for a connection");
            ImGui::PushItemWidth(200);
            ImGui::InputText("##port1", (char*)port1, 6);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                // Wait for a connection
                server.elisten(port1, "UDP");

                server.setService(
                    [&host, &wait, &connection_phase, &passcode](SOCKET sock, char data[], int size, char ipv4[]) {
                        if (!strcmp(data, passcode) || !strcmp(data, "0000")) {
                            strcpy(host, ipv4);
                            wait = false;
                            connection_phase = 1;
                        }
                    }
                );

                socketThread1 = std::thread([&server, &quit, &wait](){
                    while (!quit && wait) {
                        server.UDPReceive();
                    }
                });

                for (int i = 0; i < 4; i++) {
                    passcode[i] = rand() % 10 + '0';
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
            ImGui::Text("Code: %s", passcode);
            ImGui::SetCursorPos(ImVec2(211, 50));

            if (ImGui::Button("Cancel")) {
                wait = false;
                server.eclose();
                socketThread1.join();
            }
        }
        else if (connection_phase) {
            // When client is connected from server
            if (connection_phase == 1) {
                server.eclose();
                socketThread1.join();

                while (!client.econnect(host, port2, "TCP"));

                boxman.setCompleteCallback(
                    [&easy_event, &debug](PacketBox& box) {
                        std::vector<uchar> buf;
                        PacketBoxToBuf(box, buf);
                        if (box.type == 'K') {
                            KeyboardEvent *ke = (KeyboardEvent*)buf.data();
                            sprintf(debug, "Keyboard Event %d\n", SDLKeycodeToOSKeyCode(ke->keyCode));
                            if (ke->type == KeyDown) {
                                easy_event.sendKeyDown(SDLKeycodeToOSKeyCode(ke->keyCode));
                            }
                            else if (ke->type == KeyUp) {
                                easy_event.sendKeyUp(SDLKeycodeToOSKeyCode(ke->keyCode));
                            }
                        }
                        // else
                        // if (box.type == 'M') {
                        //     MouseEvent *me = (MouseEvent*)buf.data();

                        //     printf("Mouse move %lf %lf\n", me->x, me->y);

                        //     int x = me->x * easy_event.width;
                        //     int y = me->y * easy_event.height;
                            
                        //     if (me->type == LDown) {
                        //         easy_event.sendLDown(x, y);
                        //     }
                        //     else if (me->type == LUp) {
                        //         easy_event.sendLUp(x, y);
                        //     }
                        //     // else if (me->type == RDown) {
                        //     //     easy_event.sendRDown(x, y);
                        //     // }
                        //     // else if (me->type == RUp) {
                        //     //     easy_event.sendRUp(x, y);
                        //     // }
                        //     // else if (me->type == MouseMove) {
                        //     //     easy_event.sendMove(x, y);
                        //     // }
                        // }
                    }
                );

                server.setService(
                    [&boxman](SOCKET sock, char data[], int size, char host[]) {
                        std::vector<uchar> buf(data, data + size);
                        boxman.addPacketToBox(buf);
                    }
                );

                socketThread2 = std::thread([&server, &port1, &quit](){
                    server.elisten(port1, "UDP");

                    while (!quit) {
                        server.UDPReceive();
                    }
                });

                socketThread1 = std::thread([&quit, &easy_event, &client]() {

                    int id = 0;

                    while (!quit)
                    {
                        cv::Mat mat = easy_event.captureScreen();
                        resize(mat, mat, cv::Size(), 0.7, 0.7);
                        // printf("size = %d\n", mat.rows * mat.cols * 3);

                        std::vector<uchar> buf;
                        compressImage(mat, buf, 70);

                        // FILE *out = fopen("image_client.jpg", "wb");
                        // fwrite(buf.data(), buf.size(), 1, out);
                        // fclose(out);

                        PacketBox box;
                        BufToPacketBox(buf, box, ++id, 'I', TCP_MAX_BYTES);

                        for (int i = 0; i < (int) box.packets.size(); i++) {
                            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));

                        // exit(0);
                    }

                });

            }
            connection_phase = 2;
            ImGui::Text("%s", debug);
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

    socketThread1.join();
    socketThread2.join();

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
