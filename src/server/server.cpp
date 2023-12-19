#include "server.h"

#define SECRET "HYPNO"

#define PORT_A "40020"
#define PORT_B "40021"
#define PORT_C "40022"
// #define PORT_D "40023"

#define SCREEN_STREAM_TYPE "UDP"
#define NUM_OF_THREADS 1

char host[INET_ADDRSTRLEN];

std::mutex mtx_mouse;
std::mutex mtx_keyboard;
std::mutex mtx_frame;

ServerSocketManager server_conn_handler;
ClientSocketManager client_conn_handler;
ClientSocketManager client_heartbeat;
// ServerSocketManager server_heartbeat;

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
        else if (connected && active_id < (int) server_wrappers.size() && server_wrappers[active_id].frame_wrapper.is_hovered)
        {
            if (event.type == SDL_MOUSEMOTION) pushMouseEvent(MouseEvent(MouseMove, event.button.x - server_wrappers[active_id].frame_wrapper.start_x,  event.button.y - server_wrappers[active_id].frame_wrapper.start_y));
            else if (event.type == SDL_MOUSEWHEEL) pushMouseEvent(MouseEvent(MouseWheel, event.wheel.preciseX, event.wheel.preciseY));
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

void newConnectionHandle(char data[], char host[]) {
    if (strcmp(data, SECRET)) return;

    server_wrappers.push_back(ServerWrapper());
    server_wrappers.back().client_host = std::string(host, host + INET_ADDRSTRLEN);

    // printf("host = %s\n", server_wrappers.back().client_host.c_str());
    // printf("data = %s\n", data);
    // fflush(stdout);

    int i = (int) server_wrappers.size() - 1;

    server_wrappers[i].PORT_M = std::to_string(atoi(PORT_A) + (i + 1) * 10 + 0);
    server_wrappers[i].PORT_K = std::to_string(atoi(PORT_A) + (i + 1) * 10 + 1);
    server_wrappers[i].PORT_S = std::to_string(atoi(PORT_A) + (i + 1) * 10 + 2);

    // printf("[%s] [%s]\n", (char*)server_wrappers[i].client_host.c_str(), (char*)PORT_C);
    
    while (!client_conn_handler.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)PORT_C, "TCP"));

    std::string ports = server_wrappers[i].PORT_M + server_wrappers[i].PORT_K + server_wrappers[i].PORT_S;

    // printf("----------- start send ports info -----------------\n"); // fflush(stdout);
    while (client_conn_handler.sendData((char*)ports.data(), 15) == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // printf("------------- end send ports info -----------------\n"); // fflush(stdout);

    client_conn_handler.Close();

    // printf("[client_host] = %s\n", (char*)server_wrappers[i].client_host.c_str());

    // printf("PORT_M = %s\n", server_wrappers[i].PORT_M.c_str());
    // printf("PORT_K = %s\n", server_wrappers[i].PORT_K.c_str());
    // printf("PORT_S = %s\n", server_wrappers[i].PORT_S.c_str());

    // fflush(stdout);

    while (!server_wrappers[i].client_mouse.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)server_wrappers[i].PORT_M.c_str(), "TCP"));
    // printf("[Mouse connected for %d]\n", i); // fflush(stdout);

    while (!server_wrappers[i].client_keyboard.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)server_wrappers[i].PORT_K.c_str(), "TCP"));
    // printf("[Keyboard connected for %d]\n", i); // fflush(stdout);

    server_wrappers[i].server_screen.listen((char*)server_wrappers[i].client_host.c_str(), atoi((char*)server_wrappers[i].PORT_S.c_str()), SCREEN_STREAM_TYPE, NUM_OF_THREADS);
    // printf("[Screen connected for %d]\n", i); // fflush(stdout);

    // Receive screen capture
    std::thread thread_screen([&, i]()
    {
        server_wrappers[i].server_screen.setCompleteCallback([i](PacketBox& box) {
            std::vector<uchar> image_data;
            box.sort();
            PacketBoxToBuf(box, image_data);
            std::unique_lock<std::mutex> lock_frame(mtx_frame);
            std::queue<std::vector<uchar>>().swap(server_wrappers[i].frame_wrapper.frame_queue);
            server_wrappers[i].frame_wrapper.frame_queue.push(image_data);
            // // printf("[PUSHED] "); // fflush(stdout);
            lock_frame.unlock();
        });

        while (!quit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (active_id == i) server_wrappers[i].server_screen.receive();
        }
    });

    thread_screen.detach();

    // Set connected and default active id
    connected = true;
    active_id = i;
}

