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
    windowTitle = (char*)"Server";

    windowWidth = 1200;
    windowHeight = 640;

    initKeyMapping();
    initEasySocket();
    initEasyImgui();

    EasyClient client;
    EasyServer server;
    EasyEvent easy_event;
    
    char code[5] = "0000";
    char port[6] = "3402";
    char port2[] = "3403";
    char host[16] = "127.0.0.1";

    std::thread socketThread;
    BoxManager boxman;
    GLuint image_texture;
    std::vector<uchar> buf;
    std::queue<std::vector<uchar>> bufs;
    int id = 0;
    int width, height, channels;

    easy_event.setKeyDownCallback([&client, &id](int keyCode) {
        printf("Keyboard send %d!\n", keyCode);

        KeyboardEvent ke(cur_os, KeyDown, keyCode);

        std::vector<uchar> buf((char*)&ke, (char*)(&ke + sizeof(ke)));
        PacketBox box;
        BufToPacketBox(buf, box, ++id, 'K', MAX_BYTES);

        for (int i = 0; i < (int) box.packets.size(); i++) {
            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
        }
    });

    easy_event.setKeyUpCallback([&client, &id](int keyCode) {
        printf("Keyboard send %d!\n", keyCode);

        KeyboardEvent ke(cur_os, KeyUp, keyCode);

        std::vector<uchar> buf((char*)&ke, (char*)(&ke + sizeof(ke)));
        PacketBox box;
        BufToPacketBox(buf, box, ++id, 'K', MAX_BYTES);

        for (int i = 0; i < (int) box.packets.size(); i++) {
            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
        }
    });

    // easy_event.startHook();

    bool quit = false;

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
        bool about_popup = false;

        // Menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::MenuItem("About")) {
                about_popup = true;
            }
            if (ImGui::MenuItem("Exit")) {
                quit = true;
            }
            ImGui::EndMainMenuBar();
        }

        // About popup
        if (about_popup) {
            ImGui::OpenPopup("About");
        }
        if (ImGui::BeginPopup("About", ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Simple cross-platform remote desktop tool in LAN");
            ImGui::Separator();
            ImGui::Text("Made by: allforest01");
            ImGui::EndPopup();
        }

        ImGui::SetNextWindowPos(ImVec2(940, 30));
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
        if (ImGui::Button("Connect"))
        {
            client.econnect(host, port, "UDP");
            client.sendData(code, 5);
            client.eclose();

            boxman.setCompleteCallback(
                [&buf, &bufs](PacketBox& box) {
                    PacketBoxToBuf(box, buf);
                    // while (bufs.size()) bufs.pop();
                    if (bufs.empty()) {
                        std::lock_guard<std::mutex> lock(mtx);
                        bufs.push(buf);
                        printf("pushed!\n");
                    }
                    if (box.type == 'I')
                    {
                        // std::lock_guard<std::mutex> lock(mtx);

                        FILE *out = fopen("image_server.jpg", "wb");
                        fwrite(buf.data(), buf.size(), 1, out);
                        fclose(out);

                        // imageChanged = false;
                        // decompressImage(buf, image);
                        // imageChanged = !image.empty();
                        // imageChanged = true;

                        // exit(0);
                    }
                }
            );

            server.setService(
                [&boxman](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman.addPacketToBox(buf);
                }
            );

            server.elisten(port2, "TCP");

            socketThread = std::thread([&server, &quit](){
                while (!quit) {
                    server.TCPReceive();
                }
            });

            // client.econnect(host, port, "UDP");
        }

        ImGui::End();

        // GUI when connect to client
        // std::lock_guard<std::mutex> lock(mtx);
        if (bufs.size()) {
            printf("bufs.size() = %d\n", bufs.size());
            GLuint old_texture = image_texture;
            glGenTextures(1, &image_texture);
            bool exception_caught = false;
            try {
                // MatToTexture(image, image_texture);
                // while (bufs.size() > 1) bufs.pop();
                std::lock_guard<std::mutex> lock(mtx);
                auto cur_buf = bufs.front(); bufs.pop();
                BufToTexture(cur_buf, image_texture, width, height, channels);
                // FILE *out = fopen("image_server.jpg", "wb");
                // fwrite(buf.data(), buf.size(), 1, out);
                // fclose(out);
            }
            catch (...) {
                glDeleteTextures(1, &image_texture);
                image_texture = old_texture;
                exception_caught = true;
            }
            if (!exception_caught) {
                glDeleteTextures(1, &old_texture);
            }
        }
        // if (image.rows) {
        //     cv::imshow("image", image);
        //     cv::waitKey(1);
        // }

        ImGui::SetNextWindowPos(ImVec2(10, 30));
        ImGui::SetNextWindowSize(ImVec2(922, 600));

        ImGui::Begin("Screen");

        ImVec2 window_avail_size = ImGui::GetContentRegionAvail() - ImVec2(0, 6);
        float window_aspect = window_avail_size.x / window_avail_size.y;
        float image_aspect = width / (float)height;

        float scale;
        if (window_aspect > image_aspect) {
            // Window is wider than the image
            scale = window_avail_size.y / height;
        } else {
            // Window is narrower than the image
            scale = window_avail_size.x / width;
        }

        float scaled_width = width * scale;
        float scaled_height = height * scale;

        ImGui::ImageButton((void*)(intptr_t)image_texture, ImVec2(scaled_width, scaled_height));

        bool isHovered = ImGui::IsItemHovered();
        bool isFocused = ImGui::IsItemFocused();
        bool leftClicked = ImGui::IsMouseDown(ImGuiMouseButton_Left);
        ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
        ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
        ImVec2 mousePositionRelative = mousePositionAbsolute - screenPositionAbsolute;

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(940, 140));
        ImGui::SetNextWindowSize(ImVec2(255, 100));

        ImGui::Begin("Mouse event info", NULL, ImGuiWindowFlags_NoMove);

        ImGui::Text("Is mouse over screen? %s", isHovered ? "Yes" : "No");
        ImGui::Text("Is screen focused? %s", isFocused ? "Yes" : "No");
        ImGui::Text("Mouse Position: (%.2f, %.2f)", mousePositionRelative.x, mousePositionRelative.y);
        ImGui::Text("Mouse clicked: %s", leftClicked ? "Yes" : "No");

        ImGui::End();

        // // Event processing
        // if (isFocused)
        // {
        //     easy_event.msgLoop();

        //     printf("Mouse send! %f %f\n", mousePositionRelative.x, mousePositionRelative.y);

        //     MouseEvent me(MouseMove, mousePositionRelative.x, mousePositionRelative.y);

        //     std::vector<uchar> buf((char*)&me, (char*)(&me + sizeof(me)));
        //     PacketBox box;
        //     BufToPacketBox(buf, box, ++id, 'M', 256);

        //     for (int i = 0; i < (int) box.packets.size(); i++) {
        //         client.sendData((char*)box.packets[i].data(), box.packets[i].size());
        //     }
        // }

        // Rendering
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // easy_event.stopHook();

    socketThread.join();

    cleanEasyImgui();
    cleanEasySocket();
    cleanKeyMapping();

    return 0;
}
