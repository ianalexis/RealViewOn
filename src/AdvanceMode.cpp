#include "AdvanceMode.h"

AdvanceMode::AdvanceMode(int swVersion){
    baseVersion += std::to_wstring(swVersion) + "\\";
}

string completeBase (string complement){
    return baseVersion + complement;
}

string AdvanceMode::qolAdvanceMenu() {
    // Implementación pendiente
    return qolAdvanceMenuReg;
}

string AdvanceMode::reverseMouseWheel() {

}

string AdvanceMode::fullAntiAliasing() {

}

string AdvanceMode::forcePerformance() {

}

string AdvanceMode::darkMode() {

}

string AdvanceMode::fpsCounter() {

}
