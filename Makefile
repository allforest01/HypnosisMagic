# Cross Platform Makefile
# Compatible with MSYS2/MINGW and Mac OS X

IMGUI_DIR = lib/imgui/

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
	BUILD_DIR = build/macOS/
else
	BUILD_DIR = build/Windows/
endif

EXE = $(BUILD_DIR)/$(BUILD_TYPE)
SOURCES = src/$(BUILD_TYPE)/$(BUILD_TYPE).cpp

SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp
SOURCES += src/common/socket_manager.cpp src/common/events_manager.cpp src/common/image_manager.cpp src/common/data_manager.cpp src/common/imgui_wrapper.cpp

ifeq ($(BUILD_TYPE), server)
	SOURCES += src/server/frame_wrapper.cpp src/server/server_connection_manager.cpp
else
	SOURCES += src/client/keycode_manager.cpp src/client/client_connection_manager.cpp
endif

CXXFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -Isrc/common -I$(BUILD_TYPE)
CXXFLAGS += -std=c++11 -O3
CXXFLAGS += -Wall -Wformat -g
# CXXFLAGS += -static-libstdc++ -static-libgcc

LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"

	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -framework ApplicationServices -framework Carbon -framework CoreGraphics

	LIBS += -L/usr/lib -L/usr/local/lib
	LIBS += -L/opt/homebrew/Cellar/glew/2.2.0_1/lib
	LIBS += -L/opt/homebrew/Cellar/opencv/4.8.1_1/lib

	LIBS += -lGLEW -lSDL2 -lSDL2main
	LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

	CXXFLAGS += `sdl2-config --cflags`

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CXXFLAGS += -I/opt/homebrew/Cellar/glew/2.2.0_1/include
	CXXFLAGS += -I/opt/homebrew/Cellar/opencv/4.8.1_1/include/opencv4

	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"

	LIBS += -LC:/msys64/ucrt64/lib

	LIBS += -lgdi32 -lws2_32
	LIBS += -lopengl32 -limm32 `pkg-config --static --libs sdl2`
	LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

	CXXFLAGS += `pkg-config --cflags sdl2`
	CXXFLAGS += -IC:/msys64/ucrt64/include/opencv4

	CFLAGS = $(CXXFLAGS)
endif

OBJS = $(addprefix $(BUILD_DIR)/obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(shell mkdir -p build)

$(shell mkdir -p $(BUILD_DIR))

$(shell mkdir -p $(BUILD_DIR)/obj)

$(BUILD_DIR)/obj/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/obj/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/obj/%.o:src/common/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/obj/%.o:src/$(BUILD_TYPE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
