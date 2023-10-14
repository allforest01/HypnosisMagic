#pragma once

#include <iostream>
#include <map>
#include <string>

struct KeyCodeMapping {
    int mac;
    int windows;
};

class KeyMapping {
private:
    std::map<std::string, KeyCodeMapping> keyMapping;
public:
    KeyMapping() {
        keyMapping["rbKeyUp"] = {126, 26};
        keyMapping["rbKeyDown"] = {125, 28};
        keyMapping["rbKeyLeft"] = {123, 25};
        keyMapping["rbKeyRight"] = {124, 27};
        keyMapping["rbKeyBackspace"] = {117, 8};
        keyMapping["rbKeyEnter"] = {76, -1};
        keyMapping["rbKeyHome"] = {115, 36};
        keyMapping["rbKeyEnd"] = {119, 35};
        keyMapping["rbKeyPageDown"] = {121, 34};
        keyMapping["rbKeyPageUp"] = {116, 33};
        keyMapping["rbKeyReturn"] = {36, 13};
        keyMapping["rbKeyDelete"] = {51, 46};
        keyMapping["rbKeyTab"] = {48, 9};
        keyMapping["rbKeySpacebar"] = {49, 20};
        keyMapping["rbKeyShift"] = {56, 10};
        keyMapping["rbKeyControl"] = {59, 11};
        keyMapping["rbKeyMenu"] = {58, 18};
        keyMapping["rbKeyPrintScreen"] = {-1, 42};
        keyMapping["rbKeyEscape"] = {53, 27};
        keyMapping["rbKeyCapsLock"] = {57, 20};
        keyMapping["rbKeyHelp"] = {114, 47};
        keyMapping["rbKeyF1"] = {122, 112};
        keyMapping["rbKeyF2"] = {120, 113};
        keyMapping["rbKeyF3"] = {99, 114};
        keyMapping["rbKeyF4"] = {118, 115};
        keyMapping["rbKeyF5"] = {96, 116};
        keyMapping["rbKeyF6"] = {97, 117};
        keyMapping["rbKeyF7"] = {98, 118};
        keyMapping["rbKeyF8"] = {100, 119};
        keyMapping["rbKeyF9"] = {101, 120};
        keyMapping["rbKeyF10"] = {109, 121};
        keyMapping["rbKeyF11"] = {103, 122};
        keyMapping["rbKeyF12"] = {111, 123};
        keyMapping["rbKeyMacFn"] = {63, -1};
        keyMapping["rbKeyMacOption"] = {58, -1};
        keyMapping["rbKeyMacCommand"] = {55, -1};
        keyMapping["rbKeyWinLeftWindow"] = {-1, 91};
        keyMapping["rbKeyWinRightWindow"] = {-1, 92};
        keyMapping["rbKeyWinApplication"] = {110, 93};
        keyMapping["rbKeyQ"] = {12, 81};
        keyMapping["rbKeyW"] = {13, 87};
        keyMapping["rbKeyE"] = {14, 69};
        keyMapping["rbKeyR"] = {15, 82};
        keyMapping["rbKeyT"] = {17, 84};
        keyMapping["rbKeyY"] = {16, 89};
        keyMapping["rbKeyU"] = {32, 85};
        keyMapping["rbKeyI"] = {34, 73};
        keyMapping["rbKeyO"] = {31, 79};
        keyMapping["rbKeyP"] = {35, 80};
        keyMapping["rbKeyA"] = {-1, 65};
        keyMapping["rbKeyS"] = {1, 83};
        keyMapping["rbKeyD"] = {2, 68};
        keyMapping["rbKeyF"] = {3, 70};
        keyMapping["rbKeyG"] = {5, 71};
        keyMapping["rbKeyH"] = {4, 72};
        keyMapping["rbKeyJ"] = {38, 74};
        keyMapping["rbKeyK"] = {40, 75};
        keyMapping["rbKeyL"] = {37, 76};
        keyMapping["rbKeyZ"] = {6, 90};
        keyMapping["rbKeyX"] = {7, 88};
        keyMapping["rbKeyC"] = {8, 67};
        keyMapping["rbKeyV"] = {9, 86};
        keyMapping["rbKeyB"] = {11, 66};
        keyMapping["rbKeyN"] = {45, 78};
        keyMapping["rbKeyM"] = {46, 77};
        keyMapping["rbKey0"] = {29, 48};
        keyMapping["rbKey1"] = {18, 49};
        keyMapping["rbKey2"] = {19, 50};
        keyMapping["rbKey3"] = {20, 51};
        keyMapping["rbKey4"] = {21, 52};
        keyMapping["rbKey5"] = {23, 53};
        keyMapping["rbKey6"] = {22, 54};
        keyMapping["rbKey7"] = {26, 55};
        keyMapping["rbKey8"] = {28, 56};
        keyMapping["rbKey9"] = {25, 57};
        keyMapping["rbKeyPeriod"] = {47, 190};
        keyMapping["rbKeyComma"] = {43, 188};
        keyMapping["rbKeySlash"] = {44, 191};
        keyMapping["rbKeyNum0"] = {82, 96};
        keyMapping["rbKeyNum1"] = {83, 97};
        keyMapping["rbKeyNum2"] = {84, 98};
        keyMapping["rbKeyNum3"] = {85, 99};
        keyMapping["rbKeyNum4"] = {86, 100};
        keyMapping["rbKeyNum5"] = {87, 101};
        keyMapping["rbKeyNum6"] = {88, 102};
        keyMapping["rbKeyNum7"] = {89, 103};
        keyMapping["rbKeyNum8"] = {91, 104};
        keyMapping["rbKeyNum9"] = {92, 105};
        keyMapping["rbKeyMultiply"] = {67, 106};
        keyMapping["rbKeyAdd"] = {69, 107};
        keyMapping["rbKeySubtract"] = {78, 109};
        keyMapping["rbKeyDivide"] = {75, 111};
        keyMapping["rbKeyDecimal"] = {65, 110};
        keyMapping["rbKeyNumEqual"] = {81, -1};
    }
    int convertMacToWindows(int);
    int convertWindowsToMac(int);
};
