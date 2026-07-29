// Tests de AdvanceMode: transformacion de "Tab Props", armado de rutas
// versionadas y lectura de valores originales del registro.
//
// Los tests de getOriginalValue crean y borran una clave propia bajo
// HKEY_CURRENT_USER\SOFTWARE\RealViewOnTests. No tocan las claves reales de
// SolidWorks y se limpian solas (ver ClaveDeRegistroTemporal).

#include "doctest.h"

#include <Windows.h>

#include <string>

#include "AdvanceMode.h"
#include "test_access.h"

namespace {

// Raiz de pruebas. Deliberadamente fuera de SOFTWARE\SolidWorks.
const char* const kRaizPruebas = "SOFTWARE\\RealViewOnTests";

// getOriginalValue recorta los primeros 20 caracteres del path para quedarse con
// la subclave, asumiendo el prefijo exacto "\n[HKEY_CURRENT_USER\" que arma
// setSwVersion, y descarta el "]" final. Estos tests reproducen ese formato para
// fijar ese contrato.
std::string comoPathDeReg(const std::string& subKey) {
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

    bool escribirDword(const std::string& nombre, DWORD valor) {
        return RegSetValueExA(hKey_, nombre.c_str(), 0, REG_DWORD,
                              reinterpret_cast<const BYTE*>(&valor),
                              sizeof(valor)) == ERROR_SUCCESS;
    }

private:
    HKEY hKey_ = nullptr;
    bool creada_ = false;
};

} // namespace

TEST_SUITE("AdvanceMode::enableTab") {

TEST_CASE("una pestana deshabilitada pasa su tercer campo a 1") {
    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::enableTab(am, "4,3,0") == "4,3,1");
    CHECK(AdvanceModeTestAccess::enableTab(am, "0,0,0") == "0,0,1");
}

TEST_CASE("los campos posteriores al tercero se preservan") {
    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::enableTab(am, "4,3,0,1,255") == "4,3,1,1,255");
}

TEST_CASE("una pestana ya habilitada devuelve vacio") {
    // Vacio significa "no hay nada que cambiar": enableTabs lo usa para no
    // escribir la clave en el .reg.
    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::enableTab(am, "4,3,1") == "");
    CHECK(AdvanceModeTestAccess::enableTab(am, "4,3,2") == "");
}

TEST_CASE("una entrada vacia devuelve vacio") {
    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::enableTab(am, "") == "");
}

TEST_CASE("menos de tres campos devuelve vacio") {
    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::enableTab(am, "4") == "");
    CHECK(AdvanceModeTestAccess::enableTab(am, "4,3") == "");
}

TEST_CASE("es idempotente: aplicar dos veces no vuelve a cambiar nada") {
    AdvanceMode am;
    const std::string primera = AdvanceModeTestAccess::enableTab(am, "4,3,0");
    REQUIRE(primera == "4,3,1");
    CHECK(AdvanceModeTestAccess::enableTab(am, primera) == "");
}

} // TEST_SUITE

TEST_SUITE("AdvanceMode rutas versionadas") {

TEST_CASE("completeBase arma la ruta de la version pedida") {
    AdvanceMode am;
    am.setSwVersion(2025, false);

    CHECK(AdvanceModeTestAccess::completeBase(am, "General") ==
          "\n[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS 2025\\General");
}

TEST_CASE("setSwVersion reemplaza la version en llamadas sucesivas") {
    AdvanceMode am;
    am.setSwVersion(2020, false);
    REQUIRE(AdvanceModeTestAccess::completeBase(am, "General").find("SOLIDWORKS 2020") !=
            std::string::npos);

    am.setSwVersion(2026, false);
    const std::string ruta = AdvanceModeTestAccess::completeBase(am, "General");
    CHECK(ruta.find("SOLIDWORKS 2026") != std::string::npos);
    CHECK(ruta.find("SOLIDWORKS 2020") == std::string::npos);
}

TEST_CASE("el modo generico no ofrece los comandos QoL") {
    // Las opciones QoL leen claves de la instalacion concreta, por lo que solo
    // se cuentan cuando NO es generico.
    AdvanceMode generico;
    generico.setSwVersion(2025, true);

    AdvanceMode instalado;
    instalado.setSwVersion(2025, false);

    const std::size_t base = AdvanceModeTestAccess::regOptionsCount(generico);
    CHECK(AdvanceModeTestAccess::optionsQty(generico) == static_cast<int>(base));
    CHECK(AdvanceModeTestAccess::optionsQty(instalado) == static_cast<int>(base) + 1);
}

TEST_CASE("la ruta versionada respeta el prefijo que espera getOriginalValue") {
    // getOriginalValue hace substr(20, size-21). Si este prefijo cambia, ese
    // recorte empieza a producir subclaves incorrectas en silencio.
    AdvanceMode am;
    am.setSwVersion(2025, false);

    const std::string ruta = AdvanceModeTestAccess::rutaVersionada(am);
    CHECK(ruta.rfind("\n[HKEY_CURRENT_USER\\", 0) == 0);
    CHECK(std::string("\n[HKEY_CURRENT_USER\\").size() == 20);
}

} // TEST_SUITE

TEST_SUITE("AdvanceMode::getOriginalValue") {

TEST_CASE("lee un REG_SZ existente") {
    const std::string subKey = std::string(kRaizPruebas) + "\\Sample";
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("Tab Props", "4,3,0,1"));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Tab Props") ==
          "4,3,0,1");
}

TEST_CASE("un valor inexistente devuelve vacio") {
    const std::string subKey = std::string(kRaizPruebas) + "\\Sample";
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "No Existe") == "");
}

TEST_CASE("una clave inexistente devuelve vacio") {
    AdvanceMode am;
    const std::string subKey = std::string(kRaizPruebas) + "\\NoCreada";
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Tab Props") == "");
}

TEST_CASE("un valor que no es REG_SZ se ignora") {
    const std::string subKey = std::string(kRaizPruebas) + "\\Sample";
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirDword("Numerico", 0x1234));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Numerico") == "");
}

TEST_CASE("integracion: leer del registro y habilitar la pestana") {
    const std::string subKey = std::string(kRaizPruebas) + "\\Tab4";
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("Tab Props", "4,3,0,1"));

    AdvanceMode am;
    const std::string original =
        AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Tab Props");
    REQUIRE(original == "4,3,0,1");
    CHECK(AdvanceModeTestAccess::enableTab(am, original) == "4,3,1,1");
}

} // TEST_SUITE
