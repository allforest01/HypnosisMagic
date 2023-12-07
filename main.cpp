#include <stdio.h>
#include <thread>
#include <mutex>

#include "hypno/hypno_socket.h"
#include "hypno/hypno_event.h"
#include "hypno/hypno_image.h"
#include "hypno/hypno_data.h"
#include "hypno/hypno_imgui.h"

std::mutex mtx;

int main(int argc, char** argv)
{
    initKeyMapping();
    initHypnoSocket();

    char port[] = "3402";
    hypno_event easy_event;

    if (std::stoi(argv[1]) == 1)
    {
        HypnoServer server;
        server.hypnoListen(port, "UDP");
        BoxManager boxman;
        cv::Mat image;
        GLuint image_texture;

        int loopCount = 0;

        boxman.setCompleteCallback(
            [&image, &loopCount](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'I') {
                    std::unique_lock<std::mutex> lock(mtx);
                    decompressImage(buf, image);
                    lock.unlock();
                    if (image.rows > 0 && image.cols > 0)
                    {
                        printf("Image received!\n");
                        printf("loopCount = %d\n", loopCount);
                    }
                }
            }
        );

        server.setService(
            [&boxman](SOCKET sock, char data[], int size, char host[]) {
                std::vector<uchar> buf(data, data + size);
                boxman.addPacketToBox(buf);
            }
        );

        bool quit = false;

        std::thread socketThread([&server, &quit](){
            while (!quit) {
                server.UDPReceive();
            }
        });

        initHypnoImgui();

        ImVec2 mousePosRelativeToImage;

        while (!quit)
        {
            loopCount++;

            // SDL poll event
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) {
                    quit = true;
                }
            }

            // Start a new ImGui frame
            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();

            // Code here !!
            std::unique_lock<std::mutex> lock(mtx);
            MatToTexture(image, image_texture);
            lock.unlock();

            ImGui::SetNextWindowSize(ImVec2(image.size().width + 10, image.size().height + 40));
            ImGui::SetNextWindowPos(ImVec2(60, 100));
            ImGui::Begin("OpenCV Image", NULL, ImGuiWindowFlags_NoMove);
            ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.size().width, image.size().height));
            if (ImGui::IsItemClicked(0)) {
                mousePosRelativeToImage = ImGui::GetMousePos() - ImGui::GetItemRectMin();
            }
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(300, 50));
            ImGui::SetNextWindowPos(ImVec2(image.size().width + 100, 100)); 
            ImGui::Begin("Text Window", NULL, ImGuiWindowFlags_NoMove);
            ImGui::Text("Mouse Click Position: (%.2f, %.2f)", mousePosRelativeToImage.x, mousePosRelativeToImage.y);
            ImGui::End();

            // Rendering
            glViewport(0, 0, window_width, window_height);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            // Swap buffers
            SDL_GL_SwapWindow(window);
        }

        cleanHypnoImgui();

        socketThread.join();
    }
    else
    {
        // char host[16] = "10.211.55.23";
        // char host[16] = "10.37.129.2";

        char host[16] = "127.0.0.1";

        // printf("host = ");
        // scanf("%s", host);

        HypnoClient client;
        client.hypnoConnect(host, port, "UDP");

        int id = 0;

        while (true)
        {
            cv::Mat mat = easy_event.captureScreen();
            resize(mat, mat, cv::Size(), 1, 1);

            std::vector<uchar> buf;
            compressImage(mat, buf, 100);

            PacketBox box;
            BufToPacketBox(buf, box, ++id, 'I', 128);

            for (int i = 0; i < (int) box.packets.size(); i++) {
                client.sendData((char*)box.packets[i].data(), box.packets[i].size());
            }
            cv::waitKey(30);
        }
    }

    cleanHypnoSocket();
    cleanKeyMapping();

    return 0;
}