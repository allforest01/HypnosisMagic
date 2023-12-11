#define SECRET "aBcXyZ"

#define PORT_P "33330"
#define PORT_C "33331"

#include "server.h"

char host[16] = "10.211.55.255";

std::mutex mtx_mouse, mtx_keyboard;

ServerSocketManager server_passcode;
ClientSocketManager client_passcode;

ImGuiWrapper imgui_wrapper;
std::vector<ServerWrapper> server_wrappers;

bool quit = false;
bool connected = false;
int active_id = INT_MAX;

void pushMouseEvent(MouseEvent me) {
    std::unique_lock<std::mutex> lock(mtx_mouse);
    server_wrappers[active_id].mouse_events.push(me);
    mtx_mouse.unlock();
}

void pushKeyboardEvent(KeyboardEvent ke) {
    std::unique_lock<std::mutex> lock(mtx_keyboard);
    server_wrappers[active_id].keyboard_events.push(ke);
    mtx_keyboard.unlock();
}

void handleEvents() {
    // SDL poll event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        else if (connected && active_id < server_wrappers.size() && server_wrappers[active_id].frame_wrapper.is_hovered)
        {
            if (event.type == SDL_MOUSEMOTION) pushMouseEvent(MouseEvent(MouseMove, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) pushMouseEvent(MouseEvent(LDown, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) pushMouseEvent(MouseEvent(RDown, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) pushMouseEvent(MouseEvent(LUp, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) pushMouseEvent(MouseEvent(RUp, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
            }
            else if (event.type == SDL_KEYDOWN) pushKeyboardEvent(KeyboardEvent(KeyDown, event.key.keysym.sym));
            else if (event.type == SDL_KEYUP) pushKeyboardEvent(KeyboardEvent(KeyUp, event.key.keysym.sym));
        }
    }
}

void handleConnectButton() {
    // Send SECRET
    std::thread thread_passcode([&]()
    {
        broadcastMessage(PORT_P, SECRET, 7, inet_addr(host));

        // ------------------

        server_passcode.Listen(PORT_C, "UDP");

        server_passcode.setCallback(
            [&](SOCKET sock, char data[], int size, char host[]) {
                server_wrappers.push_back(ServerWrapper());
                server_wrappers.back().client_host = std::string(host, host + INET_ADDRSTRLEN);
                printf("host = %s\n", server_wrappers.back().client_host.c_str());
                printf("data = %s\n", data);
                fflush(stdout);
            }
        );

        while (true) {
            server_passcode.receiveData(7);
            if (server_wrappers.size()) break;
        }

        server_passcode.Close();

        // ------------------
    

        for (int i = 0; i < (int) server_wrappers.size(); i++)
        {
            server_wrappers[i].PORT_S = std::to_string(atoi(PORT_P) + (i + 1) * 3);
            server_wrappers[i].PORT_M = std::to_string(atoi(PORT_P) + (i + 1) * 3 + 1);
            server_wrappers[i].PORT_K = std::to_string(atoi(PORT_P) + (i + 1) * 3 + 2);

            client_passcode.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)PORT_C, "TCP");

            client_passcode.sendData((char*)server_wrappers[i].PORT_S.data(), 5);
            client_passcode.sendData((char*)server_wrappers[i].PORT_M.data(), 5);
            client_passcode.sendData((char*)server_wrappers[i].PORT_K.data(), 5);

            printf("[client_host] = %s\n", host);
            printf("PORT_S = %s\n", server_wrappers[i].PORT_S.c_str());
            printf("PORT_M = %s\n", server_wrappers[i].PORT_M.c_str());
            printf("PORT_K = %s\n", server_wrappers[i].PORT_K.c_str());

            printf("[%s] [%s]\n",(char*)server_wrappers[i].client_host.c_str(), (char*)server_wrappers[i].PORT_M.c_str());
            fflush(stdout);

            while (!server_wrappers[i].client_mouse.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)server_wrappers[i].PORT_M.c_str(), "TCP"));
            printf("Mouse connected!\n");

            while (!server_wrappers[i].client_keyboard.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)server_wrappers[i].PORT_K.c_str(), "TCP"));
            printf("Keyboard connected\n");

            server_wrappers[i].server_screen.Listen((char*)server_wrappers[i].PORT_S.c_str(), "UDP");
            printf("Screen connected\n");
        }

        std::thread thread_mouse([&](){
            while (!quit) {
                if (active_id < server_wrappers.size()) {
                    std::unique_lock<std::mutex> lock(mtx_mouse);
                    if (!server_wrappers[active_id].mouse_events.size()) {
                        mtx_mouse.unlock();
                        continue;
                    }

                    MouseEvent me = server_wrappers[active_id].mouse_events.front(); server_wrappers[active_id].mouse_events.pop();
                    mtx_mouse.unlock();
        
                    me.x /= server_wrappers[active_id].frame_wrapper.scaled_width;
                    me.y /= server_wrappers[active_id].frame_wrapper.scaled_height;

                    if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;

                    // printf("Mouse event %lf %lf\n", me.x, me.y);

                    std::vector<uchar> image_data((char*)&me, (char*)(&me) + sizeof(me));

                    static int id = 0;

                    PacketBox box;
                    BufToPacketBox(image_data, box, ++id, 'M', image_data.size() + 7);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        server_wrappers[active_id].client_mouse.sendData((char*)box.packets[i].data(), box.packets[i].size());
                    }
                }
            }
        });

        thread_mouse.detach();

        std::thread thread_keyboard([&](){
            while (!quit) {
                if (active_id < server_wrappers.size()) {
                    std::unique_lock<std::mutex> lock(mtx_keyboard);
                    if (!server_wrappers[active_id].keyboard_events.size()) {
                        mtx_keyboard.unlock();
                        continue;
                    }

                    KeyboardEvent ke = server_wrappers[active_id].keyboard_events.front(); server_wrappers[active_id].keyboard_events.pop();
                    mtx_keyboard.unlock();

                    // if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                    // else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                    std::vector<uchar> image_data((char*)&ke, (char*)(&ke) + sizeof(ke));

                    static int id = 0;

                    PacketBox box;
                    BufToPacketBox(image_data, box, ++id, 'K', image_data.size() + 7);

                    for (int i = 0; i < (int) box.packets.size(); i++) {
                        server_wrappers[active_id].client_keyboard.sendData((char*)box.packets[i].data(), box.packets[i].size());
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
                // printf("%d\n", box.packets.size());
                std::vector<uchar> image_data;
                PacketBoxToBuf(box, image_data);
                if (box.type == 'I') {
                    // if (server_wrappers[active_id].frame_wrapper.frame_queue.size() <= 2)
                    server_wrappers[active_id].frame_wrapper.frame_queue.push(image_data);
                }
            });

            server_wrappers[active_id].server_screen.setCallback(
                [&boxman_screen](SOCKET sock, char data[], int size, char host[]) {
                    // short id = *(data);
                    // short num = *(short*)(data + 3);
                    // printf("id = %d, num = %d\n", id, num);
                    std::vector<uchar> image_data(data, data + size);
                    boxman_screen.addPacketToBox(image_data);
                }
            );

            while (!quit) {
                if (active_id < server_wrappers.size()) {
                    server_wrappers[active_id].server_screen.receiveData(1440);
                }
            }
        });

        thread_screen.detach();

        // Connected
        connected = true;
        active_id = 0;
    });

    thread_passcode.detach();
}

