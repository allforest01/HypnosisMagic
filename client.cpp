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

#ifdef WINDOWS
    #define cur_os win
#else
    #define cur_os mac
#endif

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
    EasyClient client;
    EasyEvent easy_event;

    char code[5] = "0000";
    char port[6] = "3402";
    char port2[] = "3403";
    char host[16] = "127.0.0.1";

    std::thread socketThread;
    std::thread socketThread2;
    BoxManager boxman;

    srand(time(NULL));

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
                        if (!strcmp(data, code) || !strcmp(data, "0000")) {
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

                boxman.setCompleteCallback(
                    [&easy_event](PacketBox& box) {
                        std::vector<uchar> buf;
                        PacketBoxToBuf(box, buf);
                        if (box.type == 'K') {
                            std::lock_guard<std::mutex> lock(mtx);
                            KeyboardEvent *ke = (KeyboardEvent*)buf.data();
                            printf("Keyboard Event %d\n", ke->keyCode);
                            // if (ke->type == KeyDown) {
                            //     easy_event.sendKeyDown(ke->keyos, ke->keyCode);
                            // }
                            // else if (ke->type == KeyUp) {
                            //     easy_event.sendKeyUp(ke->keyos, ke->keyCode);
                            // }
                        }
                        else if (box.type == 'M') {
                            std::lock_guard<std::mutex> lock(mtx);
                            MouseEvent *me = (MouseEvent*)buf.data();
                            printf("Mouse move %f %f\n", me->x, me->y);
                            // int x = me->x ...
                            // int y = me->y ...
                            // if (me->type == LDown) {
                            //     easy_event.sendLDown(me->x, me->y);
                            // }
                            // else if (me->type == LUp) {
                            //     easy_event.sendLUp(me->x, me->y);
                            // }
                            // else if (me->type == RDown) {
                            //     easy_event.sendRDown(me->x, me->y);
                            // }
                            // else if (me->type == RUp) {
                            //     easy_event.sendRUp(me->x, me->y);
                            // }
                            // else if (me->type == MouseMove) {
                            //     easy_event.sendMove(me->x, me->y);
                            // }
                        }
                    }
                );

                server.setService(
                    [&boxman](SOCKET sock, char data[], int size, char host[]) {
                        std::vector<uchar> buf(data, data + size);
                        boxman.addPacketToBox(buf);
                    }
                );

                // server.elisten(port, "UDP");

                socketThread2 = std::thread([&server, &quit](){
                    while (!quit) {
                        server.UDPReceive();
                    }
                });

                client.econnect(host, port2, "TCP");

                socketThread = std::thread([&quit, &easy_event, &client]() {

                    int id = 0;

                    while (!quit)
                    {
                        cv::Mat mat = easy_event.captureScreen();
                        resize(mat, mat, cv::Size(), 1, 1);
                        // printf("size = %d\n", mat.rows * mat.cols * 3);

                        std::vector<uchar> buf;
                        compressImage(mat, buf, 70);

                        FILE *out = fopen("image_client.jpg", "wb");
                        fwrite(buf.data(), buf.size(), 1, out);
                        fclose(out);

                        PacketBox box;
                        BufToPacketBox(buf, box, ++id, 'I', MAX_BYTES);

                        for (int i = 0; i < (int) box.packets.size(); i++) {
                            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));

                        // exit(0);
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
    socketThread2.join();

    cleanEasyImgui();
    cleanEasySocket();
    cleanKeyMapping();

    return 0;
}
