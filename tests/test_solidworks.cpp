// Tests de SolidWorks: clasificacion de compatibilidad de versiones y
// construccion de las rutas base del registro (incluyendo el cambio de
// ubicacion introducido en 2023).

#include "doctest.h"

#include <string>
#include <vector>

#include "SolidWorks.h"
#include "registry_fixture.h"
#include "test_access.h"

using rvotest::ClaveDeRegistroTemporal;
using rvotest::comoPathDeReg;
using rvotest::subclaveDePrueba;

TEST_SUITE("SolidWorks::esCompatible") {

TEST_CASE("clasifica las versiones en no-compatible / compatible / futura") {
    SolidWorks sw;
    const int vMin = SolidWorksTestAccess::vMin();
    const int vMax = SolidWorksTestAccess::vMax();

    SUBCASE("por debajo de vMin es no compatible (0)") {
        CHECK(sw.esCompatible(vMin - 1) == 0);
        CHECK(sw.esCompatible(1999) == 0);
        CHECK(sw.esCompatible(0) == 0);
    }

    SUBCASE("los limites del rango soportado son compatibles (1)") {
        CHECK(sw.esCompatible(vMin) == 1);
        CHECK(sw.esCompatible(vMax) == 1);
    }

    SUBCASE("dentro del rango es compatible (1)") {
        CHECK(sw.esCompatible(2015) == 1);
        CHECK(sw.esCompatible(SolidWorksTestAccess::vCambioRaiz()) == 1);
    }

    SUBCASE("por encima de vMax es version futura (2)") {
        CHECK(sw.esCompatible(vMax + 1) == 2);
        CHECK(sw.esCompatible(9999) == 2);
    }
}

TEST_CASE("el rango soportado es coherente") {
    CHECK(SolidWorksTestAccess::vMin() < SolidWorksTestAccess::vCambioRaiz());
    CHECK(SolidWorksTestAccess::vCambioRaiz() <= SolidWorksTestAccess::vMax());
}

} // TEST_SUITE

TEST_SUITE("SolidWorks estado inicial") {

TEST_CASE("el constructor nunca deja anoActual por debajo de vMax") {
    // anoActual = max(anio del sistema + 1, vMax). Se comprueba la invariante en
    // lugar de un anio concreto para que el test no dependa del reloj.
    SolidWorks sw;
    CHECK(SolidWorksTestAccess::anoActual(sw) >= SolidWorksTestAccess::vMax());
}

TEST_CASE("arranca en modo no generico") {
    SolidWorks sw;
    CHECK(sw.getGenerico() == false);
}

TEST_CASE("setGenerico alterna la bandera") {
    SolidWorks sw;
    sw.setGenerico(true);
    CHECK(sw.getGenerico() == true);
    sw.setGenerico(false);
    CHECK(sw.getGenerico() == false);
}

TEST_CASE("la cadena de versiones sugeridas lista min, cambio de raiz y max") {
    SolidWorks sw;
    const std::string esperado = std::to_string(SolidWorksTestAccess::vMin()) + ", " +
                                 std::to_string(SolidWorksTestAccess::vCambioRaiz()) + ", " +
                                 std::to_string(SolidWorksTestAccess::vMax());
    CHECK(sw.versions == esperado);
}

} // TEST_SUITE

TEST_SUITE("SolidWorks::obtenerRegBase") {

TEST_CASE("las versiones previas a 2023 usan solo la ruta por anio") {
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, 2022);

    const std::vector<std::string> regBase = sw.obtenerRegBase();

    REQUIRE(regBase.size() == 1);
    CHECK(regBase[0] ==
          "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS 2022\\Performance\\Graphics\\Hardware");
}

TEST_CASE("desde 2023 se usa solo la ruta AllowList") {
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, SolidWorksTestAccess::vCambioRaiz());

    const std::vector<std::string> regBase = sw.obtenerRegBase();

    REQUIRE(regBase.size() == 1);
    CHECK(regBase[0] == "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList");
}

TEST_CASE("el modo generico emite ambas rutas, AllowList primero") {
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, 2022);
    sw.setGenerico(true);

    const std::vector<std::string> regBase = sw.obtenerRegBase();

    REQUIRE(regBase.size() == 2);
    CHECK(regBase[0] == "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList");
    CHECK(regBase[1] ==
          "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS 2022\\Performance\\Graphics\\Hardware");
}

TEST_CASE("llamadas repetidas no acumulan entradas") {
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, 2022);

    const std::size_t primera = sw.obtenerRegBase().size();
    const std::size_t segunda = sw.obtenerRegBase().size();

    CHECK(primera == segunda);
}

TEST_CASE("la ruta por anio interpola el anio recibido") {
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, 2019);
    CHECK(SolidWorksTestAccess::obtenerRegBaseAno(sw).find("SOLIDWORKS 2019") != std::string::npos);
}

