@REM g++ -o main *.cpp -std=c++11 -lws2_32 && .\main
g++ -o main *.cpp -IC:/msys64/ucrt64/include/opencv4 -LC:\msys64\ucrt64\lib -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio -lgdi32 -lws2_32 && .\main