void menuBar() {
    bool about_popup = false;
    bool connect_popup = false;

    // Menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Connect")) {
            connect_popup = true;
        }
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
        // ImGui::Separator();
        // ImGui::Text("Made by: allforest01");
        ImGui::EndPopup();
    }

    // Connect popup
    if (connect_popup) {
        ImGui::OpenPopup("Connect");
    }
    if (ImGui::BeginPopup("Connect", ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::PushItemWidth(200);
        ImGui::InputText("##host", (char*)host, 16);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Connect")) handleConnectButton();    
        ImGui::EndPopup();
    }
}

void clientScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(1000, 630));

    ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (active_id < server_wrappers.size())
    {
        if (server_wrappers[active_id].frame_wrapper.frame_queue.size()) {
            server_wrappers[active_id].frame_wrapper.pushToTexture();
        }

        if (!server_wrappers[active_id].frame_wrapper.isTextureGenerated()) server_wrappers[active_id].frame_wrapper.generateTexture();
        ImGui::ImageButton((void*)(intptr_t)server_wrappers[active_id].frame_wrapper.image_texture, ImVec2(server_wrappers[active_id].frame_wrapper.scaled_width, server_wrappers[active_id].frame_wrapper.scaled_height));

        server_wrappers[active_id].frame_wrapper.is_hovered = ImGui::IsItemHovered();
        server_wrappers[active_id].frame_wrapper.is_focused = ImGui::IsItemFocused();

        server_wrappers[active_id].frame_wrapper.start_x = ImGui::GetItemRectMin().x;
        server_wrappers[active_id].frame_wrapper.start_y = ImGui::GetItemRectMin().y;

        if (!server_wrappers[active_id].frame_wrapper.is_hovered || !server_wrappers[active_id].frame_wrapper.is_focused) {
            std::unique_lock<std::mutex> lock_mouse(mtx_mouse);
            std::queue<MouseEvent>().swap(server_wrappers[active_id].mouse_events);
            lock_mouse.unlock();

            std::unique_lock<std::mutex> lock_keyboard(mtx_mouse);
            std::queue<KeyboardEvent>().swap(server_wrappers[active_id].keyboard_events);
            lock_keyboard.unlock();
        }
    }

    ImGui::End();
}

void startNewFrame() {
    // Start a new ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(imgui_wrapper.window);
    ImGui::NewFrame();
}

void guiRendering() {
    // guiRendering
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

void clientListWindow() {
    ImGui::SetNextWindowPos(ImVec2(1020, 30));
    ImGui::SetNextWindowSize(ImVec2(230, 630));
    ImGui::Begin("Client List", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    for (int i = 0; i < (int) server_wrappers.size(); i++) {
        ImGui::Text("%s", server_wrappers[i].client_host.c_str());
        int scaled_width = ImGui::GetContentRegionAvail().x - 6;
        int scaled_height = server_wrappers[active_id].frame_wrapper.scaled_height * scaled_width / server_wrappers[active_id].frame_wrapper.scaled_width;
        if (!server_wrappers[active_id].frame_wrapper.isTextureGenerated()) server_wrappers[active_id].frame_wrapper.generateTexture();
        ImGui::ImageButton((void*)(intptr_t)server_wrappers[active_id].frame_wrapper.image_texture, ImVec2(scaled_width, scaled_height));
    }

    ImGui::End();
}

int main(int argc, char** argv)
{
    initSocketManager();
    imgui_wrapper = ImGuiWrapper(1260, 670, (char*)"Server");
    initImGui(imgui_wrapper);

    while (!quit)
    {
        startNewFrame();

        menuBar();
        clientScreenWindow();
        clientListWindow();

        guiRendering();
        handleEvents();
    }

    cleanImGui(imgui_wrapper);
    cleanSocketManager();

    return 0;
}
