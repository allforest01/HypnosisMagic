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

char host[16];
char passcode[7] = "ABCXYZ";

EasyServer server_passcode;
EasyClient client_passcode;
EasyServer server_mouse;
EasyServer server_keyboard;
EasyClient client_screen;

BoxManager boxman_mouse, boxman_keyboard;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_keyboard;

bool quit = false, waiting = false, connected = false;

EasyEvent easy_event;
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

void StartButtonHandle() {
    // waiting for a connection
    waiting = true;

    // for (int i = 0; i < 4; i++) {
    //     passcode[i] = rand() % 10 + '0';
    // }

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

        server_passcode.elisten(port_passcode, "UDP");
        
        while (!quit && waiting) {
            server_passcode.UDPReceive(7);
        }

        server_passcode.eclose();
        
        while (!client_passcode.econnect(host, port_passcode, "TCP"));
        client_passcode.eclose();

        boxman_mouse.setCompleteCallback([](PacketBox& box) {
            std::vector<uchar> buf;
            PacketBoxToBuf(box, buf);
            if (box.type == 'M')
            {
                MouseEvent &me = *(MouseEvent*)buf.data();
                
                int x = round(me.x * easy_event.width);
                int y = round(me.y * easy_event.height);

                sprintf(debug, "Send Mouse %d %d\n", x, y);

                if (me.type == LDown) easy_event.sendLDown(x, y);
                else if (me.type == LUp) easy_event.sendLUp(x, y);
                else if (me.type == RDown) easy_event.sendRDown(x, y);
                else if (me.type == RUp) easy_event.sendRUp(x, y);
                else if (me.type == MouseMove) easy_event.sendMove(x, y);
            }
        });

        server_mouse.setService(
            [](SOCKET sock, char data[], int size, char host[]) {
                std::vector<uchar> buf(data, data + size);
                boxman_mouse.addPacketToBox(buf);
            }
        );

        server_mouse.elisten(port_mouse, "TCP");

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
            [](SOCKET sock, char data[], int size, char host[]) {
                std::vector<uchar> buf(data, data + size);
                boxman_keyboard.addPacketToBox(buf);
            }
        );

        server_keyboard.elisten(port_keyboard, "TCP");

        std::thread thread_mouse([&]()
        {
            while (!quit) server_mouse.TCPReceive(sizeof(MouseEvent) + 7);
        });

        thread_mouse.detach();

        std::thread thread_keyboard_socket([&]()
        {
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
                
                if (ke.type == KeyDown) easy_event.sendKeyDown(SDLKeycodeToOSKeyCode(ke.keyCode));
                else if (ke.type == KeyUp) easy_event.sendKeyUp(SDLKeycodeToOSKeyCode(ke.keyCode));
            }
        });

        thread_keyboard_events.detach();

        std::thread thread_screen([&]() {

            while (!client_screen.econnect(host, port_screen, "UDP"));

            while (!quit)
            {
                static int id = 0;

                cv::Mat mat = easy_event.captureScreen();
                resize(mat, mat, cv::Size(), 0.7, 0.7);

                std::vector<uchar> buf;
                compressImage(mat, buf, 70);

                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'I', 128);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_screen.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                // printf("%d\n", box.packets.size());
            }

        });

        thread_screen.detach();

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
    ImGui::InputText("##port_passcode", (char*)port_passcode, 6);
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
    windowTitle  = (char*)"Client";
    windowWidth  = 310;
    windowHeight = 120;

    initEasySocket();
    initEasyImgui();

    srand(time(NULL));

    while (!quit)
    {
        StartNewFrame();

        if (!waiting && !connected) ListeningWindow();
        else if (waiting) waitingingWindow();
        else if (connected) ConnectedWindow();

        Rendering();
        HandleEvents();
    }

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
