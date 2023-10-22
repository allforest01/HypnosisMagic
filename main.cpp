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

        // initEasyImgui();

        cv::Mat image;
        // GLuint image_texture;
        // ImVec2 clickPosition, imagePos, mousePosRelativeToImage;

        // Main loop
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

            /* !!! CODE HERE !!! */
            void (*onMouse)(int, int, int, int, void *) = [](int event, int x, int y, int flags, void* userdata) {
                if (event == cv::EVENT_LBUTTONDOWN) {
                    // Left mouse button click
                    std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
                } else if (event == cv::EVENT_MOUSEMOVE) {
                    // Mouse movement
                    std::cout << "Mouse position: (" << x << ", " << y << ")" << std::endl;
                }
            };

            boxman.setCompleteCallback([&image, &onMouse](PacketBox& box) {
                std::vector<uchar> buf;
                PacketBoxToBuf(box, buf);
                decompressImage(buf, image);
                cv::namedWindow("Client Screen");
                cv::setMouseCallback("Client Screen", onMouse);
                cv::imshow("Client Screen", image);
                cv::waitKey(10);
            });

            server.UDPReceive();

            // // ImGui::SetNextWindowSize(ImVec2(image.size().width + 10, image.size().height + 40)); // Adjust the height to accommodate the text
            // ImGui::SetNextWindowPos(ImVec2(60, 100));
            // ImGui::Begin("OpenCV Image", NULL, ImGuiWindowFlags_NoMove);
            // ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.size().width, image.size().height));
            // if (ImGui::IsItemClicked(0)) {
            //     // Get the mouse position relative to the top-left corner of the window
            //     clickPosition = ImGui::GetMousePos();

            //     // You can also get the mouse position relative to the image if needed
            //     imagePos = ImGui::GetItemRectMin();
            //     mousePosRelativeToImage = clickPosition - imagePos;
            // }
            // ImGui::End();

            // // Create a separate ImGui window for the text
            // ImGui::SetNextWindowSize(ImVec2(300, 50)); // Adjust the size as needed
            // // ImGui::SetNextWindowPos(ImVec2(image.size().width + 100, 100)); // Position next to the image window
            // ImGui::Begin("Text Window", NULL, ImGuiWindowFlags_NoMove);
            // // Display the stored mouse position
            // ImGui::Text("Mouse Click Position: (%.2f, %.2f)", mousePosRelativeToImage.x, mousePosRelativeToImage.y);
            // ImGui::End();

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
        char host[256] = "10.37.129.2";
        // char host[256] = "127.0.0.1";
        // printf("host = ");
        // scanf("%s", host);
        EasyClient client(host, port, "UDP");

        int id = 0;

        while (true)
        {
            cv::Mat mat = easy_event.captureScreen();
            resize(mat, mat, cv::Size(), 0.5, 0.5);
            // cv::imshow("Test", mat);
            std::vector<uchar> buff;
            compressImage(mat, buff, 90);

            PacketBox box;
            BufToPacketBox(buff, box, ++id, 0, 1000);

            for (int i = 0; i < (int) box.packets.size(); i++) {
                client.sendData((char*)box.packets[i].data(), box.packets[i].size());
            }
            cv::waitKey(10);
        }
    }

    cleanEasySocket();
    cleanKeyMapping();
    return 0;
}
