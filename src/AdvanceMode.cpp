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
    for (int i = 0 ; i < advanceRegOptions.size() ; i++){
        std::cout << "Enable " + advanceRegOptions[i].name + "? (Y/N): ";
        if (yesOrNo()){
            options += "\n;- " + advanceRegOptions[i].name + "\n;```";
            for (int j = 0 ; j < advanceRegOptions[i].content.size() ; j++){
                options += completeBase(advanceRegOptions[i].content[j].path) + "]";
                for (int k = 0 ; k < advanceRegOptions[i].content[j].value.size() ; k++){
                    options += "\n" + advanceRegOptions[i].content[j].value[k];
                }
            }
            options += "\n;```\n";
            AnySelected = true;
        }
    }
    if (!AnySelected){
        options += "\n; - No Advance options selected.";
    }
    return options;
}