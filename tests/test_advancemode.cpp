// Tests de AdvanceMode: transformacion de "Tab Props", armado de rutas
// versionadas y lectura de valores originales del registro.
//
// Los tests de getOriginalValue crean y borran una clave propia bajo
// HKEY_CURRENT_USER\SOFTWARE\RealViewOnTests. No tocan las claves reales de
// SolidWorks y se limpian solas (ver ClaveDeRegistroTemporal).

#include "doctest.h"

#include <Windows.h>

#include <cstddef>
#include <string>
#include <vector>

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

TEST_CASE("QoL Commands va ultima y es la unica que necesita una instalacion") {
    // Lee claves de la instalacion concreta, por lo que las interfaces no la
    // ofrecen en modo generico.
    AdvanceMode am;
    const std::vector<AdvanceMode::Opcion> opciones = am.opciones();

    REQUIRE(opciones.size() == AdvanceModeTestAccess::regOptionsCount(am) + 1);
    CHECK(opciones.back().nombre == "QoL Commands");
    CHECK(opciones.back().soloInstalada);
    for (std::size_t i = 0; i + 1 < opciones.size(); i++) {
        CAPTURE(opciones[i].nombre);
        CHECK_FALSE(opciones[i].soloInstalada);
    }
}

TEST_CASE("las recomendadas son Performance, AntiAliasing y Spin Box") {
    AdvanceMode am;
    for (const AdvanceMode::Opcion& opcion : am.opciones()) {
        CAPTURE(opcion.nombre);
        const bool esperada = opcion.nombre == "Performance Enhance graphics" ||
                              opcion.nombre == "Full AntiAliasing" || opcion.nombre == "Spin Box Increment 1mm";
        CHECK(opcion.recomendada == esperada);
        // El nombre va limpio: cada interfaz decide como marcar las recomendadas.
        CHECK(opcion.nombre.find("Recommended") == std::string::npos);
    }
}

TEST_CASE("Spin Box es recomendada pero no viene marcada de entrada") {
    AdvanceMode am;
    for (const AdvanceMode::Opcion& opcion : am.opciones()) {
        CAPTURE(opcion.nombre);
        if (opcion.nombre == "Spin Box Increment 1mm") {
            CHECK_FALSE(opcion.marcadaPorDefecto);
        } else {
            CHECK(opcion.marcadaPorDefecto == opcion.recomendada);
        }
    }
}

} // TEST_SUITE

TEST_SUITE("AdvanceMode::armarOpciones") {

TEST_CASE("sin opciones elegidas lo avisa en el .reg") {
    AdvanceMode am;
    am.setSwVersion(2025, true);
    const std::string seccion = am.armarOpciones({});

    CHECK(seccion.find(";## Advance Mode: ##") != std::string::npos);
    CHECK(seccion.find("; - No Advance options selected.") != std::string::npos);
}

TEST_CASE("cada opcion elegida escribe sus claves bajo la version") {
    AdvanceMode am;
    am.setSwVersion(2025, true);
    const std::string seccion = am.armarOpciones({true});

    CHECK(seccion.find("; - Reverse Mouse Wheel") != std::string::npos);
    CHECK(seccion.find("[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS 2025\\General]\n"
                       "\"Reverse Zoom Direction\"=dword:00000001") != std::string::npos);
    CHECK(seccion.find("No Advance options selected") == std::string::npos);
}

TEST_CASE("en el .reg las recomendadas siguen diciendo (Recommended)") {
    AdvanceMode am;
    am.setSwVersion(2025, true);
    const std::string seccion = am.armarOpciones({false, false, false, true});

    CHECK(seccion.find("; - Performance Enhance graphics (Recommended)\n") != std::string::npos);
}

TEST_CASE("solo se escriben las opciones elegidas") {
    AdvanceMode am;
    am.setSwVersion(2025, true);
    const std::string seccion = am.armarOpciones({false, true});

    CHECK(seccion.find("; - Reverse Mouse Wheel") == std::string::npos);
    CHECK(seccion.find("; - DarkMode") != std::string::npos);
}

TEST_CASE("en modo generico QoL Commands se ignora aunque venga elegida") {
    AdvanceMode am;
    am.setSwVersion(2025, true);
    std::vector<bool> soloQol(am.opciones().size(), false);
    soloQol.back() = true;

    const std::string seccion = am.armarOpciones(soloQol);

    CHECK(seccion.find("QoL Commands") == std::string::npos);
    CHECK(seccion.find("; - No Advance options selected.") != std::string::npos);
}

TEST_CASE("armar dos veces da lo mismo") {
    // La GUI la llama para la vista previa y otra vez al generar: el aviso de
    // "ninguna opcion" no puede depender de una llamada anterior.
    AdvanceMode am;
    am.setSwVersion(2025, true);
    const std::string conOpcion = am.armarOpciones({true});
    const std::string sinOpcion = am.armarOpciones({});

    CHECK(sinOpcion.find("; - No Advance options selected.") != std::string::npos);
    CHECK(am.armarOpciones({true}) == conOpcion);
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
