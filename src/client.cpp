#define SECRET "aBcXyZ"

#define PORT_P "33330"
#define PORT_C "33331"

#include "client.h"

char host[16], debug[256];

std::mutex mtx_keyboard, mtx_screen;

ClientSocketManager client_passcode;
ServerSocketManager server_passcode;

ImGuiWrapper imgui_wrapper;
ClientWrapper client_wrapper;

bool quit = false;
bool waiting = false;
bool connected = false;

void handleEvents() {
    // SDL poll event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
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

void startButtonHandle() {
    // waiting for a connection
    waiting = true;

    std::thread thread_passcode([&](){
        server_passcode.setCallback(
            [](SOCKET sock, char data[], int size, char ipv4[]) {
                if (!strcmp(data, SECRET) || !strcmp(data, "ABCXYZ")) {
                    strcpy(host, ipv4);
                    printf("host = %s\n", host);
                    fflush(stdout);
                    waiting = false;
                    connected = 1;
                }
            }
        );

        server_passcode.Listen((char*)PORT_P, "UDP");
        
        while (!quit && waiting) {
            server_passcode.receiveData(7);
        }

        server_passcode.Close();

        // ------------------
        
        while (!client_passcode.Connect(host, (char*)PORT_C, "UDP"));

        while (!client_passcode.sendData(SECRET, 7));

        client_passcode.Close();

        // ------------------

        server_passcode.Listen((char*)PORT_C, "TCP");

        server_passcode.setCallback(
            [&](SOCKET sock, char data[], int size, char host[]) {
                client_wrapper.PORT_S = std::string(data, data + 5);
            }
        );

        while (server_passcode.receiveData(5) <= 0);

        server_passcode.setCallback(
            [&](SOCKET sock, char data[], int size, char host[]) {
                client_wrapper.PORT_M = std::string(data, data + 5);
            }
        );

        while (server_passcode.receiveData(5) <= 0);

        server_passcode.setCallback(
            [&](SOCKET sock, char data[], int size, char host[]) {
                client_wrapper.PORT_K = std::string(data, data + 5);
            }
        );

        while (server_passcode.receiveData(5) <= 0);

        printf("[server_host] = %s\n", host);
        printf("PORT_S = %s\n", client_wrapper.PORT_S.c_str());
        printf("PORT_M = %s\n", client_wrapper.PORT_M.c_str());
        printf("PORT_K = %s\n", client_wrapper.PORT_K.c_str());

        // ---------------------------------------------------

        client_wrapper.server_mouse.Listen((char*)client_wrapper.PORT_M.c_str(), "TCP");
        printf("Mouse connected!\n");

        client_wrapper.server_keyboard.Listen((char*)client_wrapper.PORT_K.c_str(), "TCP");
        printf("Keyboard connected\n");

        while (!client_wrapper.client_screen.Connect(host, (char*)client_wrapper.PORT_S.c_str(), "UDP"));
        printf("Screen connected\n");

        std::thread thread_mouse([&]()
        {
            BoxManager boxman_mouse;

            boxman_mouse.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'M')
                {
                    MouseEvent &me = *(MouseEvent*)buf.data();
                    
                    int x = round(me.x * EventsManager::getInstance().width);
                    int y = round(me.y * EventsManager::getInstance().height);

                    snprintf(debug, 256, "Send Mouse %d %d\n", x, y);

                    if (me.type == LDown) EventsManager::getInstance().emitLDown(x, y);
                    else if (me.type == LUp) EventsManager::getInstance().emitLUp(x, y);
                    else if (me.type == RDown) EventsManager::getInstance().emitRDown(x, y);
                    else if (me.type == RUp) EventsManager::getInstance().emitRUp(x, y);
                    else if (me.type == MouseMove) EventsManager::getInstance().emitMove(x, y);
                }
            });

            client_wrapper.server_mouse.setCallback(
                [&boxman_mouse](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman_mouse.addPacketToBox(buf);
                }
            );

            while (!quit) client_wrapper.server_mouse.receiveData(sizeof(MouseEvent) + 7);
        });

        thread_mouse.detach();

        std::thread thread_keyboard_socket([&]()
        {
            BoxManager boxman_keyboard;

            boxman_keyboard.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'K')
                {
                    std::unique_lock<std::mutex> lock(mtx_keyboard);
                    client_wrapper.keyboard_events.push(*(KeyboardEvent*)buf.data());
                    mtx_keyboard.unlock();
                }
            });

            client_wrapper.server_keyboard.setCallback(
                [&boxman_keyboard](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman_keyboard.addPacketToBox(buf);
                }
            );

            while (!quit) client_wrapper.server_keyboard.receiveData(sizeof(KeyboardEvent) + 7);
        });

        thread_keyboard_socket.detach();

        std::thread thread_keyboard_events([&]() {
            while (!quit) {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                if (!client_wrapper.keyboard_events.size()) {
                    mtx_keyboard.unlock();
                    continue;
                }
                
                KeyboardEvent ke = client_wrapper.keyboard_events.front(); client_wrapper.keyboard_events.pop();
                mtx_keyboard.unlock();
                
                if (ke.type == KeyDown) EventsManager::getInstance().emitKeyDown(SDLKeycodeToOSKeyCode(ke.keyCode));
                else if (ke.type == KeyUp) EventsManager::getInstance().emitKeyUp(SDLKeycodeToOSKeyCode(ke.keyCode));
            }
        });

        thread_keyboard_events.detach();

        std::thread thread_screen_socket([&]() {

            while (!quit)
            {
                // printf("START PUSH\n"); fflush(stdout);

                cv::Mat mat = EventsManager::getInstance().captureScreen();
                // resize(mat, mat, cv::Size(), 1, 1);

                std::vector<uchar> frame;
                // auto start1 = std::chrono::high_resolution_clock::now();
                compressImage(mat, frame, 90);
                // auto end1 = std::chrono::high_resolution_clock::now();
                // std::chrono::duration<double> duration1 = end1 - start1;
                // printf("compressImage = %lf\n", duration1.count());

                static int id = 0;

                PacketBox box;
                // auto start = std::chrono::high_resolution_clock::now();
                BufToPacketBox(frame, box, ++id, 'I', 1440);
                // auto end = std::chrono::high_resolution_clock::now();
                // std::chrono::duration<double> duration = end - start;
                // printf("BufToPacketBox = %lf\n", duration.count());

                std::unique_lock<std::mutex> lock(mtx_screen);
                client_wrapper.frame_box_queue.push(box);
                mtx_screen.unlock();

                // printf("END PUSH\n"); fflush(stdout);

            }

        });

        thread_screen_socket.detach();

        std::thread thread_screen_events([&]() {

            while (!quit)
            {
                // printf("START SEND\n"); fflush(stdout);

                std::unique_lock<std::mutex> lock(mtx_screen);
                if (!client_wrapper.frame_box_queue.size()) {
                    mtx_screen.unlock();
                    continue;
                }

                PacketBox box = client_wrapper.frame_box_queue.front(); client_wrapper.frame_box_queue.pop();
                mtx_screen.unlock();

                // auto start = std::chrono::high_resolution_clock::now();
                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_wrapper.client_screen.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
                // auto end = std::chrono::high_resolution_clock::now();
                // std::chrono::duration<double> duration = end - start;
                // printf("client_wrapper.client_screen = %lf\n", duration.count());
                
                // std::this_thread::sleep_for(std::chrono::milliseconds(16));

                // printf("packets.size() = %lu\n", box.packets.size());

                // printf("END SEND\n"); fflush(stdout);
            }

        });

        thread_screen_events.detach();

    });
    
    thread_passcode.detach();
}

void listeningWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 80));

    ImGui::Begin("Listen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    // Open UDP socket to waiting for connect from server
    // ImGui::Text("Open a port to waiting for a connection");
    // ImGui::PushItemWidth(200);
    // ImGui::InputText("##PORT_P", (char*)PORT_P, 6);
    // ImGui::PopItemWidth();

    // ImGui::SameLine();
    ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 20);
    if (ImGui::Button("Start", buttonSize)) startButtonHandle();
    if (ImGui::Button("Exit", buttonSize)) { quit = true; }

    ImGui::End();
}

void waitingWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 80));
    ImGui::Begin("Listen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    // waiting for a connection from server

    ImGui::PushItemWidth(-1);
    ImGui::ProgressBar(ImGui::GetTime() * -0.2f, ImVec2(0, 0), "");
    ImGui::PopItemWidth();

    // ImGui::SetCursorPos(ImVec2(9, 53));
    // ImGui::Text("Code: %s", SECRET);
    // ImGui::SetCursorPos(ImVec2(211, 50));

    ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 20);
    if (ImGui::Button("Cancel", buttonSize)) { waiting = false; }

    ImGui::End();
}

void connectedWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 80));
    ImGui::Begin("Listen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    // When client is connected from server
    ImGui::Text("%s", debug);
    ImGui::End();
}

int main(int argc, char** argv)
{
    initSocketManager();
    imgui_wrapper = ImGuiWrapper(200, 60, (char*)"Client");
    initImGui(imgui_wrapper);

    while (!quit)
    {
        startNewFrame();

        if (!waiting && !connected) listeningWindow();
        else if (waiting) waitingWindow();
        else if (connected) connectedWindow();

        guiRendering();
        handleEvents();
    }

    cleanImGui(imgui_wrapper);
    cleanSocketManager();

    return 0;
}
