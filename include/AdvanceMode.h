#pragma once
#include <string>

using std::string;


class AdvanceMode {
public:
    AdvanceMode(int swVersion); // Constructor

    string qolAdvanceMenu();
    string reverseMouseWheel();
    string fullAntiAliasing();
    string forcePerformance();
    string darkMode();
    string fpsCounter();

private:
    string baseVersion = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    string completeBase (string complement);
    
    string qolAdvanceMenuReg = "TBD";//TODO: Definir
    string reverseMouseWheelReg = "\\General\n\"Reverse Zoom Direction\"=dword:00000001";
    string fullAntiAliasingReg = "\\Edges\n\"Show Anti Alias Shaded Edges\"=dword:00000000\n\"Show Fullscene Anti Alias\"=dword:00000001";
    string forcePerformanceReg = "\\Performance]\n\"Use Performance Pipeline 2020\"=dword:00000001\n\"Use GPU Silhouette Edges\"=dword:00000001";
    string darkModeReg = "\\General]\n\"Color Theme ID\"=dword:00000003\\n;Light: 1, Medium: 2, Dark: 3, Medium Light: 4.";
    string fpsCounterReg = "\\Performance]\n\"OpenGL Print Statistics\"=dword:00000001";

};

