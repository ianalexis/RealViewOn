#pragma once
#include <string>
#include <iostream>
#include "teclado.h"

using std::string;
class AdvanceMode {
public:
    AdvanceMode(int swVersion); // Constructor
    void setSwVersion(int swVersion);
    string askAdvanceOptions();

private:
    const string rutaBase = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    string rutaVersionada = "";
    string completeBase (string complement);
    
    struct RegSetting {
        string name;
        string content;
    };

    RegSetting regSettings[6] = {
        {"Reverse Mouse Wheel", "\\General]\n\"Reverse Zoom Direction\"=dword:00000001"},
        {"DarkMode", "\\General]\n\"Color Theme ID\"=dword:00000003\n;Light: 1, Medium: 2, Dark: 3, Medium Light: 4."},
        {"FPS Viewer", "\\Performance]\n\"OpenGL Print Statistics\"=dword:00000001"},
        {"Advance Menu Options (Recommended)", "TBD]\n\"TBD\"=dword:TBD"}, // TODO: Definir
        {"Performance Enhance graphics (Recommended)", "\\Performance]\n\"Use Performance Pipeline 2020\"=dword:00000001\n\"Use GPU Silhouette Edges\"=dword:00000001"},
        {"Full AntiAliasing (Recommended)", "\\Edges]\n\"Show Anti Alias Shaded Edges\"=dword:00000000\n\"Show Fullscene Anti Alias\"=dword:00000001"}
    };
};;

