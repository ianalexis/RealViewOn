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
        std::vector<multiRegSetting> content;
    };

    const std::vector<RegSetting> advanceRegOptions = {
        {"Reverse Mouse Wheel",
            {{"General",
                {"\"Reverse Zoom Direction\"=dword:00000001"}}}},
        {"DarkMode",
            {{"General",
                {"\"Color Theme ID\"=dword:00000003\n;Light: 1, Medium: 2, Dark: 3, Medium Light: 4."}}}},
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
        {"QoL Commands",
            {{"User Interface\\CommandManager\\PartContext\\Tab11\\GB5",//TODO: Make it smart searching the last GB in the path and adding the new GB in the next position.
                {"\"Btn0\"=\"2,34247\"",
                "\"Btn1\"=\"2,37915\"",
                "\"Btn2\"=\"2,38240\""}},
            {"User Interface\\ViewTools\\Part-Assy",
                {"\"Btn11\"=\"33594\""}}}}//TODO: Make it smart searching the last Btn in the path and adding the new Btn in the next position.
    };
    //TODO: Enable \User Interface\CommandManager\PartContext\Tab{4,5,7,8,11} reading the "Tab Props" key and changing the third value to 1 (Example: "Tab Props"="23278,1,0,16" to "Tab Props"="23278,1,1,16")
    //User Interface\CommandManager\PartContext\Tab4]
    //"Tab Props"="23275,1,1,27"
    //            
    //User Interface\CommandManager\PartContext\Tab11]
    //"Tab Props"="58490,1,1,17"
    //            
    //User Interface\CommandManager\PartContext\Tab5]
    //"Tab Props"="23276,1,1,33"
    //            
    //User Interface\CommandManager\PartContext\Tab7]
    //"Tab Props"="23277,1,1,18"
    //            
    //User Interface\CommandManager\PartContext\Tab8]
    //"Tab Props"="23278,1,1,16"
};