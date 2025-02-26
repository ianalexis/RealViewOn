#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "teclado.h"

using std::string;
class AdvanceMode {
public:
    AdvanceMode(); // Constructor
    void setSwVersion(int swVersion, bool generico);
    string askAdvanceOptions();

private:
    bool generico = false;
    int optionsQty = 0;
    int optionNumber = 1;
    const string rutaBase = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    string rutaVersionada = "";
    const string rutaTab = "User Interface\\CommandManager\\PartContext\\Tab";
    string completeBase (string complement);
    bool anySelected = false;
    std::vector <int> tabsToEnable = {4,5,7,8,11};

    string selectRegOptions();
    string qolCommands();
    string enableTabs();
    string enableTab(string value);
    string getOriginalValue(string tabPath, string valueName);
    string enableBtn();
    void AdvanceMode::askToEnable(string optionName);

    struct multiRegSetting {
        string path;
        std::vector<string> value;
    };
    
    struct RegSetting {
        string name;
        std::vector<multiRegSetting> content;
    };

    const std::vector<RegSetting> regOptions = {
        {"Reverse Mouse Wheel",
            {{"General",
                {"\"Reverse Zoom Direction\"=dword:00000001"}}}},
        {"DarkMode",
            {{"General",
                {"\"Color Theme ID\"=dword:00000003\n;Light: 1, Medium: 2, Dark: 3, Medium Light: 4."}},
            {"Colors",
                {"\"Assembly, Edit Part\"=\"16776960\"",
                "\"Tree Item Normal\"=\"15790320\""}}}},
        {"FPS Viewer",
            {{"Performance",
                {"\"OpenGL Print Statistics\"=dword:00000001"}}}},
        {"Performance Enhance graphics (Recommended)",
            {{"Performance",
                {"\"Use Performance Pipeline 2020\"=dword:00000001",
                "\"Use GPU Silhouette Edges\"=dword:00000001"}}}},
        {"Full AntiAliasing (Recommended)",
            {{"Edges",
                {"\"Show Anti Alias Shaded Edges\"=dword:00000000",
                "\"Show Fullscene Anti Alias\"=dword:00000001"}}}},
        {"Spin Box Increment 1mm (Recommended)",
            {{"General",
                {"\"Length Increment (Metric)\"=\"0.001\""}}}}
    };

    const std::vector<multiRegSetting> btnsToEnable = {
        {"User Interface\\CommandManager\\PartContext\\Tab11\\GB4",
            {"2,34247",
            "2,37915",
            "2,38240"}},
        {"User Interface\\ViewTools\\Part-Assy",
            {"33594"}}
    };  
};