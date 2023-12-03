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

std::mutex mtx;

int main(int argc, char** argv)
{
    windowTitle = (char*)"Server";

    windowWidth = 1200;
    windowHeight = 640;

    initEasySocket();
    initEasyImgui();

    EasyClient client;
    EasyServer server;
    
    char passcode[5] = "0000";
    char port1[6] = "3402";
    char port2[6] = "3403";
    char host[16] = "127.0.0.1";

    std::thread socketThread1;
    BoxManager boxman;
    GLuint image_texture;
    std::vector<uchar> buf;
    int width, height, channels, id = 0;
    std::queue<std::vector<uchar>> bufs;
    auto lastMove = std::chrono::system_clock::now();

    glGenTextures(1, &image_texture);

    bool connected = false;
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
            else if (connected)
            {
                if (event.type == SDL_KEYDOWN)
                {
                    // Handle key press events
                    std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << " (Code: " << static_cast<int>(event.key.keysym.sym) << ")" << std::endl;

                    int keyCode = static_cast<int>(event.key.keysym.sym);
                    KeyboardEvent ke(KeyDown, keyCode);

                    std::vector<uchar> buf((char*)&ke, (char*)(&ke + sizeof(ke)));
                    PacketBox box;
                    BufToPacketBox(buf, box, ++id, 'K', 24);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }

                }
                else if (event.type == SDL_KEYUP)
                {
                    // Handle key release events
                    std::cout << "Key released: " << SDL_GetKeyName(event.key.keysym.sym) << " (Code: " << static_cast<int>(event.key.keysym.sym) << ")" << std::endl;

                    int keyCode = static_cast<int>(event.key.keysym.sym);
                    KeyboardEvent ke(KeyUp, keyCode);

                    std::vector<uchar> buf((char*)&ke, (char*)(&ke + sizeof(ke)));
                    PacketBox box;
                    BufToPacketBox(buf, box, ++id, 'K', 24);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        double x = (double) (event.button.x - 20) / width;
                        double y = (double) (event.button.y - 60) / height;

                        printf("Mouse left down! %lf %lf\n", x, y);

                        MouseEvent me(LDown, x, y);

                        std::vector<uchar> buf((char*)&me, (char*)(&me + sizeof(me)));
                        PacketBox box;
                        BufToPacketBox(buf, box, ++id, 'M', 24);

                        for (int i = 0; i < (int) box.packets.size(); i++) {
                            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                        }
                    }
                }
                else if (event.type == SDL_MOUSEBUTTONUP)
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        double x = (double) (event.button.x - 20) / width;
                        double y = (double) (event.button.y - 60) / height;

                        printf("Mouse left up! %lf %lf\n", x, y);

                        MouseEvent me(LUp, x, y);

                        std::vector<uchar> buf((char*)&me, (char*)(&me + sizeof(me)));
                        PacketBox box;
                        BufToPacketBox(buf, box, ++id, 'M', 24);

                        for (int i = 0; i < (int) box.packets.size(); i++) {
                            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                        }
                    }
                }
                else if (event.type == SDL_MOUSEMOTION)
                {
                    // auto passedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastMove).count();
                    // if (passedTime < 100) continue;
                    // printf("%d\n", passedTime);

                    double x = (double) (event.button.x - 20) / width;
                    double y = (double) (event.button.y - 60) / height;

                    printf("Mouse move! %lf %lf\n", x, y);

                    MouseEvent me(MouseMove, x, y);

                    std::vector<uchar> buf((char*)&me, (char*)(&me + sizeof(me)));
                    PacketBox box;
                    BufToPacketBox(buf, box, ++id, 'M', 24);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        client.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }

                    // lastMove = std::chrono::system_clock::now();
                }
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
        ImGui::InputText("##port1", (char*)port1, 6);
        ImGui::PopItemWidth();

        ImGui::Text("Code");
        ImGui::SameLine();
        ImGui::PushItemWidth(135);
        ImGui::InputText("##passcode", (char*)passcode, 5);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Connect"))
        {
            client.econnect(host, port1, "UDP");
            client.sendData(passcode, 5);
            client.eclose();

            boxman.setCompleteCallback(
                [&buf, &bufs](PacketBox& box) {
                    PacketBoxToBuf(box, buf);
                    if (box.type == 'I')
                    {
                        // while (bufs.size()) bufs.pop();
                        if (bufs.size() <= 2) {
                            // std::lock_guard<std::mutex> lock(mtx);
                            bufs.push(buf);
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

            socketThread1 = std::thread([&server, &port2, &quit](){
                server.elisten(port2, "TCP");

                while (!quit) {
                    server.TCPReceive(1440);
                }
            });

            while (!client.econnect(host, port1, "TCP"));

            connected = true;
        }

        ImGui::End();

        // GUI when connect to client
        if (bufs.size()) {
            // std::lock_guard<std::mutex> lock(mtx);
            auto cur_buf = bufs.front(); bufs.pop();
            BufToTexture(cur_buf, image_texture, width, height, channels);
        }

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

        // Rendering
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    socketThread1.join();

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
