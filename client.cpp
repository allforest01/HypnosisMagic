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

#define port_passcoode "3401"
#define port_screen    "3402"
#define port_mouse     "3403"
#define port_keyboard  "3404"

char host[16] = "127.0.0.1";
char passcode[5] = "0000";

EasyServer server_passcode, server_mouse, server_keyboard;
EasyClient client_screen;

std::thread thread_passcode, thread_screen, thread_mouse, thread_keyboard;

BoxManager boxman;

bool quit = false, waiting = false;

EasyEvent easy_event;
int connection_phase = 0;
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
}

void ListeningWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");
    // Open UDP socket to waiting for connect from server
    ImGui::Text("Open a port to waiting for a connection");
    ImGui::PushItemWidth(200);
    ImGui::InputText("##port_passcode", (char*)port_passcoode, 6);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Start")) {
        // waiting for a connection
        for (int i = 0; i < 4; i++) {
            passcode[i] = rand() % 10 + '0';
        }

        thread_passcode = std::thread([](){
            server_passcode.setService(
                [](SOCKET sock, char data[], int size, char ipv4[]) {
                    if (!strcmp(data, passcode) || !strcmp(data, "0000")) {
                        strcpy(host, ipv4);
                        waiting = false;
                        connection_phase = 1;
                    }
                }
            );

            server_passcode.elisten(port_passcoode, "UDP");
            
            while (!quit && waiting) {
                server_passcode.UDPReceive(5);
            }
        });

        waiting = true;
    }
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
        server_passcode.eclose();
        thread_passcode.join();
    }
    ImGui::End();
}

void ConnectedWindow() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::SetNextWindowSize(ImVec2(270, 80));
    ImGui::Begin("Port");

    // When client is connected from server
    if (connection_phase == 1)
    {
        server_passcode.eclose();
        thread_passcode.join();

        while (!client_screen.econnect(host, port_screen, "TCP"));

        thread_mouse = std::thread([]()
        {
            boxman.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'K')
                {
                    KeyboardEvent *ke = (KeyboardEvent*)buf.data();
                    if (ke->type == KeyDown) easy_event.sendKeyDown(SDLKeycodeToOSKeyCode(ke->keyCode));
                    else if (ke->type == KeyUp) easy_event.sendKeyUp(SDLKeycodeToOSKeyCode(ke->keyCode));
                }
                else if (box.type == 'M')
                {
                    MouseEvent *me = (MouseEvent*)buf.data();

                    int x = round(me->x * easy_event.width);
                    int y = round(me->y * easy_event.height);

                    sprintf(debug, "Send Mouse %d %d\n", x, y);

                    if (me->type == LDown) easy_event.sendLDown(x, y);
                    else if (me->type == LUp) easy_event.sendLUp(x, y);
                    else if (me->type == RDown) easy_event.sendRDown(x, y);
                    else if (me->type == RUp) easy_event.sendRUp(x, y);
                    else if (me->type == MouseMove) easy_event.sendMove(x, y);
                }
            });

            server_mouse.setService(
                [](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman.addPacketToBox(buf);
                }
            );

            server_mouse.elisten(port_mouse, "TCP");

            while (!quit) server_mouse.TCPReceive(24);
        });

        thread_keyboard = std::thread([]()
        {
            server_keyboard.setService(
                [](SOCKET sock, char data[], int size, char host[]) {
                    std::vector<uchar> buf(data, data + size);
                    boxman.addPacketToBox(buf);
                }
            );

            server_keyboard.elisten(port_keyboard, "TCP");

            while (!quit) server_keyboard.TCPReceive(24);
        });

        thread_screen = std::thread([]() {

            int id = 0;

            while (!quit)
            {
                cv::Mat mat = easy_event.captureScreen();
                resize(mat, mat, cv::Size(), 0.7, 0.7);

                std::vector<uchar> buf;
                compressImage(mat, buf, 70);

                PacketBox box;
                BufToPacketBox(buf, box, ++id, 'I', 1440);

                for (int i = 0; i < (int) box.packets.size(); i++) {
                    client_screen.sendData((char*)box.packets[i].data(), box.packets[i].size());
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }

        });
    }

    connection_phase = 2;

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
        HandleEvents();
        StartNewFrame();

        if (!waiting && !connection_phase) ListeningWindow();
        else if (waiting) waitingingWindow();
        else if (connection_phase) ConnectedWindow();

        Rendering();
    }

    thread_screen.join();
    thread_mouse.join();
    thread_keyboard.join();

    cleanEasyImgui();
    cleanEasySocket();

    return 0;
}