void startListen() {
    std::thread thread_conn_handler([&]()
    {
        server_conn_handler.Listen((char*)PORT_A, "UDP");

        server_conn_handler.setReceiveCallback(
            [](SOCKET sock, char data[], int size, char host[]) {
                std::thread threadNewConnection(newConnectionHandle, data, host);
                threadNewConnection.join();
            }
        );

        while (!quit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            server_conn_handler.receiveData(6);
        }

        server_conn_handler.Close();
    });

    thread_conn_handler.detach();

    std::thread thread_heartbeat([&]()
    {
        // while (!server_heartbeat.Listen((char*)PORT_D, "UDP"));

        int cur_active_id = INT_MAX;

        while (!quit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            const int const_active_id = active_id;

            if (const_active_id == INT_MAX || const_active_id >= (int)server_wrappers.size()) continue;

            if (cur_active_id != const_active_id) {
                // printf("[Changed]\n"); // fflush(stdout);
                if (cur_active_id != INT_MAX) {
                    while (client_heartbeat.isSocketAlive() && client_heartbeat.sendData((char*)"d", 1) == -1);
                    client_heartbeat.Close();
                }
                // printf("client_heartbeat start connect!\n"); // fflush(stdout);
                while (active_id != INT_MAX && const_active_id == active_id && !client_heartbeat.Connect((char*)server_wrappers[const_active_id].client_host.c_str(), (char*)PORT_B, "UDP"));
                // printf("client_heartbeat connect successful!\n"); // fflush(stdout);
                cur_active_id = const_active_id;
            }

            while (client_heartbeat.isSocketAlive() && client_heartbeat.sendData((char*)"a", 1) == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // bool disconnect = false;
            // auto start = std::chrono::high_resolution_clock::now();

            // while (server_heartbeat.receiveData(1) <= 0) {
            //     auto end = std::chrono::high_resolution_clock::now();
            //     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            //     // printf("duration = %d\n", duration); // fflush(stdout);

            //     if (duration.count() >= 1000) {
            //         disconnect = true;
            //         return;
            //     }
            // }

            // if (disconnect) {
            //     cur_active_id = active_id = INT_MAX;
            //     client_heartbeat.Close();
            //     continue;
            // }
        }

        if (cur_active_id != INT_MAX) {
            while (client_heartbeat.isSocketAlive() && client_heartbeat.sendData((char*)"q", 1) == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            client_heartbeat.Close();
        }

        for (int i = 0; i < (int) server_wrappers.size(); i++) {
            if (i != cur_active_id && client_heartbeat.Connect((char*)server_wrappers[i].client_host.c_str(), (char*)PORT_B, "UDP")) {
                while (client_heartbeat.isSocketAlive() && client_heartbeat.sendData((char*)"q", 1) == -1) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                client_heartbeat.Close();
            }
        }
    });

    thread_heartbeat.detach();

    std::thread thread_mouse([&](){
        while (!quit)
        {
            if (connected && active_id < (int) server_wrappers.size()) {
                std::unique_lock<std::mutex> lock(mtx_mouse);
                if (!server_wrappers[active_id].mouse_events.size()) {
                    lock.unlock();
                    continue;
                }

                MouseEvent me = server_wrappers[active_id].mouse_events.front(); server_wrappers[active_id].mouse_events.pop();
                mtx_mouse.unlock();

                if (me.type != MouseWheel)
                {
                    me.x /= server_wrappers[active_id].frame_wrapper.scaled_width;
                    me.y /= server_wrappers[active_id].frame_wrapper.scaled_height;

                    if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;
                }

                // // printf("Mouse event %lf %lf\n", me.x, me.y);

                // static int id = 0;

                if (active_id != INT_MAX && server_wrappers[active_id].client_mouse.isSocketAlive()) {
                    server_wrappers[active_id].client_mouse.sendData((char*)&me, sizeof(me));
                }
                // printf("sent mouse!\n"); // fflush(stdout);
            }
            else std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    thread_mouse.detach();

    std::thread thread_keyboard([&](){
        while (!quit)
        {
            if (connected && active_id < (int) server_wrappers.size()) {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                if (!server_wrappers[active_id].keyboard_events.size()) {
                    lock.unlock();
                    continue;
                }

                KeyboardEvent ke = server_wrappers[active_id].keyboard_events.front(); server_wrappers[active_id].keyboard_events.pop();
                mtx_keyboard.unlock();

                // if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                // else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                // static int id = 0;

                if (active_id != INT_MAX && server_wrappers[active_id].client_keyboard.isSocketAlive()) {
                    server_wrappers[active_id].client_keyboard.sendData((char*)&ke, sizeof(ke));
                }
                // printf("sent keyboard!\n"); // fflush(stdout);
            }
            else std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    thread_keyboard.detach();
}

void menuBar() {
    bool about_popup = false;
    // bool connect_popup = false;

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
        // ImGui::Separator();
        // ImGui::Text("Made by: allforest01");
        ImGui::EndPopup();
    }
}

void clientScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(1000, 630));

    ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (connected && active_id < (int) server_wrappers.size())
    {
        if (server_wrappers[active_id].frame_wrapper.isTexturePushed() && !server_wrappers[active_id].frame_wrapper.scale_calculated) {
            server_wrappers[active_id].frame_wrapper.scale_calculated = true;
            
            ImVec2 window_avail_size = ImGui::GetContentRegionAvail();
            window_avail_size.y -= 6;

            float window_aspect = window_avail_size.x / window_avail_size.y;
            float image_aspect = (float) server_wrappers[active_id].frame_wrapper.width / server_wrappers[active_id].frame_wrapper.height;

            float scale;
            if (window_aspect > image_aspect) {
                // imgui_wrapper.window is wider than the image
                scale = window_avail_size.y / server_wrappers[active_id].frame_wrapper.height;
            } else {
                // imgui_wrapper.window is narrower than the image
                scale = window_avail_size.x / server_wrappers[active_id].frame_wrapper.width;
            }

            server_wrappers[active_id].frame_wrapper.scaled_width = server_wrappers[active_id].frame_wrapper.width * scale;
            server_wrappers[active_id].frame_wrapper.scaled_height = server_wrappers[active_id].frame_wrapper.height * scale;
        }

        ImGui::ImageButton((void*)(intptr_t)server_wrappers[active_id].frame_wrapper.image_texture, ImVec2(server_wrappers[active_id].frame_wrapper.scaled_width, server_wrappers[active_id].frame_wrapper.scaled_height));

        server_wrappers[active_id].frame_wrapper.is_hovered = ImGui::IsItemHovered();
        server_wrappers[active_id].frame_wrapper.is_focused = ImGui::IsItemFocused();

        server_wrappers[active_id].frame_wrapper.start_x = ImGui::GetItemRectMin().x;
        server_wrappers[active_id].frame_wrapper.start_y = ImGui::GetItemRectMin().y;

        if (!server_wrappers[active_id].frame_wrapper.is_hovered || !server_wrappers[active_id].frame_wrapper.is_focused) {
            std::unique_lock<std::mutex> lock_mouse(mtx_mouse);
            // std::queue<MouseEvent>().swap(server_wrappers[active_id].mouse_events);
            while (server_wrappers[active_id].mouse_events.size()) {
                server_wrappers[active_id].mouse_events.pop();
            }
            lock_mouse.unlock();

            std::unique_lock<std::mutex> lock_keyboard(mtx_mouse);
            // std::queue<KeyboardEvent>().swap(server_wrappers[active_id].keyboard_events);
            while (server_wrappers[active_id].keyboard_events.size()) {
                server_wrappers[active_id].keyboard_events.pop();
            }
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
    SDL_Delay(10);
}

void clientListWindow() {
    ImGui::SetNextWindowPos(ImVec2(1020, 30));
    ImGui::SetNextWindowSize(ImVec2(230, 630));
    ImGui::Begin("Client List", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (connected) {
        for (int i = 0; i < (int) server_wrappers.size(); i++) {
            ImGui::Text("%s", server_wrappers[i].client_host.c_str());

            if ((active_id == i || !server_wrappers[i].frame_wrapper.isTexturePushed()) && server_wrappers[i].frame_wrapper.frame_queue.size()) {
                std::unique_lock<std::mutex> lock_frame(mtx_frame);
                server_wrappers[i].frame_wrapper.pushToTexture();
                // // printf("[POPED] "); // fflush(stdout);
                lock_frame.unlock();
            }

            int scaled_width = 0;
            int scaled_height = 0;

            if (server_wrappers[i].frame_wrapper.isTexturePushed()) {
                scaled_width = ImGui::GetContentRegionAvail().x - 6;
                scaled_height = server_wrappers[i].frame_wrapper.height * scaled_width / server_wrappers[i].frame_wrapper.width;
            }

            if (ImGui::ImageButton((void*)(intptr_t)server_wrappers[i].frame_wrapper.image_texture, ImVec2(scaled_width, scaled_height))) {
                active_id = i;
            }
        }
    }

    ImGui::End();
}

int main(int argc, char** argv)
{
    initSocketManager();
    imgui_wrapper = ImGuiWrapper(1260, 670, (char*)"Server");
    initImGui(imgui_wrapper);

    server_wrappers.reserve(16);
    startListen();

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
