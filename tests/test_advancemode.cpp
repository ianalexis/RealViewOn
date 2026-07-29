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
#include "registry_fixture.h"
#include "test_access.h"

using rvotest::ClaveDeRegistroTemporal;
using rvotest::comoPathDeReg;
using rvotest::subclaveDePrueba;

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
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("Tab Props", "4,3,0,1"));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Tab Props") ==
          "4,3,0,1");
}

TEST_CASE("un valor inexistente devuelve vacio") {
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "No Existe") == "");
}

TEST_CASE("una clave inexistente devuelve vacio") {
    AdvanceMode am;
    const std::string subKey = subclaveDePrueba("NoCreada");
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Tab Props") == "");
}

TEST_CASE("un REG_SZ de datos vacios devuelve vacio, no revienta") {
    // bufferSize == 0. El "bufferSize - 1" anterior daba un largo de 0xFFFFFFFF y
    // hacia fallar la construccion del string, abortando todo el modo avanzado.
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTextoCrudo("Vacio", "", 0));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Vacio") == "");
}

TEST_CASE("un REG_SZ sin terminador nulo no arrastra basura") {
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    // 7 bytes exactos, sin el '\0' final.
    REQUIRE(clave.escribirTextoCrudo("SinNulo", "4,3,0,1", 7));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "SinNulo") == "4,3,0,1");
}

TEST_CASE("un valor mas grande que el buffer devuelve vacio") {
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("Enorme", std::string(2048, 'x')));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Enorme") == "");
}

TEST_CASE("un path con formato inesperado devuelve vacio en lugar de lanzar") {
    // Antes se hacia substr(20, size - 21) sin validar: un path mas corto que el
    // prefijo lanzaba std::out_of_range.
    AdvanceMode am;
    const char* pathsInvalidos[] = {
        "",
        "]",
        "corto",
        "[HKEY_CURRENT_USER\\SOFTWARE\\Test]",   // le falta el "\n" inicial
        "\n[HKEY_LOCAL_MACHINE\\SOFTWARE\\Test]", // otra raiz
        "\n[HKEY_CURRENT_USER\\SOFTWARE\\Test",   // sin "]" final
        "\n[HKEY_CURRENT_USER\\]",                // subclave vacia
    };

    for (const char* path : pathsInvalidos) {
        CAPTURE(path);
        CHECK_NOTHROW(AdvanceModeTestAccess::getOriginalValue(am, path, "Tab Props"));
        CHECK(AdvanceModeTestAccess::getOriginalValue(am, path, "Tab Props") == "");
    }
}

TEST_CASE("un valor que no es REG_SZ se ignora") {
    const std::string subKey = subclaveDePrueba("Sample");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirDword("Numerico", 0x1234));

    AdvanceMode am;
    CHECK(AdvanceModeTestAccess::getOriginalValue(am, comoPathDeReg(subKey), "Numerico") == "");
}

TEST_CASE("integracion: leer del registro y habilitar la pestana") {
    const std::string subKey = subclaveDePrueba("Tab4");
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
