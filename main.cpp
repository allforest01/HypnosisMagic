#define IMGUI_DEFINE_MATH_OPERATORS

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
#include "EasyLibs/EasyImgui.h"

int main(int argc, char** argv)
{
    char port[] = "3402";
    EasyEvent easy_event;

    initKeyMapping();
    initEasySocket();

    if (std::stoi(argv[1]) == 1)
    {
        EasyServer server(port, "UDP");
        BoxManager boxman;

        server.setServices([&boxman](SOCKET sock, char data[], int size) {
            std::vector<uchar> buf(data, data + size);
            boxman.addPacketToBox(buf);
        });

        cv::Mat image;

        // initEasyImgui();

        bool quit = false;
        while (!quit)
        {
            // // SDL poll event
            // SDL_Event event;
            // while (SDL_PollEvent(&event)) {
            //     ImGui_ImplSDL2_ProcessEvent(&event);
            //     if (event.type == SDL_QUIT) {
            //         quit = true;
            //     }
            // }

            // // Start a new ImGui frame
            // ImGui_ImplOpenGL2_NewFrame();
            // ImGui_ImplSDL2_NewFrame(window);
            // ImGui::NewFrame();

            void (*onMouse)(int, int, int, int, void *) = [](int event, int x, int y, int flags, void* userdata) {
                if (event == cv::EVENT_LBUTTONDOWN) {
                    std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
                } else if (event == cv::EVENT_MOUSEMOVE) {
                    std::cout << "Mouse position: (" << x << ", " << y << ")" << std::endl;
                }
            };

            boxman.setCompleteCallback([&image, &onMouse](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                if (box.type == 'I') {
                    decompressImage(buf, image);
                    cv::namedWindow("Client Screen");
                    cv::setMouseCallback("Client Screen", onMouse);
                    cv::imshow("Client Screen", image);
                    cv::waitKey(10);
                }
                else if (box.type == 'L') {
                    // printf("L: %s\n", buf.data());
                }
            });

            server.UDPReceive();

            // // Rendering
            // glViewport(0, 0, windowWidth, windowHeight);
            // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            // glClear(GL_COLOR_BUFFER_BIT);
            // ImGui::Render();
            // ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            // // Swap buffers
            // SDL_GL_SwapWindow(window);
        }

        // cleanEasyImgui();
    }
    else
    {
        // char host[256] = "10.211.55.23";
        // char host[256] = "10.37.129.2";
        char host[256] = "127.0.0.1";
        // printf("host = ");
        // scanf("%s", host);
        EasyClient client(host, port, "UDP");

        int id = 0;

        while (true)
        {
            cv::Mat mat = easy_event.captureScreen();
            resize(mat, mat, cv::Size(), 0.5, 0.5);
            std::vector<uchar> buf;
            compressImage(mat, buf, 90);

            PacketBox box;
            BufToPacketBox(buf, box, ++id, 'I', 1000);

            for (int i = 0; i < (int) box.packets.size(); i++) {
                client.sendData((char*)box.packets[i].data(), box.packets[i].size());
            }
            cv::waitKey(30);

            std::string str = "Magic number: " + std::to_string(rand());
            std::vector<uchar> buf2(str.begin(), str.end());
            PacketBox box2;
            BufToPacketBox(buf2, box2, ++id, 'L', 1);
            client.sendData((char*)box2.packets[0].data(), box2.packets[0].size());
            cv::waitKey(30);
        }
    }

    cleanEasySocket();
    cleanKeyMapping();
    return 0;
}
