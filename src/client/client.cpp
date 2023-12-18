#include "client.h"

#define SECRET "HYPNO"
#define PORT_A "40020"
#define PORT_B "40021"
#define PORT_C "40022"

#define SCREEN_STREAM_TYPE "UDP"
#define NUM_OF_THREADS 1
#define PACKET_SIZE 1400

char host[INET_ADDRSTRLEN] = "10.211.55.255";
char debug_message[256] = "Debug message...";

std::mutex mtx_keyboard, mtx_screen;

ServerSocketManager server_passcode;
ServerSocketManager server_keep_alive;

ImGuiWrapper imgui_wrapper;
ClientWrapper client_wrapper;

bool quit = false, connected = false;
bool alive = true;
bool notification_popup = false;

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

void connectButtonHandle() {
    std::thread thread_passcode([&]()
    {
        broadcastMessage(PORT_A, SECRET, 6, inet_addr(host));

        auto start = std::chrono::high_resolution_clock::now();

        while (!server_passcode.Listen((char*)PORT_C, "TCP")) {

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            printf("duration = %d\n", duration); fflush(stdout);

            if (duration.count() >= 3000) {
                notification_popup = true;
                return;
            }
        }

        printf("PORT_C connect successful!");

        inet_ntop(AF_INET, &(server_passcode.client_address.sin_addr), host, INET_ADDRSTRLEN);

        server_passcode.setReceiveCallback(
            [&](SOCKET sock, char data[], int size, char host[]) {
                client_wrapper.PORT_M = std::string(data +  0, data +  5);
                client_wrapper.PORT_K = std::string(data +  5, data + 10);
                client_wrapper.PORT_S = std::string(data + 10, data + 15);
            }
        );

        printf("----------- start receive ports info -----------------\n"); fflush(stdout);
        while (server_passcode.receiveData(15) == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        printf("------------- end receive ports info ----------------\n"); fflush(stdout);

        server_passcode.Close();

        printf("[server_host] = %s\n", host);
        printf("PORT_M = %s\n", client_wrapper.PORT_M.c_str());
        printf("PORT_K = %s\n", client_wrapper.PORT_K.c_str());
        printf("PORT_S = %s\n", client_wrapper.PORT_S.c_str());

        // ---------------------------------------------------

        while (!client_wrapper.server_mouse.Listen((char*)client_wrapper.PORT_M.c_str(), "TCP"));
        printf("[Mouse connected]\n");

        while (!client_wrapper.server_keyboard.Listen((char*)client_wrapper.PORT_K.c_str(), "TCP"));
        printf("[Keyboard connected]\n");

        client_wrapper.client_screen.connect(host, atoi((char*)client_wrapper.PORT_S.c_str()), SCREEN_STREAM_TYPE, NUM_OF_THREADS);
        printf("[Screen connected]\n");

        std::thread thread_keep_alive([&](){
            printf("server_keep_alive start listen!\n"); fflush(stdout);
            while (!server_keep_alive.Listen((char*)PORT_B, "UDP"));
            printf("server_keep_alive listen successful!\n"); fflush(stdout);

            server_keep_alive.setReceiveCallback(
                [&](SOCKET sock, char data[], int size, char host[]) {
                    if (data[0] == 'q') {
                        quit = true;
                        alive = false;
                        server_keep_alive.Close();
                        printf("QUIT\n"); fflush(stdout);
                    }
                    else if (data[0] == 'a') {
                        alive = true;
                        printf("ALIVE\n"); fflush(stdout);
                    }
                    else if (data[0] == 'd') {
                        alive = false;
                        printf("DEAD\n"); fflush(stdout);
                    }
                }
            );

            while (!quit)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                server_keep_alive.receiveData(1);
            }
        });

        thread_keep_alive.detach();

        std::thread thread_mouse([&]()
        {
            client_wrapper.server_mouse.setReceiveCallback(
                [&](SOCKET sock, char data[], int size, char host[]) {
                    if (alive) {
                        MouseEvent &me = *(MouseEvent*)data;

                        if (me.type == MouseWheel)
                        {
                            int x = round(me.x * 40);
                            int y = round(me.y * 40);

                            printf("Send Wheel %f %f\n", me.x, me.y); fflush(stdout);

                            EventsManager::getInstance().emitWheel(x, y);
                        }
                        else
                        {
                            int x = round(me.x * EventsManager::getInstance().width);
                            int y = round(me.y * EventsManager::getInstance().height);

                            snprintf(debug_message, sizeof(debug_message), "Send Mouse %d %d\n", x, y);

                            if (me.type == MouseMove) EventsManager::getInstance().emitMove(x, y);
                            else if (me.type == LDown) EventsManager::getInstance().emitLDown(x, y);
                            else if (me.type == LUp) EventsManager::getInstance().emitLUp(x, y);
                            else if (me.type == RDown) EventsManager::getInstance().emitRDown(x, y);
                            else if (me.type == RUp) EventsManager::getInstance().emitRUp(x, y);
                        }
                    }
                    else std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            );

            while (!quit) {
                if (alive) {
                    client_wrapper.server_mouse.receiveData(sizeof(MouseEvent));
                    printf("recv mouse!\n");
                }
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        thread_mouse.detach();

        std::thread thread_keyboard_events([&]() {
            while (!quit)
            {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                if (!client_wrapper.keyboard_events.size()) {
                    mtx_keyboard.unlock();
                    continue;
                }

                KeyboardEvent ke = client_wrapper.keyboard_events.front(); client_wrapper.keyboard_events.pop();
                mtx_keyboard.unlock();

                if (alive)
                {
                    if (ke.type == KeyDown) EventsManager::getInstance().emitKeyDown(SDLKeycodeToOSKeyCode(ke.keyCode));
                    else if (ke.type == KeyUp) EventsManager::getInstance().emitKeyUp(SDLKeycodeToOSKeyCode(ke.keyCode));
                }
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        thread_keyboard_events.detach();

        std::thread thread_keyboard_socket([&]()
        {
            client_wrapper.server_keyboard.setReceiveCallback(
                [&](SOCKET sock, char data[], int size, char host[]) {
                    if (alive) {
                        std::unique_lock<std::mutex> lock(mtx_keyboard);
                        client_wrapper.keyboard_events.push(*(KeyboardEvent*)data);
                        mtx_keyboard.unlock();
                    }
                    else std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            );

            while (!quit) {
                if (alive) {
                    client_wrapper.server_keyboard.receiveData(sizeof(KeyboardEvent));
                    printf("recv keyboard!\n");
                }
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        thread_keyboard_socket.detach();

        std::thread thread_screen_events([&]() {

            while (!quit)
            {   
                if (alive)
                {
                    cv::Mat mat = EventsManager::getInstance().captureScreen();
                    // resize(mat, mat, cv::Size(), 1, 1);

                    std::vector<uchar> frame;
                    compressImage(mat, frame, 80);

                    static int id = 0;

                    PacketBox box;
                    BufToPacketBox(frame, box, ++id, 'I', PACKET_SIZE);

                    std::unique_lock<std::mutex> lock(mtx_screen);
                    std::queue<PacketBox>().swap(client_wrapper.frame_box_queue);
                    client_wrapper.frame_box_queue.push(box);
                    mtx_screen.unlock();
                }
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

        });

        thread_screen_events.detach();

        std::thread thread_screen_socket([&]() {

            while (!quit)
            {
                std::unique_lock<std::mutex> lock(mtx_screen);
                if (!client_wrapper.frame_box_queue.size()) continue;

                PacketBox box = client_wrapper.frame_box_queue.front(); client_wrapper.frame_box_queue.pop();
                mtx_screen.unlock();

                if (alive)
                {
                    client_wrapper.client_screen.send(box);
                    printf("Sent!\n"); fflush(stdout);
                }
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

        });

        thread_screen_socket.detach();

        connected = true;

    });
    
    thread_passcode.detach();
}

void startWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(210, 80));

    ImGui::Begin("Listen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    ImGui::PushItemWidth(-1);
    ImGui::InputText("Broadcast", host, sizeof(host));

    ImGui::SetCursorPos(ImVec2(8, 34));

    if (ImGui::Button("Connect", ImVec2(130, 20))) connectButtonHandle();
    ImGui::SameLine();

    if (ImGui::Button("Exit", ImVec2(ImGui::GetContentRegionAvail().x - 1, 20))) { quit = true; }

    if (notification_popup) {
        ImGui::Text("Can't connect to the server!");
    }

    ImGui::End();
}

void connectedWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(210, 80));
    ImGui::Begin("Listen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    // When client is connected from server
    ImGui::Text("%s", debug_message);
    ImGui::End();
}

int main(int argc, char** argv)
{
    initSocketManager();
    imgui_wrapper = ImGuiWrapper(210, 80, (char*)"Client");
    initImGui(imgui_wrapper);

    while (!quit)
    {
        startNewFrame();

        if (!connected) startWindow();
        else connectedWindow();

        guiRendering();
        handleEvents();
    }

    cleanImGui(imgui_wrapper);
    cleanSocketManager();

    return 0;
}
