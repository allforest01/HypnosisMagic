#include <stdio.h>
#include <chrono>
#include <thread>

#include "image_manager.h"
#include "data_manager.h"
#include "client_wrapper.h"
#include "imgui_wrapper.h"
#include "keycode_manager.h"

#define SECRET "HYPNO"

#define PORT_A "63840"
#define PORT_B "63841"
#define PORT_C "63842"

#define SCREEN_STREAM_TYPE "UDP"
#define NUM_OF_THREADS 1
#define PACKET_SIZE 1468