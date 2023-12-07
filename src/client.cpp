#include "../include/constant.h"
#include "../include/client.h"

char host[16] = "255.255.255.255";
char debug[256] = "Debug message";

ImGuiWrapper imgui_wrapper;

HypnoServer server_passcode;
HypnoClient client_passcode;
HypnoServer server_mouse;
HypnoServer server_keyboard;
HypnoClient client_screen;

std::queue<KeyboardEvent> keyboard_events;
std::queue<PacketBox> frame_box_queue;
std::mutex mtx_keyboard, mtx_screen;

bool quit = false, waiting = false, connected = false;

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
        server_passcode.setService(
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

        server_passcode.hypnoListen((char*)PORT_P, "UDP");
        
        while (!quit && waiting) {
            server_passcode.receiveData(7);
        }

        server_passcode.hypnoClose();
        
        while (!client_passcode.hypnoConnect(host, (char*)PORT_P, "TCP"));

        client_passcode.hypnoClose();

        server_mouse.hypnoListen((char*)PORT_M, "TCP");

        server_keyboard.hypnoListen((char*)PORT_K, "TCP");

        std::thread thread_mouse([&]()
        {
            BoxManager boxman_mouse;

            boxman_mouse.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'M')
                {
                    MouseEvent &me = *(MouseEvent*)buf.data();
                    
                    int x = round(me.x * HypnoEvent::getInstance().width);
                    int y = round(me.y * HypnoEvent::getInstance().height);

                    snprintf(debug, 256, "Send Mouse %d %d\n", x, y);

                    if (me.type == LDown) HypnoEvent::getInstance().emitLDown(x, y);
                    else if (me.type == LUp) HypnoEvent::getInstance().emitLUp(x, y);
                    else if (me.type == RDown) HypnoEvent::getInstance().emitRDown(x, y);
                    else if (me.type == RUp) HypnoEvent::getInstance().emitRUp(x, y);
                    else if (me.type == MouseMove) HypnoEvent::getInstance().emitMove(x, y);
                }
            });

            server_mouse.setService(
                [&boxman_mouse](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman_mouse.addPacketToBox(buf);
                }
            );

            while (!quit) server_mouse.receiveData(sizeof(MouseEvent) + 7);
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
                    keyboard_events.push(*(KeyboardEvent*)buf.data());
                    mtx_keyboard.unlock();
                }
            });

            server_keyboard.setService(
                [&boxman_keyboard](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman_keyboard.addPacketToBox(buf);
                }
            );

            while (!quit) server_keyboard.receiveData(sizeof(KeyboardEvent) + 7);
        });

        thread_keyboard_socket.detach();

        std::thread thread_keyboard_events([&]() {
            while (!quit) {
                std::unique_lock<std::mutex> lock(mtx_keyboard);
                if (!keyboard_events.size()) {
                    mtx_keyboard.unlock();
                    continue;
                }
                
                KeyboardEvent ke = keyboard_events.front(); keyboard_events.pop();
                mtx_keyboard.unlock();
                
                if (ke.type == KeyDown) HypnoEvent::getInstance().emitKeyDown(SDLKeycodeToOSKeyCode(ke.keyCode));
                else if (ke.type == KeyUp) HypnoEvent::getInstance().emitKeyUp(SDLKeycodeToOSKeyCode(ke.keyCode));
            }
        });

        thread_keyboard_events.detach();

        std::thread thread_screen_socket([&]() {

            while (!client_screen.hypnoConnect(host, (char*)PORT_S, "UDP"));

            while (!quit)
            {
                cv::Mat mat = HypnoEvent::getInstance().captureScreen();
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
                frame_box_queue.push(box);
                mtx_screen.unlock();

            }

        });

        thread_screen_socket.detach();

        std::thread thread_screen_events([&]() {

            while (!quit)
            {
                std::unique_lock<std::mutex> lock(mtx_screen);
                if (!frame_box_queue.size()) {
                    mtx_screen.unlock();
                    continue;
                }

                PacketBox box = frame_box_queue.front(); frame_box_queue.pop();
                mtx_screen.unlock();

                // auto start = std::chrono::high_resolution_clock::now();
                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_screen.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
                // auto end = std::chrono::high_resolution_clock::now();
                // std::chrono::duration<double> duration = end - start;
                // printf("client_screen = %lf\n", duration.count());
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));

                printf("packets.size() = %lu\n", box.packets.size());
            }

        });

        thread_screen_events.detach();

    });
    
    thread_passcode.detach();
}

void listeningWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");
    // Open UDP socket to waiting for connect from server
    ImGui::Text("Open a port to waiting for a connection");
    ImGui::PushItemWidth(200);
    ImGui::InputText("##PORT_P", (char*)PORT_P, 6);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Start")) startButtonHandle();
    ImGui::End();
}

void waitingWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");
    // waitinging for a connection from server
    ImGui::PushItemWidth(-1);
    ImGui::ProgressBar(ImGui::GetTime() * -0.2f, ImVec2(0, 0), "waitinging for a connection");
    ImGui::PopItemWidth();

    ImGui::SetCursorPos(ImVec2(9, 53));
    ImGui::Text("Code: %s", SECRET);
    ImGui::SetCursorPos(ImVec2(211, 50));

    if (ImGui::Button("Cancel")) {
        waiting = false;
    }
    ImGui::End();
}

void connectedWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");

    // When client is connected from server
    ImGui::Text("%s", debug);
    ImGui::End();
}

int main(int argc, char** argv)
{
    initHypnoSocket();
    imgui_wrapper = ImGuiWrapper(310, 120, (char*)"Client");
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
    cleanHypnoSocket();

    return 0;
}
