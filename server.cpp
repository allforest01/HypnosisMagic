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

std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;

BoxManager boxman_screen;

bool quit = false, connected = false;

bool isHovered = false, isFocused = false;
GLuint image_texture;
std::vector<uchar> buf;
int width, height, channels;
std::queue<std::vector<uchar>> bufs;

void HandleEvents() {
    // SDL poll event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        else if (connected && isFocused && isHovered)
        {
            if (event.type == SDL_KEYDOWN) keyboard_events.push(KeyboardEvent(KeyDown, event.key.keysym.sym));
            else if (event.type == SDL_KEYUP) keyboard_events.push(KeyboardEvent(KeyUp, event.key.keysym.sym));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) mouse_events.push(MouseEvent(LDown, event.button.x - 20,  event.button.y - 60));
                else if (event.button.button == SDL_BUTTON_RIGHT) mouse_events.push(MouseEvent(RDown, event.button.x - 20,  event.button.y - 60));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) mouse_events.push(MouseEvent(LUp, event.button.x - 20,  event.button.y - 60));
                else if (event.button.button == SDL_BUTTON_RIGHT) mouse_events.push(MouseEvent(RUp, event.button.x - 20,  event.button.y - 60));
            }
            else if (event.type == SDL_MOUSEMOTION) mouse_events.push(MouseEvent(MouseMove, event.button.x - 20,  event.button.y - 60));
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
            PacketBoxToBuf(box, buf);
            if (box.type == 'I') {
                if (bufs.size() <= 2) bufs.push(buf);
            }
        });

        server_screen.setService(
            [](SOCKET sock, char data[], int size, char host[]) {
                std::vector<uchar> buf(data, data + size);
                boxman_screen.addPacketToBox(buf);
            }
        );

        server_screen.elisten(port_screen, "TCP");

        while (!quit) server_screen.TCPReceive(1440);
    });

    thread_screen.detach();

    thread_mouse = std::thread([](){
        // client_mouse send mouse events
        while (!client_mouse.econnect(host, port_mouse, "TCP"));

        while (!quit) {
            if (mouse_events.size()) {
                MouseEvent me = mouse_events.front(); mouse_events.pop();

                static int id = 0;
    
                me.x /= width;
                me.y /= height;

                printf("Mouse event %lf %lf\n", me.x, me.y);

                std::vector<uchar> buf((char*)&me, (char*)(&me) + sizeof(me));
                // std::cout << "mouse buf.size() = " << buf.size() << '\n';
                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'M', 26);

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
                KeyboardEvent ke = keyboard_events.front(); keyboard_events.pop();

                static int id = 0;

                if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                std::vector<uchar> buf((char*)&ke, (char*)(&ke) + sizeof(ke));
                // std::cout << "keyboard buf.size() = " << buf.size() << '\n';
                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'K', 14);

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

void ConvertImage() {
    if (bufs.size()) {
        auto cur_buf = bufs.front(); bufs.pop();
        BufToTexture(cur_buf, image_texture, width, height, channels);
    }
}

void ScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(922, 600));

    ImGui::Begin("Screen");

    ConvertImage();

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

    isHovered = ImGui::IsItemHovered();
    isFocused = ImGui::IsItemFocused();

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
        HandleEvents();
        StartNewFrame();

        MenuBar();
        ConnectToClientWindow();
        ScreenWindow();
        MouseEventInfoWindow();

        Rendering();
    }

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
