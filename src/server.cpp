#include "constant.h"
#include "server.h"

char host[16] = "10.211.55.255";

FrameWrapper frame_wrapper;
ImGuiWrapper imgui_wrapper;

ClientManager client_passcode;
ServerManager server_passcode;
ClientManager client_mouse;
ClientManager client_keyboard;
ServerManager server_screen;

std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_mouse, mtx_keyboard;
std::vector<std::string> client_hosts;

bool quit = false, connected = false;

void pushMouseEvent(MouseEvent me) {
    std::unique_lock<std::mutex> lock(mtx_mouse);
    mouse_events.push(me);
    mtx_mouse.unlock();
}

void pushKeyboardEvent(KeyboardEvent ke) {
    std::unique_lock<std::mutex> lock(mtx_keyboard);
    keyboard_events.push(ke);
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
        else if (connected && frame_wrapper.is_hovered)
        {
            if (event.type == SDL_MOUSEMOTION) pushMouseEvent(MouseEvent(MouseMove, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT) pushMouseEvent(MouseEvent(LDown, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) pushMouseEvent(MouseEvent(RDown, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT) pushMouseEvent(MouseEvent(LUp, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
                else if (event.button.button == SDL_BUTTON_RIGHT) pushMouseEvent(MouseEvent(RUp, event.button.x - frame_wrapper.start_x,  event.button.y - frame_wrapper.start_y));
            }
            else if (event.type == SDL_KEYDOWN) pushKeyboardEvent(KeyboardEvent(KeyDown, event.key.keysym.sym));
            else if (event.type == SDL_KEYUP) pushKeyboardEvent(KeyboardEvent(KeyUp, event.key.keysym.sym));
        }
    }
}

void connectButtonHandle() {
    // Send SECRET
    std::thread thread_passcode([&]()
    {
        broadcastMessage(PORT_P, SECRET, 7, inet_addr(host));

        server_passcode.Listen(PORT_C, "UDP");

        server_passcode.setService(
            [&](SOCKET sock, char data[], int size, char host[]) {
                client_hosts.push_back(std::string(host, host + INET_ADDRSTRLEN));
                printf("host = %s\n", client_hosts.back().c_str());
                printf("data = %s\n", data);
                fflush(stdout);
            }
        );

        while (true) {
            server_passcode.receiveData(1);
            if (client_hosts.size()) break;
        }

        server_passcode.Close();

        strcpy(host, client_hosts[0].c_str());
        printf("[host] = %s\n", host);

        // ---------------------------------------------------

        printf("Start thread_mouse\n");

        // client_mouse send mouse events
        while (!client_mouse.Connect(host, PORT_M, "TCP"));

        printf("Start thread_keyboard\n");

        // client_mouse send mouse events
        while (!client_keyboard.Connect(host, PORT_K, "TCP"));

        std::thread thread_mouse([&](){
            while (!quit) {
                std::unique_lock<std::mutex> lock(mtx_mouse);
                if (!mouse_events.size()) {
                    mtx_mouse.unlock();
                    continue;
                }

                MouseEvent me = mouse_events.front(); mouse_events.pop();
                mtx_mouse.unlock();
    
                me.x /= frame_wrapper.scaled_width;
                me.y /= frame_wrapper.scaled_height;

                if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;

                // printf("Mouse event %lf %lf\n", me.x, me.y);

                std::vector<uchar> image_data((char*)&me, (char*)(&me) + sizeof(me));

                static int id = 0;

                PacketBox box;
                BufToPacketBox(image_data, box, ++id, 'M', image_data.size() + 7);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_mouse.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
            }
        });

        thread_mouse.detach();

        std::thread thread_keyboard([&](){
            while (!quit) {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                if (!keyboard_events.size()) {
                    mtx_keyboard.unlock();
                    continue;
                }

                KeyboardEvent ke = keyboard_events.front(); keyboard_events.pop();
                mtx_keyboard.unlock();

                // if (ke.type == KeyDown) std::cout << "Key pressed: " << SDL_GetKeyName(ke.keyCode) << std::endl;
                // else if (ke.type == KeyUp) std::cout << "Key released: " << SDL_GetKeyName(ke.keyCode) << std::endl;

                std::vector<uchar> image_data((char*)&ke, (char*)(&ke) + sizeof(ke));

                static int id = 0;

                PacketBox box;
                BufToPacketBox(image_data, box, ++id, 'K', image_data.size() + 7);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_keyboard.sendData((char*)box.packets[i].data(), box.packets[i].size());
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
                    // if (frame_wrapper.frame_queue.size() <= 2)
                    frame_wrapper.frame_queue.push(image_data);
                }
            });

            server_screen.setService(
                [&boxman_screen](SOCKET sock, char data[], int size, char host[]) {
                    // short id = *(data);
                    // short num = *(short*)(data + 3);
                    // printf("id = %d, num = %d\n", id, num);
                    std::vector<uchar> image_data(data, data + size);
                    boxman_screen.addPacketToBox(image_data);
                }
            );

            server_screen.Listen(PORT_S, "UDP");

            while (!quit) server_screen.receiveData(1440);
        });

        thread_screen.detach();

        // Connected
        connected = true;
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
        if (ImGui::Button("Connect")) connectButtonHandle();    
        ImGui::EndPopup();
    }
}

void clientScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(1000, 630));

    ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (frame_wrapper.frame_queue.size()) {
        frame_wrapper.pushToTexture();
        static int cnt = 0;
        printf("cnt = %d\n", ++cnt);
    }

    ImGui::ImageButton((void*)(intptr_t)frame_wrapper.image_texture, ImVec2(frame_wrapper.scaled_width, frame_wrapper.scaled_height));

    frame_wrapper.is_hovered = ImGui::IsItemHovered();
    frame_wrapper.is_focused = ImGui::IsItemFocused();

    frame_wrapper.start_x = ImGui::GetItemRectMin().x;
    frame_wrapper.start_y = ImGui::GetItemRectMin().y;

    if (!frame_wrapper.is_hovered || !frame_wrapper.is_focused) {
        std::unique_lock<std::mutex> lock_mouse(mtx_mouse);
        std::queue<MouseEvent>().swap(mouse_events);
        lock_mouse.unlock();

        std::unique_lock<std::mutex> lock_keyboard(mtx_mouse);
        std::queue<KeyboardEvent>().swap(keyboard_events);
        lock_keyboard.unlock();
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

void clientList() {
    ImGui::SetNextWindowPos(ImVec2(1020, 30));
    ImGui::SetNextWindowSize(ImVec2(230, 630));
    ImGui::Begin("Client List", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Client 1");
    int scaled_width = ImGui::GetContentRegionAvail().x - 6;
    int scaled_height = frame_wrapper.scaled_height * scaled_width / frame_wrapper.scaled_width;
    ImGui::ImageButton((void*)(intptr_t)frame_wrapper.image_texture, ImVec2(scaled_width, scaled_height));
    ImGui::End();
}

int main(int argc, char** argv)
{
    initSocketManager();
    imgui_wrapper = ImGuiWrapper(1260, 670, (char*)"Server");
    initImGui(imgui_wrapper);
    frame_wrapper.initTexture();

    while (!quit)
    {
        startNewFrame();

        menuBar();
        clientScreenWindow();
        clientList();

        guiRendering();
        handleEvents();
    }

    cleanImGui(imgui_wrapper);
    frame_wrapper.cleanTexture();
    cleanSocketManager();

    return 0;
}
