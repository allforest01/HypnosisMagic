#include "../include/port.h"
#include "../include/client.h"

char host[16];
char passcode[7] = "ABCXYZ";

HypnoServer server_passcode;
HypnoClient client_passcode;
HypnoServer server_mouse;
HypnoServer server_keyboard;
HypnoClient client_screen;

std::queue<KeyboardEvent> keyboard_events;
std::queue<PacketBox> framebox_queue;
std::mutex mtx_keyboard, mtx_screen;
HypnoEvent easy_event;

bool quit = false, waiting = false, connected = false;

ImGuiWrapper imgui_wrapper;
char debug[256] = "Debug message";

void HandleEvents() {
    // SDL poll event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
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

void StartButtonHandle() {
    // waiting for a connection
    waiting = true;

    std::thread thread_passcode([&](){
        server_passcode.setService(
            [](SOCKET sock, char data[], int size, char ipv4[]) {
                if (!strcmp(data, passcode) || !strcmp(data, "ABCXYZ")) {
                    strcpy(host, ipv4);
                    printf("host = %s\n", host);
                    fflush(stdout);
                    waiting = false;
                    connected = 1;
                }
            }
        );

        server_passcode.hypnoListen(PORT_PASSCODE, "UDP");
        
        while (!quit && waiting) {
            server_passcode.UDPReceive(7);
        }

        server_passcode.hypnoClose();
        
        while (!client_passcode.hypnoConnect(host, PORT_PASSCODE, "TCP"));

        client_passcode.hypnoClose();

        server_mouse.hypnoListen(PORT_MOUSE, "TCP");

        server_keyboard.hypnoListen(PORT_KEYBOARD, "TCP");

        std::thread thread_mouse([&]()
        {
            BoxManager boxman_mouse;

            boxman_mouse.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'M')
                {
                    MouseEvent &me = *(MouseEvent*)buf.data();
                    
                    int x = round(me.x * easy_event.width);
                    int y = round(me.y * easy_event.height);

                    sprintf(debug, "Send Mouse %d %d\n", x, y);

                    if (me.type == LDown) easy_event.emitLDown(x, y);
                    else if (me.type == LUp) easy_event.emitLUp(x, y);
                    else if (me.type == RDown) easy_event.emitRDown(x, y);
                    else if (me.type == RUp) easy_event.emitRUp(x, y);
                    else if (me.type == MouseMove) easy_event.emitMove(x, y);
                }
            });

            server_mouse.setService(
                [&boxman_mouse](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman_mouse.addPacketToBox(buf);
                }
            );

            while (!quit) server_mouse.TCPReceive(sizeof(MouseEvent) + 7);
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

            while (!quit) server_keyboard.TCPReceive(sizeof(KeyboardEvent) + 7);
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
                
                if (ke.type == KeyDown) easy_event.emitKeyDown(SDLKeycodeToOSKeyCode(ke.keyCode));
                else if (ke.type == KeyUp) easy_event.emitKeyUp(SDLKeycodeToOSKeyCode(ke.keyCode));
            }
        });

        thread_keyboard_events.detach();

        std::thread thread_screen_socket([&]() {

            while (!client_screen.hypnoConnect(host, PORT_SCREEN, "UDP"));

            while (!quit)
            {
                cv::Mat mat = easy_event.captureScreen();
                // resize(mat, mat, cv::Size(), 1, 1);

                std::vector<uchar> frame;
                auto start1 = std::chrono::high_resolution_clock::now();
                compressImage(mat, frame, 70);
                auto end1 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration1 = end1 - start1;
                printf("compressImage = %lf\n", duration1.count());

                static int id = 0;

                PacketBox box;
                auto start = std::chrono::high_resolution_clock::now();
                BufToPacketBox(frame, box, ++id, 'I', 1440);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                printf("BufToPacketBox = %lf\n", duration.count());

                std::unique_lock<std::mutex> lock(mtx_screen);
                framebox_queue.push(box);
                mtx_screen.unlock();
                
                // break;
            }

        });

        thread_screen_socket.detach();

        std::thread thread_screen_events([&]() {

            while (!quit)
            {
                std::unique_lock<std::mutex> lock(mtx_screen);
                if (!framebox_queue.size()) {
                    mtx_screen.unlock();
                    continue;
                }

                PacketBox box = framebox_queue.front(); framebox_queue.pop();
                mtx_screen.unlock();

                auto start = std::chrono::high_resolution_clock::now();
                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_screen.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                printf("client_screen = %lf\n", duration.count());
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

        });

        thread_screen_events.detach();

    });
    
    thread_passcode.detach();
}

void ListeningWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");
    // Open UDP socket to waiting for connect from server
    ImGui::Text("Open a port to waiting for a connection");
    ImGui::PushItemWidth(200);
    ImGui::InputText("##PORT_PASSCODE", (char*)PORT_PASSCODE, 6);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Start")) StartButtonHandle();
    ImGui::End();
}

void waitingingWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");
    // waitinging for a connection from server
    ImGui::PushItemWidth(-1);
    ImGui::ProgressBar(ImGui::GetTime() * -0.2f, ImVec2(0, 0), "waitinging for a connection");
    ImGui::PopItemWidth();

    ImGui::SetCursorPos(ImVec2(9, 53));
    ImGui::Text("Code: %s", passcode);
    ImGui::SetCursorPos(ImVec2(211, 50));

    if (ImGui::Button("Cancel")) {
        waiting = false;
    }
    ImGui::End();
}

void ConnectedWindow() {
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
        StartNewFrame();

        if (!waiting && !connected) ListeningWindow();
        else if (waiting) waitingingWindow();
        else if (connected) ConnectedWindow();

        Rendering();
        HandleEvents();
    }

    cleanImGui(imgui_wrapper);

    cleanHypnoSocket();

    return 0;
}
