# Cross Platform Makefile
# Compatible with MSYS2/MINGW and Mac OS X

IMGUI_DIR = external/imgui/
HYPNO_DIR = lib/hypno/

EXE = src/$(BUILD_TYPE)
SOURCES = $(BUILD_TYPE).cpp

SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp
SOURCES += $(HYPNO_DIR)/hypno_socket.cpp $(HYPNO_DIR)/hypno_event.cpp $(HYPNO_DIR)/hypno_keycode.cpp $(HYPNO_DIR)/hypno_image.cpp $(HYPNO_DIR)/hypno_data.cpp
SOURCES += src/imgui_wrapper.cpp src/frame_wrapper.cpp src/server_connection_manager.cpp src/client_connection_manager.cpp

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
	# LIBS += -L/opt/homebrew/Cellar/raylib/4.5.0/lib

	LIBS += -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
	LIBS += -lGLEW -lSDL2 -lSDL2main
	# LIBS += -lraylib

	CXXFLAGS += `sdl2-config --cflags`

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CXXFLAGS += -I/opt/homebrew/Cellar/opencv/4.8.1_1/include/opencv4
	CXXFLAGS += -I/opt/homebrew/Cellar/glew/2.2.0_1/include
	# CXXFLAGS += -I/opt/homebrew/Cellar/raylib/4.5.0/include

	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"

	BUILD_DIR = $(BUILD_DIR_WIN)

	# LIBS += raylib.a
	LIBS += -LC:\msys64\ucrt64\lib

	LIBS += -lgdi32
	LIBS += -lopengl32 -limm32 `pkg-config --static --libs sdl2`
	LIBS += -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
	LIBS += -lws2_32
	# LIBS += -lraylib
	# LIBS += -lwinmm

	CXXFLAGS += `pkg-config --cflags sdl2`
	# CXXFLAGS += `pkg-config --cflags Qt6Core`
	CXXFLAGS += -IC:/msys64/ucrt64/include/opencv4

	CFLAGS = $(CXXFLAGS)
endif

OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(shell mkdir -p $(BUILD_DIR))

$(BUILD_DIR)/%.o:src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(HYPNO_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
