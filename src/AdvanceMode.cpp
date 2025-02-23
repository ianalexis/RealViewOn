#include "AdvanceMode.h"
#include "teclado.h"
#include <iostream>

AdvanceMode::AdvanceMode(){
}

void AdvanceMode::setSwVersion(int swVersion){
    rutaVersionada = "\n" + std::string(rutaBase.begin(), rutaBase.end()) + std::to_string(swVersion) + "\\";
}

string AdvanceMode::completeBase (string complement){
    return rutaVersionada + complement;
}

string AdvanceMode::askAdvanceOptions(){
    string options = "\n\n;## Advance Mode: ##";
    bool AnySelected = false;
    for (int i = 0 ; i < regSettings.size() ; i++){
        std::cout << "Enable " + regSettings[i].name + "? (Y/N): ";
        if (yesOrNo()){
            options += "\n;- " + regSettings[i].name + "\n;```";
            options += completeBase(regSettings[i].content);//TODO: Hacer algun tipo de for que por cada multiRegSetting.path complete con sus values
            options += "\n;```\n";
            AnySelected = true;
        }
    }
    if (!AnySelected){
        options += "\n; - No Advance options selected.";
    }
    return options;
}