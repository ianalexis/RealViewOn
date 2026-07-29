#ifndef RVO_REGISTRY_FIXTURE_H
#define RVO_REGISTRY_FIXTURE_H

// Utilidades para los tests que necesitan leer valores reales del registro.
//
// Todo se crea bajo HKEY_CURRENT_USER\SOFTWARE\RealViewOnTests y se borra al
// destruirse el fixture. Deliberadamente NO se usa SOFTWARE\SolidWorks, para que
// correr la suite en una maquina con SolidWorks instalado sea inocuo.

#include <Windows.h>

#include <string>

namespace rvotest {

inline const char* const kRaizPruebas = "SOFTWARE\\RealViewOnTests";

inline std::string subclaveDePrueba(const std::string& nombre) {
    return std::string(kRaizPruebas) + "\\" + nombre;
}

// getOriginalValue() espera el formato que produce completeBase() + "]", o sea
// "\n[HKEY_CURRENT_USER\<subclave>]".
inline std::string comoPathDeReg(const std::string& subKey) {
    return "\n[HKEY_CURRENT_USER\\" + subKey + "]";
}

class ClaveDeRegistroTemporal {
public:
    explicit ClaveDeRegistroTemporal(const std::string& subKey) {
        creada_ = RegCreateKeyExA(HKEY_CURRENT_USER, subKey.c_str(), 0, nullptr,
                                  REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, &hKey_,
                                  nullptr) == ERROR_SUCCESS;
    }

    ~ClaveDeRegistroTemporal() {
        if (hKey_ != nullptr) {
            RegCloseKey(hKey_);
        }
        // Borra el arbol completo de pruebas, no solo esta subclave, para no dejar
        // la raiz vacia colgada.
        RegDeleteTreeA(HKEY_CURRENT_USER, kRaizPruebas);
        RegDeleteKeyA(HKEY_CURRENT_USER, kRaizPruebas);
    }

    ClaveDeRegistroTemporal(const ClaveDeRegistroTemporal&) = delete;
    ClaveDeRegistroTemporal& operator=(const ClaveDeRegistroTemporal&) = delete;

    bool creada() const { return creada_; }

    bool escribirTexto(const std::string& nombre, const std::string& valor) {
        return RegSetValueExA(hKey_, nombre.c_str(), 0, REG_SZ,
                              reinterpret_cast<const BYTE*>(valor.c_str()),
                              static_cast<DWORD>(valor.size() + 1)) == ERROR_SUCCESS;
    }

    // Permite escribir un REG_SZ con una cantidad arbitraria de bytes, incluido 0.
    // escribirTexto() siempre suma el terminador nulo, asi que no puede expresar el
    // caso de datos vacios.
    bool escribirTextoCrudo(const std::string& nombre, const void* datos, DWORD bytes) {
        return RegSetValueExA(hKey_, nombre.c_str(), 0, REG_SZ, static_cast<const BYTE*>(datos),
                              bytes) == ERROR_SUCCESS;
    }

    bool escribirDword(const std::string& nombre, DWORD valor) {
        return RegSetValueExA(hKey_, nombre.c_str(), 0, REG_DWORD,
                              reinterpret_cast<const BYTE*>(&valor),
                              sizeof(valor)) == ERROR_SUCCESS;
    }

private:
    HKEY hKey_ = nullptr;
    bool creada_ = false;
};

} // namespace rvotest

#endif // RVO_REGISTRY_FIXTURE_H
