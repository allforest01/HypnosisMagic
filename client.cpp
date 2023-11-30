#include <stdio.h>
#include <thread>
#include <mutex>

#include "EasyLibs/EasySocket.h"
#include "EasyLibs/EasyEvent.h"
#include "EasyLibs/EasyImage.h"
#include "EasyLibs/EasyData.h"
#include "EasyLibs/EasyImgui.h"

std::mutex mtx;

int main(int argc, char** argv)
{
    initKeyMapping();
    initEasySocket();

    char port[] = "3402";
    EasyEvent easy_event;

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
        resize(mat, mat, cv::Size(), 0.7, 0.7);
        printf("size = %d\n", mat.rows * mat.cols * 3);

        std::vector<uchar> buf;
        compressImage(mat, buf, 70);

        PacketBox box;
        BufToPacketBox(buf, box, ++id, 'I', 128);

        for (int i = 0; i < (int) box.packets.size(); i++) {
            client.sendData((char*)box.packets[i].data(), box.packets[i].size());
        }
        cv::waitKey(20);
    }

    cleanEasySocket();
    cleanKeyMapping();

    return 0;
}
