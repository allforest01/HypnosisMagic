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

#define port_passcoode "3401"
#define port_screen    "3402"
#define port_mouse     "3403"
#define port_keyboard  "3404"

char host[16] = "127.0.0.1";
char passcode[5] = "0000";

EasyClient client_passcode;
EasyClient client_mouse;
EasyClient client_keyboard;
EasyServer server_screen;

// std::thread thread_passcode;
std::thread thread_screen;
std::thread thread_mouse;
std::thread thread_keyboard;

BoxManager boxman_screen;

std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_mouse, mtx_keyboard;

bool quit = false, connected = false;
bool isHovered = false, isFocused = false;

GLuint image_texture;
std::vector<uchar> buf;
int width, height, channels;
std::queue<std::vector<uchar>> bufs;
int scaled_width, scaled_height;
int startX, startY;

void PushMouseEvent(MouseEvent me) {
    std::unique_lock<std::mutex> lock(mtx_mouse);
    mouse_events.push(me);
    mtx_mouse.unlock();
}

void PushKeyboardEvent(KeyboardEvent ke) {
    std::unique_lock<std::mutex> lock(mtx_keyboard);
    keyboard_events.push(ke);
    mtx_keyboard.unlock();
}

void HandleEvents() {
    // SDL poll event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        else if (connected && isHovered)
        {
            if (event.type == SDL_MOUSEMOTION) PushMouseEvent(MouseEvent(MouseMove, event.button.x - startX,  event.button.y - startY));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LDown, event.button.x - startX,  event.button.y - startY));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RDown, event.button.x - startX,  event.button.y - startY));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LUp, event.button.x - startX,  event.button.y - startY));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RUp, event.button.x - startX,  event.button.y - startY));
            }
            else if (event.type == SDL_KEYDOWN) PushKeyboardEvent(KeyboardEvent(KeyDown, event.key.keysym.sym));
            else if (event.type == SDL_KEYUP) PushKeyboardEvent(KeyboardEvent(KeyUp, event.key.keysym.sym));
        }
    }
}

void ConnectButtonHandle() {
    // Send passcode
    client_passcode.econnect(host, port_passcoode, "UDP");
    client_passcode.sendData(passcode, 5);
    client_passcode.eclose();

    // Receive screen capture
    thread_screen = std::thread([]()
    {
        boxman_screen.setCompleteCallback([](PacketBox& box) {
            printf("%d\n", box.packets.size());
            PacketBoxToBuf(box, buf);
            if (box.type == 'I') {
                // if (bufs.size() <= 2)
                bufs.push(buf);
            }
        });

        server_screen.setService(
            [](SOCKET sock, char data[], int size, char host[]) {
                std::vector<uchar> buf(data, data + size);
                boxman_screen.addPacketToBox(buf);
            }
        );

        server_screen.elisten(port_screen, "UDP");

        while (!quit) server_screen.UDPReceive(128);
    });

    thread_screen.detach();

    thread_mouse = std::thread([](){
        // client_mouse send mouse events
        while (!client_mouse.econnect(host, port_mouse, "TCP"));

        while (!quit) {
            if (mouse_events.size()) {
                std::unique_lock<std::mutex> lock(mtx_mouse);
                MouseEvent me = mouse_events.front(); mouse_events.pop();
                mtx_mouse.unlock();

                static int id = 0;
    
                me.x /= width;
                me.y /= height;

                if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;

                printf("Mouse event %lf %lf\n", me.x, me.y);

                std::vector<uchar> buf((char*)&me, (char*)(&me) + sizeof(me));
                // std::cout << "mouse buf.size() = " << buf.size() << '\n';
                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'M', buf.size() + 7);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_mouse.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
            }
        }
    });

    thread_mouse.detach();

    thread_keyboard = std::thread([](){
        // client_keyboard send keyboard events
        while (!client_keyboard.econnect(host, port_keyboard, "TCP"));

        while (!quit) {
            if (keyboard_events.size()) {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                KeyboardEvent ke = keyboard_events.front(); keyboard_events.pop();
                mtx_keyboard.unlock();

                static int id = 0;

                if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                std::vector<uchar> buf((char*)&ke, (char*)(&ke) + sizeof(ke));
                // std::cout << "keyboard buf.size() = " << buf.size() << '\n';
                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'K', buf.size() + 7);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_keyboard.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
            }
        }
    });

    thread_keyboard.detach();

    // Connected!
    connected = true;
}

void MenuBar() {
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
}

void ConnectToClientWindow() {
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
    ImGui::InputText("##port_passcode", (char*)port_passcoode, 6);
    ImGui::PopItemWidth();

    ImGui::Text("Code");
    ImGui::SameLine();
    ImGui::PushItemWidth(135);
    ImGui::InputText("##passcode", (char*)passcode, 5);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Connect")) ConnectButtonHandle();

    ImGui::End();
}

void ScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(922, 600));

    ImGui::Begin("Screen");

    if (bufs.size()) {
        auto cur_buf = bufs.front(); bufs.pop();

        unsigned char* imageData = stbi_load_from_memory(cur_buf.data(), cur_buf.size(), &width, &height, &channels, 3);

        ImVec2 window_avail_size = ImGui::GetContentRegionAvail() - ImVec2(0, 6);
        float window_aspect = window_avail_size.x / window_avail_size.y;
        float image_aspect = (float) width / height;

        float scale;
        if (window_aspect > image_aspect) {
            // Window is wider than the image
            scale = window_avail_size.y / height;
        } else {
            // Window is narrower than the image
            scale = window_avail_size.x / width;
        }

        scaled_width = width * scale;
        scaled_height = height * scale;

        ImageToTexture(imageData, image_texture, width, height, channels);

        stbi_image_free(imageData);
    }

    ImGui::ImageButton((void*)(intptr_t)image_texture, ImVec2(scaled_width, scaled_height));

    isHovered = ImGui::IsItemHovered();
    isFocused = ImGui::IsItemFocused();

    startX = ImGui::GetItemRectMin().x;
    startY = ImGui::GetItemRectMin().y;

    if (!isHovered || !isFocused) {
        std::queue<MouseEvent>().swap(mouse_events);
        std::queue<KeyboardEvent>().swap(keyboard_events);
    }

    ImGui::End();
}

void MouseEventInfoWindow() {
    ImGui::SetNextWindowPos(ImVec2(940, 140));
    ImGui::SetNextWindowSize(ImVec2(255, 100));

    ImGui::Begin("Mouse event info", NULL, ImGuiWindowFlags_NoMove);

    ImGui::Text("Is mouse over screen? %s", isHovered ? "Yes" : "No");
    ImGui::Text("Is screen focused? %s", isFocused ? "Yes" : "No");

    ImGui::End();
}

void StartNewFrame() {
    // Start a new ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void Rendering() {
    // Rendering
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    SDL_GL_SwapWindow(window);

    // Introduce a delay to reduce CPU usage
    SDL_Delay(16);
}

int main(int argc, char** argv)
{
    windowTitle  = (char*)"server_screen";
    windowWidth  = 1200;
    windowHeight = 640;

    initEasySocket();
    initEasyImgui();

    glGenTextures(1, &image_texture);

    while (!quit)
    {
        StartNewFrame();

        MenuBar();
        ConnectToClientWindow();
        ScreenWindow();
        MouseEventInfoWindow();

        Rendering();
        HandleEvents();
    }

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
