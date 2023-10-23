#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
// #include "EasyLibs/EasyImgui.h"

int main(int argc, char** argv)
{
    initKeyMapping();
    initEasySocket();

    char port[] = "3402";
    EasyEvent easy_event;

    if (std::stoi(argv[1]) == 1)
    {
        EasyServer server(port, "UDP");
        BoxManager boxman;

        server.setService([&boxman](SOCKET sock, char data[], int size) {
            std::vector<uchar> buf(data, data + size);
            boxman.addPacketToBox(buf);
        });

        cv::Mat image;

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
                if (image.rows > 0 && image.cols > 0 ) {
                    cv::namedWindow("Client Screen");
                    cv::setMouseCallback("Client Screen", onMouse);
                    cv::imshow("Client Screen", image);
                    cv::waitKey(10);
                }
            }
            else if (box.type == 'L') {
                // printf("L: %s\n", buf.data());
            }
        });

        bool quit = false;

        while (!quit) {
            server.UDPReceive();
        }

        // initEasyImgui();

        // ImVec2 clickPosition, imagePos, mousePosRelativeToImage;

        // while (!quit) {
        //     // SDL poll event
        //     SDL_Event event;
        //     while (SDL_PollEvent(&event)) {
        //         ImGui_ImplSDL2_ProcessEvent(&event);
        //         if (event.type == SDL_QUIT) {
        //             quit = true;
        //         }
        //     }

        //     // Start a new ImGui frame
        //     ImGui_ImplOpenGL2_NewFrame();
        //     ImGui_ImplSDL2_NewFrame(window);
        //     ImGui::NewFrame();

        //     GLuint image_texture = MatToTexture(image);

        //     ImGui::SetNextWindowSize(ImVec2(image.size().width + 10, image.size().height + 40)); // Adjust the height to accommodate the text
        //     ImGui::SetNextWindowPos(ImVec2(60, 100));

        //     ImGui::Begin("OpenCV Image", NULL, ImGuiWindowFlags_NoMove);

        //     ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.size().width, image.size().height));

        //     if (ImGui::IsItemClicked(0)) {
        //         // Get the mouse position relative to the top-left corner of the window
        //         clickPosition = ImGui::GetMousePos();

        //         // You can also get the mouse position relative to the image if needed
        //         imagePos = ImGui::GetItemRectMin();
        //         mousePosRelativeToImage = clickPosition - imagePos;
        //     }

        //     ImGui::End();

        //     // Create a separate ImGui window for the text
        //     ImGui::SetNextWindowSize(ImVec2(300, 50)); // Adjust the size as needed
        //     ImGui::SetNextWindowPos(ImVec2(image.size().width + 100, 100)); // Position next to the image window
        //     ImGui::Begin("Text Window", NULL, ImGuiWindowFlags_NoMove);

        //     // Display the stored mouse position
        //     ImGui::Text("Mouse Click Position: (%.2f, %.2f)", mousePosRelativeToImage.x, mousePosRelativeToImage.y);

        //     ImGui::End();

        //     // Rendering
        //     glViewport(0, 0, windowWidth, windowHeight);
        //     glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        //     glClear(GL_COLOR_BUFFER_BIT);
        //     ImGui::Render();
        //     ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        //     // Swap buffers
        //     SDL_GL_SwapWindow(window);
        // }

        // cleanEasyImgui();
    }
    else
    {
        // char host[16] = "10.211.55.23";
        // char host[16] = "10.37.129.2";

        char host[16] = "127.0.0.1";

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
            BufToPacketBox(buf, box, ++id, 'I', 128);

            for (int i = 0; i < (int) box.packets.size(); i++) {
                client.sendData((char*)box.packets[i].data(), box.packets[i].size());
            }
            cv::waitKey(20);

            // std::string str = "Magic number: " + std::to_string(rand());
            // std::vector<uchar> buf2(str.begin(), str.end());

            // PacketBox box2;
            // BufToPacketBox(buf2, box2, ++id, 'L', 1);

            // client.sendData((char*)box2.packets[0].data(), box2.packets[0].size());
            // cv::waitKey(20);
        }
    }

    cleanEasySocket();
    cleanKeyMapping();
    return 0;
}
