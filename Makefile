#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW and Mac OS X
#
# Mac OS X:
#	brew install gcc
#	brew install qt
#	brew install opencv
#	brew install sdl2
#	brew install glew
# MSYS2:
#	pacman -S mingw-w64-ucrt-x86_64-toolchain
#	pacman -S mingw-w64-ucrt-x86_64-qt6-base
#	pacman -S mingw-w64-ucrt-x86_64-opencv
#	pacman -S mingw-w64-ucrt-x86_64-SDL2
#	pacman -S mingw-w64-ucrt-x86_64-glew

#CXX = g++
#CXX = clang++

EXE = main
IMGUI_DIR = imgui/
EASYLIBS_DIR = EasyLibs/

SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(EASYLIBS_DIR)/EasySocket.cpp $(EASYLIBS_DIR)/EasyEvent.cpp $(EASYLIBS_DIR)/KeyMapping.cpp $(EASYLIBS_DIR)/EasyImage.cpp $(EASYLIBS_DIR)/EasyData.cpp $(EASYLIBS_DIR)/EasyImgui.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp

BUILD_DIR_MAC = build_mac/
BUILD_DIR_WIN = build_win/

UNAME_S := $(shell uname -s)

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat

LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"

	BUILD_DIR = $(BUILD_DIR_MAC)

	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -framework ApplicationServices -framework Carbon

	LIBS += -L/usr/local/lib
	LIBS += -L/opt/homebrew/Cellar/opencv/4.8.1_1/lib
	LIBS += -L/opt/homebrew/Cellar/glew/2.2.0_1/lib

	LIBS += -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
	LIBS += -lGLEW -lSDL2 -lSDL2main

	CXXFLAGS += -std=c++11
	CXXFLAGS += `sdl2-config --cflags`

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CXXFLAGS += -I/opt/homebrew/Cellar/opencv/4.8.1_1/include/opencv4
	CXXFLAGS += -I/opt/homebrew/Cellar/glew/2.2.0_1/include

	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"

	BUILD_DIR = $(BUILD_DIR_WIN)

	LIBS += -LC:\msys64\ucrt64\lib

	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`
	LIBS += -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
	LIBS += -lws2_32

	CXXFLAGS += -std=c++11
	CXXFLAGS += `pkg-config --cflags sdl2`
	CXXFLAGS += -IC:/msys64/ucrt64/include/opencv4

	CFLAGS = $(CXXFLAGS)
endif

OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(shell mkdir -p $(BUILD_DIR))

$(BUILD_DIR)/%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(EASYLIBS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
