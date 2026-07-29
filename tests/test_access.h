#ifndef RVO_TEST_ACCESS_H
#define RVO_TEST_ACCESS_H

// Puentes de acceso para tests unitarios.
//
// Varias funciones puras de RealViewOn son privadas, y exponerlas publicamente
// solo para los tests ensancharia la API de las clases. En su lugar cada clase
// declara `friend struct <Clase>TestAccess;` y este archivo implementa esos
// structs. Solo se compila dentro del proyecto de tests: el binario de
// produccion nunca lo incluye y las declaraciones friend no generan codigo.
//
// Regla: nada de lo que se exponga aca debe llamar a entradaTeclado()/yesOrNo(),
// porque _getch() bloquearia la ejecucion en CI. Ver tests/README.md.

#include <string>
#include <vector>

#include "AdvanceMode.h"
#include "GPU.h"
#include "SolidWorks.h"

struct GPUTestAccess {
    static GPU::Brand brand(const GPU& gpu) { return gpu.brand; }
    static std::string renderer(const GPU& gpu) { return gpu.renderer; }
    static std::string brWorkarounds(const GPU& gpu) { return gpu.brWorkarounds; }
    static std::string baseDataToString(GPU& gpu) { return gpu.baseDataToString(); }

    // No prompt: buscarEnRenderMap solo escribe `brand` cuando encuentra una
    // coincidencia. Es la via segura para probar strings que NO matchean, ya que
    // el constructor de GPU llamaria a selecectBrandManual() y bloquearia.
    static void buscarEnRenderMap(GPU& gpu, const std::string& buscado) {
        gpu.buscarEnRenderMap(buscado);
    }
};

struct AdvanceModeTestAccess {
    static std::string enableTab(AdvanceMode& am, const std::string& value) {
        return am.enableTab(value);
    }
    static std::string completeBase(AdvanceMode& am, const std::string& complement) {
        return am.completeBase(complement);
    }
    static std::string getOriginalValue(AdvanceMode& am, const std::string& path,
                                       const std::string& valueName) {
        return am.getOriginalValue(path, valueName);
    }
    static std::string rutaVersionada(const AdvanceMode& am) { return am.rutaVersionada; }
    static int optionsQty(const AdvanceMode& am) { return am.optionsQty; }
    static std::size_t regOptionsCount(const AdvanceMode& am) { return am.regOptions.size(); }
};

struct SolidWorksTestAccess {
    // setVersion() publico consulta yesOrNo() cuando la version no esta
    // instalada, lo que bloquearia en CI. Los tests fijan el campo directamente.
    static void setSwVersion(SolidWorks& sw, int v) { sw.swVersion = v; }
    static int swVersion(const SolidWorks& sw) { return sw.swVersion; }
    static int anoActual(const SolidWorks& sw) { return sw.anoActual; }
    static int vMin() { return SolidWorks::vMin; }
    static int vCambioRaiz() { return SolidWorks::vCambioRaiz; }
    static int vMax() { return SolidWorks::vMax; }
    static std::string obtenerRegBaseAno(SolidWorks& sw) { return sw.obtenerRegBaseAno(); }
    static std::string obtenerRegBaseAllowList(SolidWorks& sw) {
        return sw.obtenerRegBaseAllowList();
    }
    // Sobrecarga privada que lee una clave arbitraria. No pide input (a diferencia
    // de obtenerCurrent() sin argumentos, que confirma el renderer con yesOrNo()).
    static GPU::Current obtenerCurrent(SolidWorks& sw, const std::string& path) {
        return sw.obtenerCurrent(path);
    }
};

#endif // RVO_TEST_ACCESS_H
