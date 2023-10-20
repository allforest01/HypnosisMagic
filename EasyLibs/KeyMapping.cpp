#include "KeyMapping.h"

std::map<std::string, KeyCodeMapping> keyMapping;

void initKeyMapping() {
    keyMapping["rbKeyUp"] = KeyCodeMapping(126, 26);
    keyMapping["rbKeyDown"] = KeyCodeMapping(125, 28);
    keyMapping["rbKeyLeft"] = KeyCodeMapping(123, 25);
    keyMapping["rbKeyRight"] = KeyCodeMapping(124, 27);
    keyMapping["rbKeyBackspace"] = KeyCodeMapping(117, 8);
    keyMapping["rbKeyEnter"] = KeyCodeMapping(76, -1);
    keyMapping["rbKeyHome"] = KeyCodeMapping(115, 36);
    keyMapping["rbKeyEnd"] = KeyCodeMapping(119, 35);
    keyMapping["rbKeyPageDown"] = KeyCodeMapping(121, 34);
    keyMapping["rbKeyPageUp"] = KeyCodeMapping(116, 33);
    keyMapping["rbKeyReturn"] = KeyCodeMapping(36, 13);
    keyMapping["rbKeyDelete"] = KeyCodeMapping(51, 46);
    keyMapping["rbKeyTab"] = KeyCodeMapping(48, 9);
    keyMapping["rbKeySpacebar"] = KeyCodeMapping(49, 20);
    keyMapping["rbKeyShift"] = KeyCodeMapping(56, 10);
    keyMapping["rbKeyControl"] = KeyCodeMapping(59, 11);
    keyMapping["rbKeyMenu"] = KeyCodeMapping(58, 18);
    keyMapping["rbKeyPrintScreen"] = KeyCodeMapping(-1, 42);
    keyMapping["rbKeyEscape"] = KeyCodeMapping(53, 27);
    keyMapping["rbKeyCapsLock"] = KeyCodeMapping(57, 20);
    keyMapping["rbKeyHelp"] = KeyCodeMapping(114, 47);
    keyMapping["rbKeyF1"] = KeyCodeMapping(122, 112);
    keyMapping["rbKeyF2"] = KeyCodeMapping(120, 113);
    keyMapping["rbKeyF3"] = KeyCodeMapping(99, 114);
    keyMapping["rbKeyF4"] = KeyCodeMapping(118, 115);
    keyMapping["rbKeyF5"] = KeyCodeMapping(96, 116);
    keyMapping["rbKeyF6"] = KeyCodeMapping(97, 117);
    keyMapping["rbKeyF7"] = KeyCodeMapping(98, 118);
    keyMapping["rbKeyF8"] = KeyCodeMapping(100, 119);
    keyMapping["rbKeyF9"] = KeyCodeMapping(101, 120);
    keyMapping["rbKeyF10"] = KeyCodeMapping(109, 121);
    keyMapping["rbKeyF11"] = KeyCodeMapping(103, 122);
    keyMapping["rbKeyF12"] = KeyCodeMapping(111, 123);
    keyMapping["rbKeyMacFn"] = KeyCodeMapping(63, -1);
    keyMapping["rbKeyMacOption"] = KeyCodeMapping(58, -1);
    keyMapping["rbKeyMacCommand"] = KeyCodeMapping(55, -1);
    keyMapping["rbKeyWinLeftWindow"] = KeyCodeMapping(-1, 91);
    keyMapping["rbKeyWinRightWindow"] = KeyCodeMapping(-1, 92);
    keyMapping["rbKeyWinApplication"] = KeyCodeMapping(110, 93);
    keyMapping["rbKeyQ"] = KeyCodeMapping(12, 81);
    keyMapping["rbKeyW"] = KeyCodeMapping(13, 87);
    keyMapping["rbKeyE"] = KeyCodeMapping(14, 69);
    keyMapping["rbKeyR"] = KeyCodeMapping(15, 82);
    keyMapping["rbKeyT"] = KeyCodeMapping(17, 84);
    keyMapping["rbKeyY"] = KeyCodeMapping(16, 89);
    keyMapping["rbKeyU"] = KeyCodeMapping(32, 85);
    keyMapping["rbKeyI"] = KeyCodeMapping(34, 73);
    keyMapping["rbKeyO"] = KeyCodeMapping(31, 79);
    keyMapping["rbKeyP"] = KeyCodeMapping(35, 80);
    keyMapping["rbKeyA"] = KeyCodeMapping(-1, 65);
    keyMapping["rbKeyS"] = KeyCodeMapping(1, 83);
    keyMapping["rbKeyD"] = KeyCodeMapping(2, 68);
    keyMapping["rbKeyF"] = KeyCodeMapping(3, 70);
    keyMapping["rbKeyG"] = KeyCodeMapping(5, 71);
    keyMapping["rbKeyH"] = KeyCodeMapping(4, 72);
    keyMapping["rbKeyJ"] = KeyCodeMapping(38, 74);
    keyMapping["rbKeyK"] = KeyCodeMapping(40, 75);
    keyMapping["rbKeyL"] = KeyCodeMapping(37, 76);
    keyMapping["rbKeyZ"] = KeyCodeMapping(6, 90);
    keyMapping["rbKeyX"] = KeyCodeMapping(7, 88);
    keyMapping["rbKeyC"] = KeyCodeMapping(8, 67);
    keyMapping["rbKeyV"] = KeyCodeMapping(9, 86);
    keyMapping["rbKeyB"] = KeyCodeMapping(11, 66);
    keyMapping["rbKeyN"] = KeyCodeMapping(45, 78);
    keyMapping["rbKeyM"] = KeyCodeMapping(46, 77);
    keyMapping["rbKey0"] = KeyCodeMapping(29, 48);
    keyMapping["rbKey1"] = KeyCodeMapping(18, 49);
    keyMapping["rbKey2"] = KeyCodeMapping(19, 50);
    keyMapping["rbKey3"] = KeyCodeMapping(20, 51);
    keyMapping["rbKey4"] = KeyCodeMapping(21, 52);
    keyMapping["rbKey5"] = KeyCodeMapping(23, 53);
    keyMapping["rbKey6"] = KeyCodeMapping(22, 54);
    keyMapping["rbKey7"] = KeyCodeMapping(26, 55);
    keyMapping["rbKey8"] = KeyCodeMapping(28, 56);
    keyMapping["rbKey9"] = KeyCodeMapping(25, 57);
    keyMapping["rbKeyPeriod"] = KeyCodeMapping(47, 190);
    keyMapping["rbKeyComma"] = KeyCodeMapping(43, 188);
    keyMapping["rbKeySlash"] = KeyCodeMapping(44, 191);
    keyMapping["rbKeyNum0"] = KeyCodeMapping(82, 96);
    keyMapping["rbKeyNum1"] = KeyCodeMapping(83, 97);
    keyMapping["rbKeyNum2"] = KeyCodeMapping(84, 98);
    keyMapping["rbKeyNum3"] = KeyCodeMapping(85, 99);
    keyMapping["rbKeyNum4"] = KeyCodeMapping(86, 100);
    keyMapping["rbKeyNum5"] = KeyCodeMapping(87, 101);
    keyMapping["rbKeyNum6"] = KeyCodeMapping(88, 102);
    keyMapping["rbKeyNum7"] = KeyCodeMapping(89, 103);
    keyMapping["rbKeyNum8"] = KeyCodeMapping(91, 104);
    keyMapping["rbKeyNum9"] = KeyCodeMapping(92, 105);
    keyMapping["rbKeyMultiply"] = KeyCodeMapping(67, 106);
    keyMapping["rbKeyAdd"] = KeyCodeMapping(69, 107);
    keyMapping["rbKeySubtract"] = KeyCodeMapping(78, 109);
    keyMapping["rbKeyDivide"] = KeyCodeMapping(75, 111);
    keyMapping["rbKeyDecimal"] = KeyCodeMapping(65, 110);
    keyMapping["rbKeyNumEqual"] = KeyCodeMapping(81, -1);
}

int MacKeyToWinKey(int macKey) {
    for (const auto& pair : keyMapping) {
        if (pair.second.macKey == macKey) {
            return pair.second.winKey;
        }
    }
    return -1;
}

int WinKeyToMacKey(int winKey) {
    for (const auto& pair : keyMapping) {
        if (pair.second.winKey == winKey) {
            return pair.second.macKey;
        }
    }
    return -1;
}
