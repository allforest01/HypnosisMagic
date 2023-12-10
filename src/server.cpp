#define SECRET "aBcXyZ"

#define PORT_P "50300"
#define PORT_M "50301"
#define PORT_K "50302"
#define PORT_S "50303"

#include "../include/server.h"

char host[16] = "10.211.55.255";

FrameWrapper frame_wrapper;
ImGuiWrapper imgui_wrapper;

HypnoClient client_passcode;
HypnoServer server_passcode;
HypnoClient client_mouse;
HypnoClient client_keyboard;
ServerConnectionManager server_screen;

std::queue<MouseEvent> mouse_events;
std::queue<KeyboardEvent> keyboard_events;
std::mutex mtx_mouse, mtx_keyboard, mtx_frame;

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

        server_passcode.hypnoListen(PORT_P, "TCP");

        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(server_passcode.client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
        strcpy(host, ipv4);
        printf("host = %s\n", host);
        fflush(stdout);

        server_passcode.hypnoClose();

        // client_mouse send mouse events
        while (!client_mouse.hypnoConnect(host, PORT_M, "UDP"));

        printf("Done mouse connect!\n");

        // client_mouse send mouse events
        while (!client_keyboard.hypnoConnect(host, PORT_K, "UDP"));

        printf("Done keyboard connect!\n");

        printf("PORT_S = %s\n", (char*) PORT_S);
        printf("atoi(PORT_S) = %d\n", atoi((char*)PORT_S));

        printf("PORT_S = %d\n", atoi((char*)PORT_S));
        server_screen.listen(host, atoi((char*)PORT_S), "UDP", 4);

        printf("Done screen listen!\n");

        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::thread thread_mouse([&](){
            while (!quit) {
                std::unique_lock<std::mutex> lock(mtx_mouse);
                if (!mouse_events.size()) {
                    mtx_mouse.unlock();
                    continue;
                }

                MouseEvent me = mouse_events.front(); mouse_events.pop();
                mtx_mouse.unlock();
    
                me.x /= frame_wrapper.width;
                me.y /= frame_wrapper.height;

                if (me.x < 0.0 || me.x > 1.0 || me.y < 0.0 || me.y > 1.0) continue;

                // printf("Mouse event %lf %lf\n", me.x, me.y);

                std::vector<uchar> image_data((char*)&me, (char*)(&me) + sizeof(me));

                static int id = 0;

                PacketBox box;
                BufToPacketBox(image_data, box, ++id, 'M', image_data.size() + 7);

                for (int i = 0; i < (int) box.data.size(); i++) {
                    client_mouse.sendData((char*)box.data[i].data(), box.data[i].size());
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

                for (int i = 0; i < (int) box.data.size(); i++) {
                    client_keyboard.sendData((char*)box.data[i].data(), box.data[i].size());
                }
            }
        });

        thread_keyboard.detach();

        // Receive screen capture
        std::thread thread_screen([&]()
        {
            printf("Start thread_screen\n");
            fflush(stdout);

            server_screen.setCompleteCallback([](PacketBox& box) {
                std::vector<uchar> image_data;
                box.sort();
                PacketBoxToBuf(box, image_data);
                if (box.type == 'I')
                {
                    // if (frame_wrapper.frame_queue.size() <= 2)
                    std::unique_lock<std::mutex> lock_frame(mtx_frame);
                    frame_wrapper.frame_queue.push(image_data);
                    lock_frame.unlock();
                }
            });

            // printf("Done!");

            while (!quit) {
                static int cnt = 0;

                auto start = std::chrono::high_resolution_clock::now();

                server_screen.receive();

                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;

                printf("server_screen.receive = %lf\n", duration.count());
                printf("cnt = %d\n", ++cnt);

                // std::this_thread::sleep_for(std::chrono::milliseconds(16));

                // break;
            }
            
        });

        thread_screen.detach();

        // Connected
        connected = true;
    });

    thread_passcode.detach();
}

void menuBar() {
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

void connectionWindow() {
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
    ImGui::InputText("##PORT_P", (char*)PORT_P, 6);
    ImGui::PopItemWidth();

    ImGui::Text("Code");
    ImGui::SameLine();
    ImGui::PushItemWidth(135);
    ImGui::InputText("##SECRET", (char*)SECRET, 5);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Connect")) connectButtonHandle();

    ImGui::End();
}

void clientScreenWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 30));
    ImGui::SetNextWindowSize(ImVec2(922, 600));

    ImGui::Begin("Screen");

    if (frame_wrapper.frame_queue.size()) {
        std::unique_lock<std::mutex> lock_frame(mtx_frame);
        auto frame = frame_wrapper.frame_queue.front(); frame_wrapper.frame_queue.pop();
        lock_frame.unlock();

        frame_wrapper.pushToTexture(frame);
    }

    ImGui::ImageButton((void*)(intptr_t)frame_wrapper.image_texture, ImVec2(frame_wrapper.width, frame_wrapper.height));

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

int main(int argc, char** argv)
{
    initHypnoSocket();
    imgui_wrapper = ImGuiWrapper(1200, 640, (char*)"Server");
    initImGui(imgui_wrapper);
    frame_wrapper.initTexture();

    while (!quit)
    {
        startNewFrame();

        menuBar();
        connectionWindow();
        clientScreenWindow();

        guiRendering();
        handleEvents();
    }

    cleanImGui(imgui_wrapper);
    frame_wrapper.cleanTexture();
    server_screen.clean();
    cleanHypnoSocket();

    return 0;
}
