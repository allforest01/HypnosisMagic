#include "KeyMapping.h"

int KeyMapping::convertMacToWindows(int macKey) {
    for (const auto& pair : this->keyMapping) {
        if (pair.second.windows == macKey) {
            return pair.second.windows;
        }
    }
    return -1;
}

int KeyMapping::convertWindowsToMac(int windowsKey) {
    for (const auto& pair : this->keyMapping) {
        if (pair.second.windows == windowsKey) {
            return pair.second.mac;
        }
    }
    return -1;
}
