#pragma once

#include <iostream>
#include <map>
#include <string>

struct KeyCodeMapping {
    int macKey, winKey;
    KeyCodeMapping() {}
    KeyCodeMapping(int macKey, int winKey): macKey(macKey), winKey(winKey) {}
};

void initKeyMapping();

int MacKeyToWinKey(int);
int WinKeyToMacKey(int);
