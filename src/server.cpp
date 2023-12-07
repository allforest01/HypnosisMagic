#include "../include/port.h"

#include <stdio.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>

#include "../lib/hypno/hypno_socket.h"
#include "../lib/hypno/hypno_event.h"
#include "../lib/hypno/hypno_image.h"
#include "../lib/hypno/hypno_data.h"
#include "../lib/hypno/hypno_keycode.h"

#include "../include/imgui_wrapper.h"
#include "../include/frame_wrapper.h"

char host[16] = "10.211.55.255";
char passcode[7] = "ABCXYZ";

HypnoClient client_passcode;
HypnoServer server_passcode;
HypnoClient client_mouse;
HypnoClient client_keyboard;
HypnoServer server_screen;

std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_mouse, mtx_keyboard;
bool is_hovered = false, is_focused = false;

bool quit = false, waiting = false, connected = false;

ImGuiWrapper imgui_wrapper;
FrameWrapper frame_wrapper;

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
        else if (connected && is_hovered)
        {
            if (event.type == SDL_MOUSEMOTION) PushMouseEvent(MouseEvent(MouseMove, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LDown, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RDown, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LUp, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RUp, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            }
            else if (event.type == SDL_KEYDOWN) PushKeyboardEvent(KeyboardEvent(KeyDown, event.key.keysym.sym));
            else if (event.type == SDL_KEYUP) PushKeyboardEvent(KeyboardEvent(KeyUp, event.key.keysym.sym));
        }
    }
}

void ConnectButtonHandle() {
    // Send passcode
    std::thread thread_passcode([&]()
    {
        broadcastMessage(PORT_PASSCODE, passcode, 7, inet_addr(host));

        server_passcode.hypnoListen(PORT_PASSCODE, "TCP");

        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(server_passcode.client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
        strcpy(host, ipv4);
        printf("host = %s\n", host);
        fflush(stdout);

        server_passcode.hypnoClose();

        printf("Start thread_mouse\n");

        // client_mouse send mouse events
        while (!client_mouse.hypnoConnect(host, PORT_MOUSE, "TCP"));

        printf("Start thread_keyboard\n");

        // client_mouse send mouse events
        while (!client_keyboard.hypnoConnect(host, PORT_KEYBOARD, "TCP"));

        std::thread thread_mouse([&](){
            while (!quit) {
                if (mouse_events.size()) {
                    std::unique_lock<std::mutex> lock(mtx_mouse);
                    MouseEvent me = mouse_events.front(); mouse_events.pop();
                    mtx_mouse.unlock();

                    static int id = 0;
        
                    me.x /= frame_wrapper.width;
                    me.y /= frame_wrapper.height;

                    if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;

                    printf("Mouse event %lf %lf\n", me.x, me.y);

                    std::vector<uchar> image_data((char*)&me, (char*)(&me) + sizeof(me));
                    // std::cout << "mouse image_data.size() = " << image_data.size() << '\n';
                    PacketBox box;
                    BufToPacketBox(image_data, box, ++id, 'M', image_data.size() + 7);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        client_mouse.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }
                }
            }
        });

        thread_mouse.detach();

        std::thread thread_keyboard([&](){
            while (!quit) {
                if (keyboard_events.size()) {
                    std::unique_lock<std::mutex> lock(mtx_keyboard);
                    KeyboardEvent ke = keyboard_events.front(); keyboard_events.pop();
                    mtx_keyboard.unlock();

                    static int id = 0;

                    if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                    else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                    std::vector<uchar> image_data((char*)&ke, (char*)(&ke) + sizeof(ke));
                    // std::cout << "keyboard image_data.size() = " << image_data.size() << '\n';
                    PacketBox box;
                    BufToPacketBox(image_data, box, ++id, 'K', image_data.size() + 7);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        client_keyboard.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }
                }
            }
        });

        thread_keyboard.detach();

        // Receive screen capture
        std::thread thread_screen([&]()
        {
            printf("Start thread_screen\n");

            BoxManager boxman_screen;

            boxman_screen.setCompleteCallback([](PacketBox& box) {
                printf("%d\n", box.packets.size());
                std::vector<uchar> image_data;
                PacketBoxToBuf(box, image_data);
                if (box.type == 'I') {
                    // if (frame_wrapper.frame_queue.size() <= 2)
                    frame_wrapper.frame_queue.push(image_data);
                }
            });

            server_screen.setService(
                [&boxman_screen](SOCKET sock, char data[], int size, char host[]) {
                    short id = *(data);
                    short num = *(short*)(data + 3);
                    // printf("id = %d, num = %d\n", id, num);
                    std::vector<uchar> image_data(data, data + size);
                    boxman_screen.addPacketToBox(image_data);
                }
            );

            server_screen.hypnoListen(PORT_SCREEN, "UDP");

            while (!quit) server_screen.UDPReceive(128);
        });

        thread_screen.detach();

        // Connected
        connected = true;
    });

    thread_passcode.detach();
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
    ImGui::InputText("##PORT_PASSCODE", (char*)PORT_PASSCODE, 6);
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

    if (frame_wrapper.frame_queue.size()) frame_wrapper.pushToTexture();

    ImGui::ImageButton((void*)(intptr_t)frame_wrapper.image_texture, ImVec2(frame_wrapper.scaled_width, frame_wrapper.scaled_height));

    is_hovered = ImGui::IsItemHovered();
    is_focused = ImGui::IsItemFocused();

    frame_wrapper.start_x = ImGui::GetItemRectMin().x;
    frame_wrapper.start_y = ImGui::GetItemRectMin().y;

    if (!is_hovered || !is_focused) {
        std::unique_lock<std::mutex> lock_mouse(mtx_mouse);
        std::queue<MouseEvent>().swap(mouse_events);
        lock_mouse.unlock();

        std::unique_lock<std::mutex> lock_keyboard(mtx_mouse);
        std::queue<KeyboardEvent>().swap(keyboard_events);
        lock_keyboard.unlock();
    }

    ImGui::End();
}

void MouseEventInfoWindow() {
    ImGui::SetNextWindowPos(ImVec2(940, 140));
    ImGui::SetNextWindowSize(ImVec2(255, 100));

    ImGui::Begin("Mouse event info", NULL, ImGuiWindowFlags_NoMove);

    ImGui::Text("Is mouse over screen? %s", is_hovered ? "Yes" : "No");
    ImGui::Text("Is screen focused? %s", is_focused ? "Yes" : "No");

    ImGui::End();
}

void StartNewFrame() {
    // Start a new ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(imgui_wrapper.window);
    ImGui::NewFrame();
}

void Rendering() {
    // Rendering
    glViewport(0, 0, imgui_wrapper.window_width, imgui_wrapper.window_height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    SDL_GL_SwapWindow(imgui_wrapper.window);

    // Introduce a delay to reduce CPU usage
    SDL_Delay(16);
}

int main(int argc, char** argv)
{
    initHypnoSocket();

    imgui_wrapper = ImGuiWrapper(1200, 640, (char*)"Server");

    initImGui(imgui_wrapper);

    frame_wrapper.initTexture();

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

    cleanImGui(imgui_wrapper);

    frame_wrapper.cleanTexture();

    cleanHypnoSocket();

    return 0;
}
