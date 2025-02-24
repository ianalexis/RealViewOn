#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "teclado.h"

using std::string;
class AdvanceMode {
public:
    AdvanceMode(); // Constructor
    void setSwVersion(int swVersion);
    string askAdvanceOptions();

private:
    const string rutaBase = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    string rutaVersionada = "";
    string completeBase (string complement);

    struct multiRegSetting {
        string path;
        std::vector<string> value;
    };
    
    struct RegSetting {
        string name;
        multiRegSetting content;
    };

    const std::vector<RegSetting> regSettings = {
        {"Reverse Mouse Wheel",
            {"General",
                {"Reverse Zoom Direction\"=dword:00000001"}}},
        {"DarkMode",
            {"General",
                {"Color Theme ID\"=dword:00000003\n;Light: 1, Medium: 2, Dark: 3, Medium Light: 4."}}},
        {"FPS Viewer",
            {"Performance",
                {"OpenGL Print Statistics\"=dword:00000001"}}},
        {"Performance Enhance graphics (Recommended)",
            {"Performance",
                {"Use Performance Pipeline 2020\"=dword:00000001",
                "Use GPU Silhouette Edges\"=dword:00000001"}}},
        {"Full AntiAliasing (Recommended)",
            {"Edges",
                {"Show Anti Alias Shaded Edges\"=dword:00000000",
                "Show Fullscene Anti Alias\"=dword:00000001"}}},
        {"Advance Menu Options (Recommended)",
            {"TBD",
                {"TBD\"=dword:TBD"}}}
    };
};

