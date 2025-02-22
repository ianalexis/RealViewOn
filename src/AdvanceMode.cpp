#include "AdvanceMode.h"
#include "teclado.h"
#include <iostream>

AdvanceMode::AdvanceMode(int swVersion){
    setSwVersion(swVersion);
}

void AdvanceMode::setSwVersion(int swVersion){
    rutaVersionada = "\n" + std::string(rutaBase.begin(), rutaBase.end()) + std::to_string(swVersion) + "\\";
}

string AdvanceMode::completeBase (string complement){
    return rutaVersionada + complement;
}

string AdvanceMode::askAdvanceOptions(){
    string options = "";
    for (int i = 0; i < 6; i++){
        std::cout << "Do you want to enable " + regSettings[i].name + "? (Y/N): ";
        if (yesOrNo()){
            options += ";- " + regSettings[i].name + "\n;```" + completeBase(regSettings[i].content) + "\n;```\n\n";
        }
    }
    options.erase(options.size() - 2);
    return options;
}