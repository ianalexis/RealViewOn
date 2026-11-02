#include "SolidWorks.h"
#include <Windows.h>
#include <string.h>
#include <string>
#include <cstring>
#include <functional>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "GPU.h"


using std::string;

// Constructor
SolidWorks::SolidWorks() {
    anoActual = obtenerAnoActual() < vMax ? vMax : obtenerAnoActual();
}

// Obtiene el año actual del sistema operativo + 1.
int SolidWorks::obtenerAnoActual() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    return (st.wYear + 1);
}

// Configura la versión de SolidWorks.
void SolidWorks::setVersion(int v, bool generico) {
    if (esCompatible(v) == 0) {
        throw std::runtime_error("The version of SolidWorks is not compatible.");
    }
    swVersion = v;
    setGenerico(generico);
}

bool SolidWorks::requiereGenerico(int v) {
    return !versionInstalada(v) || esCompatible(v) == 2;
}

bool SolidWorks::esDelFuturo(int v) {
    return v > anoActual;
}

// Comprueba si una versión es compatible. 0 = No compatible, 1 = Compatible , 2 = Versión futura.
int SolidWorks::esCompatible(int v) {
    return (v >= vMin) ? (v <= vMax) ? 1 : 2 : 0;
}

// Setea como generico el modo de instalación
void SolidWorks::setGenerico(bool g) {
    generico = g;
}

// Recorre las versiones de SolidWorks instaladas y devuelve un listado con el año de la versión y si es compatible.
const std::vector<std::pair<int, int>>& SolidWorks::obtenerVersionesInstaladas() {
    versiones.clear();
    for (int i = vMin; i <= anoActual; i++) {
        string versionKey = swRegRuta + std::to_string(i);
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            versiones.push_back(std::make_pair(i, esCompatible(i)));
            RegCloseKey(hKey);
        }
    }
    return versiones;
}

GPU::Current SolidWorks::detectarCurrent() {
    GPU::Current current;
    if (swVersion < vCambioRaiz || generico) {
        current = obtenerCurrentAno();
        // El origin se asigna sobre el resultado: obtenerCurrent(path) devuelve un
        // Current nuevo, así que fijarlo antes de esta línea se perdía y el .reg
        // salía sin la línea "Origin" para las versiones previas a vCambioRaiz.
        if (!current.renderer.empty()) {
            current.origin = "SolidWorks " + std::to_string(swVersion);
        }
    }
    if (current.renderer.empty() || swVersion >= vCambioRaiz || generico) {
        GPU::Current tempCurrent = obtenerCurrentRaiz();
        if (!tempCurrent.renderer.empty()) {
            current = tempCurrent;
            current.origin = "SolidWorks Root";
        }
    }
    return current;
}

std::vector<std::pair<std::string, std::string>> SolidWorks::candidatosRenderer() {
    std::vector<std::pair<std::string, std::string>> candidatos;
    auto agregar = [&candidatos](const std::pair<std::string, std::string>& candidato) {
        for (const auto& existente : candidatos) {
            if (existente.first == candidato.first) {
                return;
            }
        }
        candidatos.push_back(candidato);
    };
    for (const auto& renderer : obtenerRendererGenerico()) {
        agregar(renderer);
    }
    for (const auto& adaptador : windowsDisplayAdapters()) {
        agregar(adaptador);
    }
    return candidatos;
}

std::vector<std::pair<std::string, std::string>> SolidWorks::windowsDisplayAdapters() {
    std::vector<std::pair<std::string, std::string>> adaptadores;
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);
    int deviceIndex = 0;

    while (EnumDisplayDevicesA(NULL, deviceIndex, &dd, 0)) {
        std::string deviceString(dd.DeviceString);

        bool found = false;
        for (auto& adaptador : adaptadores) {
            if (adaptador.first == deviceString) {
                adaptador.second += " ," + std::to_string(deviceIndex);
                found = true;
                break;
            }
        }

        if (!found && !deviceString.empty()) {
            adaptadores.push_back(std::make_pair(deviceString, "Windows Display Adapter " + std::to_string(deviceIndex)));
        }

        deviceIndex++;
    }
    return adaptadores;
}

// Lee un valor del registro en el buffer indicado.
// bytesLeidos SIEMPRE se reinicia a la capacidad del buffer antes de consultar:
// RegQueryValueExA escribe ahi la cantidad copiada (y, ante ERROR_MORE_DATA, el
// tamaño requerido, que puede superar la capacidad real). Reutilizar el valor
// entre lecturas hacía que la segunda consulta recibiera la capacidad sobrante de
// la primera, salteando valores en silencio o declarando un buffer mayor que el
// real.
static bool leerValorDeRegistro(HKEY hKey, const char* valueName, char* buffer, DWORD capacidad,
                                DWORD& bytesLeidos, DWORD& tipo) {
    bytesLeidos = capacidad;
    tipo = 0;
    if (RegQueryValueExA(hKey, valueName, NULL, &tipo, reinterpret_cast<LPBYTE>(buffer),
                         &bytesLeidos) != ERROR_SUCCESS) {
        return false;
    }
    // El valor no entraba en el buffer: se descarta en lugar de leer de más.
    return bytesLeidos <= capacidad;
}

