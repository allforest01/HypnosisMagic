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

#define port_passcode "3401"
#define port_screen   "3402"
#define port_mouse    "3403"
#define port_keyboard "3404"

char host[16] = "10.211.55.255";
char passcode[7] = "ABCXYZ";

EasyClient client_passcode;
EasyServer server_passcode;
EasyClient client_mouse;
EasyClient client_keyboard;
EasyServer server_screen;

BoxManager boxman_screen;
std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_mouse, mtx_keyboard;

bool quit = false, waiting = false, connected = false;
bool is_hovered = false, is_focused = false;

GLuint image_texture;
std::vector<uchar> image_data;
std::queue<std::vector<uchar>> queue_image_data;
int image_width, image_height, image_channels;
int image_scaled_width, image_scaled_height;
int image_start_x, image_start_y;

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
            if (event.type == SDL_MOUSEMOTION) PushMouseEvent(MouseEvent(MouseMove, event.button.x - image_start_x,  event.button.y - image_start_y));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LDown, event.button.x - image_start_x,  event.button.y - image_start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RDown, event.button.x - image_start_x,  event.button.y - image_start_y));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) PushMouseEvent(MouseEvent(LUp, event.button.x - image_start_x,  event.button.y - image_start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) PushMouseEvent(MouseEvent(RUp, event.button.x - image_start_x,  event.button.y - image_start_y));
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
        broadcast(port_passcode, passcode, 7, inet_addr(host));

        server_passcode.elisten(port_passcode, "TCP");

        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(server_passcode.client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
        strcpy(host, ipv4);
        printf("host = %s\n", host);
        fflush(stdout);

        server_passcode.eclose();

        printf("Start thread_mouse\n");

        // client_mouse send mouse events
        while (!client_mouse.econnect(host, port_mouse, "TCP"));

        printf("Start thread_keyboard\n");

        // client_mouse send mouse events
        while (!client_keyboard.econnect(host, port_keyboard, "TCP"));

        std::thread thread_mouse([&](){
            while (!quit) {
                if (mouse_events.size()) {
                    std::unique_lock<std::mutex> lock(mtx_mouse);
                    MouseEvent me = mouse_events.front(); mouse_events.pop();
                    mtx_mouse.unlock();

                    static int id = 0;
        
                    me.x /= image_width;
                    me.y /= image_height;

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

            boxman_screen.setCompleteCallback([](PacketBox& box) {
                printf("%d\n", box.packets.size());
                PacketBoxToBuf(box, image_data);
                if (box.type == 'I') {
                    // if (queue_image_data.size() <= 2)
                    queue_image_data.push(image_data);
                }
            });

            server_screen.setService(
                [](SOCKET sock, char data[], int size, char host[]) {
                    short id = *(data);
                    short num = *(short*)(data + 3);
                    // printf("id = %d, num = %d\n", id, num);
                    std::vector<uchar> image_data(data, data + size);
                    boxman_screen.addPacketToBox(image_data);
                }
            );

            server_screen.elisten(port_screen, "UDP");

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
    ImGui::InputText("##port_passcode", (char*)port_passcode, 6);
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

    if (queue_image_data.size()) {
        auto cur_buf = queue_image_data.front(); queue_image_data.pop();

        unsigned char* imageData = stbi_load_from_memory(cur_buf.data(), cur_buf.size(), &image_width, &image_height, &image_channels, 3);

        ImVec2 window_avail_size = ImGui::GetContentRegionAvail() - ImVec2(0, 6);
        float window_aspect = window_avail_size.x / window_avail_size.y;
        float image_aspect = (float) image_width / image_height;

        float scale;
        if (window_aspect > image_aspect) {
            // Window is wider than the image
            scale = window_avail_size.y / image_height;
        } else {
            // Window is narrower than the image
            scale = window_avail_size.x / image_width;
        }

        image_scaled_width = image_width * scale;
        image_scaled_height = image_height * scale;

        ImageToTexture(imageData, image_texture, image_width, image_height, image_channels);

        stbi_image_free(imageData);
    }

    ImGui::ImageButton((void*)(intptr_t)image_texture, ImVec2(image_scaled_width, image_scaled_height));

    is_hovered = ImGui::IsItemHovered();
    is_focused = ImGui::IsItemFocused();

    image_start_x = ImGui::GetItemRectMin().x;
    image_start_y = ImGui::GetItemRectMin().y;

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
