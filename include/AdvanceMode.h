#pragma once
#include <string>
#include <vector>

// Opciones del modo avanzado. No pregunta nada: las interfaces muestran
// opciones() y pasan lo elegido a armarOpciones().
class AdvanceMode {
public:
    struct Opcion {
        std::string nombre;
        bool recomendada = false;
        bool marcadaPorDefecto = false; // La GUI la ofrece ya marcada.
        bool soloInstalada = false;     // No disponible en modo genérico.
    };

    AdvanceMode(); // Constructor
    void setSwVersion(int swVersion, bool generico);
    // Todas las opciones, en el orden que espera armarOpciones(). QoL Commands va
    // última y es la única que necesita una instalación concreta.
    std::vector<Opcion> opciones() const;
    // Sección "Advance Mode" del .reg. `elegidas` sigue el orden de opciones();
    // en modo genérico se ignoran las que tienen soloInstalada.
    std::string armarOpciones(const std::vector<bool>& elegidas);

private:
    friend struct AdvanceModeTestAccess; // Solo para tests unitarios (tests/test_access.h). No usar en producción.

    bool generico = false;
    const std::string rutaBase = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    std::string rutaVersionada = "";
    const std::string rutaTab = "User Interface\\CommandManager\\PartContext\\Tab";
    std::string completeBase (std::string complement);
    bool anySelected = false;
    std::vector <int> tabsToEnable = {4,5,7,8,11};

    std::string selectRegOptions(const std::vector<bool>& elegidas);
    static std::string nombreEnReg(const std::string& nombre, bool recomendada);
    std::string enableTabs();
    std::string enableTab(std::string value);
    std::string getOriginalValue(std::string tabPath, std::string valueName);
    std::string enableBtn();

    struct multiRegSetting {
        std::string path;
        std::vector<std::string> value;
    };

    struct RegSetting {
        std::string name;
        std::vector<multiRegSetting> content;
        bool recomendada = false;
        bool marcadaPorDefecto = false;
    };

    const std::vector<RegSetting> regOptions = {
        {"Reverse Mouse Wheel",
            {{"General",
                {"\"Reverse Zoom Direction\"=dword:00000001"}}}},
        {"DarkMode",
            {{"General",
                {"\"Color Theme ID\"=dword:00000003"}},
            {"Colors",
                {"\"Assembly, Edit Part\"=\"16776960\"",
                "\"Tree Item Normal\"=\"15790320\""}}}},
        {"FPS Viewer",
            {{"Performance",
                {"\"OpenGL Print Statistics\"=dword:00000001"}}}},
        {"Performance Enhance graphics",
            {{"Performance",
                {"\"Use Performance Pipeline 2020\"=dword:00000001",
                "\"Use GPU Silhouette Edges\"=dword:00000001"}}},
            true, true},
        {"Full AntiAliasing",
            {{"Edges",
                {"\"Show Anti Alias Shaded Edges\"=dword:00000000",
                "\"Show Fullscene Anti Alias\"=dword:00000001"}}},
            true, true},
        // Recomendada, pero cambia una preferencia personal: no va marcada de entrada.
        {"Spin Box Increment 1mm",
            {{"General",
                {"\"Length Increment (Metric)\"=\"0.001\""}}},
            true, false}
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