TEST_CASE("las rutas base abren corchete y no lo cierran") {
    // GPU::completar* concatena subclaves y agrega el "]" final, por lo que la
    // ruta base debe quedar deliberadamente sin cerrar.
    SolidWorks sw;
    SolidWorksTestAccess::setSwVersion(sw, 2022);

    for (const std::string& base : {SolidWorksTestAccess::obtenerRegBaseAllowList(sw),
                                    SolidWorksTestAccess::obtenerRegBaseAno(sw)}) {
        CHECK(base.front() == '[');
        CHECK(base.find(']') == std::string::npos);
    }
}

} // TEST_SUITE

TEST_SUITE("SolidWorks::obtenerCurrent(path)") {

// Los tres valores (renderer, vendor, workarounds) se leen del mismo buffer.
// El bug que estos tests fijan: bufferSize se inicializaba una sola vez, asi que
// cada lectura recibia como capacidad la cantidad de bytes que habia devuelto la
// lectura anterior.

TEST_CASE("lee los tres valores aunque el vendor sea mas largo que el renderer") {
    // Con un renderer corto seguido de un vendor largo, la version anterior pasaba
    // una capacidad de 3 bytes a la consulta de vendor y lo descartaba en silencio.
    const std::string subKey = subclaveDePrueba("Current");
    const std::string vendorLargo(200, 'V');
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("renderer", "NV"));
    REQUIRE(clave.escribirTexto("vendor", vendorLargo));
    REQUIRE(clave.escribirDword("workarounds", 0x52400c84));

    SolidWorks sw;
    const GPU::Current current = SolidWorksTestAccess::obtenerCurrent(sw, subKey);

    CHECK(current.renderer == "NV");
    CHECK(current.vendor == vendorLargo);
    CHECK(current.workarounds == "52400c84");
}

TEST_CASE("un valor que no entra en el buffer se descarta sin afectar a los demas") {
    // Un vendor de mas de 256 bytes no cabe. Antes, ERROR_MORE_DATA dejaba en
    // bufferSize el tamano requerido (> capacidad real), y la consulta siguiente
    // se hacia declarando ese tamano sobre el buffer de 256.
    const std::string subKey = subclaveDePrueba("Current");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("renderer", "NV"));
    REQUIRE(clave.escribirTexto("vendor", std::string(300, 'V')));
    REQUIRE(clave.escribirDword("workarounds", 0x4000000));

    SolidWorks sw;
    const GPU::Current current = SolidWorksTestAccess::obtenerCurrent(sw, subKey);

    CHECK(current.renderer == "NV");
    CHECK(current.vendor == "");            // no entraba: se descarta
    CHECK(current.workarounds == "4000000"); // y la lectura siguiente sigue siendo correcta
}

TEST_CASE("el workarounds REG_DWORD se formatea en hexadecimal minuscula") {
    // GPU::setBrWorkarounds compara este string contra brandBaseAvoid, que esta
    // escrito en minuscula ("52400c84"). Si el formato cambiara a mayuscula, la
    // sustitucion de la clave problematica dejaria de detectarse.
    const std::string subKey = subclaveDePrueba("Current");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("renderer", "AMD Radeon"));
    REQUIRE(clave.escribirDword("workarounds", 0x52400C84));

    SolidWorks sw;
    const GPU::Current current = SolidWorksTestAccess::obtenerCurrent(sw, subKey);

    CHECK(current.workarounds == "52400c84");
}

TEST_CASE("un workarounds de texto se lee tal cual") {
    const std::string subKey = subclaveDePrueba("Current");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("renderer", "NV"));
    REQUIRE(clave.escribirTexto("workarounds", "12000001"));

    SolidWorks sw;
    CHECK(SolidWorksTestAccess::obtenerCurrent(sw, subKey).workarounds == "12000001");
}

TEST_CASE("sin renderer devuelve un Current vacio y no mira los demas valores") {
    const std::string subKey = subclaveDePrueba("Current");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("vendor", "NVIDIA Corporation"));

    SolidWorks sw;
    const GPU::Current current = SolidWorksTestAccess::obtenerCurrent(sw, subKey);

    CHECK(current.renderer == "");
    CHECK(current.vendor == "");
}

TEST_CASE("una clave inexistente devuelve un Current vacio") {
    SolidWorks sw;
    const GPU::Current current =
        SolidWorksTestAccess::obtenerCurrent(sw, subclaveDePrueba("NoCreada"));

    CHECK(current.renderer == "");
    CHECK(current.vendor == "");
    CHECK(current.workarounds == "");
}

TEST_CASE("no asigna origin: eso queda a cargo de quien llama") {
    const std::string subKey = subclaveDePrueba("Current");
    ClaveDeRegistroTemporal clave(subKey);
    REQUIRE(clave.creada());
    REQUIRE(clave.escribirTexto("renderer", "NV"));

    SolidWorks sw;
    CHECK(SolidWorksTestAccess::obtenerCurrent(sw, subKey).origin == "");
}

} // TEST_SUITE