// Los valores REG_SZ del registro no tienen garantizado el terminador nulo, así
// que se acota por strnlen en lugar de asumir bytesLeidos - 1.
static string comoTexto(const char* buffer, DWORD bytes) {
    return string(buffer, strnlen(buffer, bytes));
}

GPU::Current SolidWorks::obtenerCurrent(string path){
    HKEY hKey;
    GPU::Current currentTemp;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return currentTemp;
    }

    char value[256];
    DWORD bytesLeidos = 0;
    DWORD valueType = 0;

    // Obtener renderer
    if (leerValorDeRegistro(hKey, "renderer", value, sizeof(value), bytesLeidos, valueType)) {
        currentTemp.renderer = comoTexto(value, bytesLeidos);
    }
    if (currentTemp.renderer.empty()) {
        RegCloseKey(hKey);
        return currentTemp;
    }
    // Obtener vendor
    if (leerValorDeRegistro(hKey, "vendor", value, sizeof(value), bytesLeidos, valueType)) {
        currentTemp.vendor = comoTexto(value, bytesLeidos);
    }
    // Obtener workarounds
    if (leerValorDeRegistro(hKey, "workarounds", value, sizeof(value), bytesLeidos, valueType)) {
        if (valueType == REG_DWORD) {
            if (bytesLeidos >= sizeof(DWORD)) {
                DWORD workaroundsValue = 0;
                // memcpy en lugar de *(DWORD*)value: el buffer es char[] y no
                // tiene garantizada la alineación de un DWORD.
                std::memcpy(&workaroundsValue, value, sizeof(workaroundsValue));
                std::stringstream ss;
                ss << std::hex << workaroundsValue;
                currentTemp.workarounds = ss.str();
            }
        } else {
            currentTemp.workarounds = comoTexto(value, bytesLeidos);
        }
    }
    RegCloseKey(hKey);
    return currentTemp;
}

// Trae Current en carpeta raiz.
GPU::Current SolidWorks::obtenerCurrentRaiz() {
    string regPath = "SOFTWARE\\SolidWorks\\AllowList\\Current";
    return obtenerCurrent(regPath);
}

// Trae Current en carpeta de version.
GPU::Current SolidWorks::obtenerCurrentAno() {
    string regPath = swRegRuta + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware\\Current";
    return obtenerCurrent(regPath);
}

// Busca render en todo el registro (modo generico)
std::vector<std::pair<std::string, std::string>> SolidWorks::obtenerRendererGenerico() {
    HKEY hKey;
    std::vector<std::pair<std::string, std::string>> renderers;
    const string basePath = "SOFTWARE\\SolidWorks";

    // Función recursiva para buscar claves Renderer
    std::function<void(HKEY, const string&)> buscarRendererEnSubclaves = [&](HKEY hKey, const string& subKeyPath) {
        char subKeyName[256];
        DWORD subKeyNameSize;
        DWORD index = 0;

        while (true) {
            subKeyNameSize = sizeof(subKeyName);
            if (RegEnumKeyExA(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
                break;
            }

            HKEY hSubKey;
            string fullSubKeyPath = subKeyPath + "\\" + subKeyName;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, fullSubKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                DWORD valueType;
                char valueData[256];
                DWORD valueDataSize = sizeof(valueData);

                if (RegQueryValueExA(hSubKey, "Renderer", NULL, &valueType, (LPBYTE)valueData, &valueDataSize) == ERROR_SUCCESS) {
                    if (valueType == REG_SZ) {
                        string rendererValue(valueData, valueDataSize);
                        rendererValue.resize(strnlen(valueData, valueDataSize));
                        renderers.push_back(std::make_pair(rendererValue, fullSubKeyPath));
                    }
                }

                // Llamada recursiva para buscar en subclaves
                buscarRendererEnSubclaves(hSubKey, fullSubKeyPath);
                RegCloseKey(hSubKey);
            }
            index++;
        }
    };

    if (RegOpenKeyExA(HKEY_CURRENT_USER, basePath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        buscarRendererEnSubclaves(hKey, basePath);
        RegCloseKey(hKey);
    }
    return renderers;
}

// Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
std::vector<std::string> SolidWorks::obtenerRegBase() {
    regBase.clear();
    if (swVersion >= vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAllowList());
    }
    if (swVersion < vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAno());
    }
    return regBase;
}

string SolidWorks::obtenerRegBaseAllowList() {
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList";
}

string SolidWorks::obtenerRegBaseAno() {
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware";
}

// Valida que la version esté en el listado de versiones instaladas
bool SolidWorks::versionInstalada(int v) {
    for (const auto& version : versiones) {
        if (version.first == v) {
            return true;
        }
    }
    return false;
}

bool SolidWorks::getGenerico() {
    return generico;
}