// Tests de GPU: deteccion de marca a partir de vendor/renderer, seleccion de
// workarounds y armado del contenido del .reg.
//
// IMPORTANTE: el constructor de GPU llama a selecectBrandManual() (que a su vez
// llama a _getch()) cuando NO logra deducir la marca. Todo `GPU` construido en
// estos tests debe usar un vendor o renderer reconocible, o el runner quedaria
// bloqueado esperando una tecla. Para probar cadenas que no matchean se usa
// GPUTestAccess::buscarEnRenderMap, que nunca pide input.

#include "doctest.h"

#include <string>
#include <vector>

#include "GPU.h"
#include "test_access.h"

namespace {

GPU::Current current(const std::string& renderer, const std::string& vendor = "",
                     const std::string& workarounds = "", const std::string& origin = "") {
    GPU::Current c;
    c.renderer = renderer;
    c.vendor = vendor;
    c.workarounds = workarounds;
    c.origin = origin;
    return c;
}

// Cualquier renderer que la tabla reconozca; sirve para instanciar un GPU sin
// disparar el prompt manual cuando lo que se quiere probar es otra cosa.
GPU makeDetectableGpu() { return GPU(current("NVIDIA GeForce RTX 3060")); }

const std::string kAllowList = "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList";
const std::string kPorAnio =
    "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS 2022\\Performance\\Graphics\\Hardware";

} // namespace

TEST_SUITE("GPU::brandToString") {

TEST_CASE("cada marca tiene su etiqueta") {
    GPU gpu = makeDetectableGpu();
    CHECK(gpu.brandToString(GPU::Brand::NVIDIA) == "NVIDIA");
    CHECK(gpu.brandToString(GPU::Brand::AMD) == "AMD");
    CHECK(gpu.brandToString(GPU::Brand::INTEL) == "Intel");
    CHECK(gpu.brandToString(GPU::Brand::UNKNOWN) == "Unknown");
}

} // TEST_SUITE

TEST_SUITE("GPU deteccion de marca") {

TEST_CASE("el vendor tiene prioridad sobre el renderer") {
    // Vendor y renderer se contradicen a proposito: gana el vendor.
    GPU gpu(current("AMD Radeon RX 6800", "NVIDIA Corporation"));
    CHECK(GPUTestAccess::brand(gpu) == GPU::Brand::NVIDIA);
}

TEST_CASE("cae al renderer cuando el vendor esta vacio") {
    struct Caso {
        const char* renderer;
        GPU::Brand esperada;
    };

    const Caso casos[] = {
        {"NVIDIA GeForce RTX 3060", GPU::Brand::NVIDIA},
        {"Quadro K2200", GPU::Brand::NVIDIA},
        {"GeForce GTX 1050 Ti", GPU::Brand::NVIDIA},
        {"AMD Radeon RX 6800", GPU::Brand::AMD},
        {"Radeon Pro W5700", GPU::Brand::AMD},
        {"ATI FirePro V4900", GPU::Brand::AMD},
        {"Intel(R) UHD Graphics 630", GPU::Brand::INTEL},
        {"Intel(R) Iris(R) Xe Graphics", GPU::Brand::INTEL},
    };

    for (const Caso& caso : casos) {
        CAPTURE(caso.renderer);
        GPU gpu(current(caso.renderer));
        CHECK(GPUTestAccess::brand(gpu) == caso.esperada);
    }
}

TEST_CASE("los renderers con caracteres no ASCII se detectan igual") {
    // Los nombres reales traen (R), (TM) o sus simbolos. Pasar un char negativo a
    // std::toupper es comportamiento indefinido, asi que la conversion acota el
    // char a unsigned char antes de convertirlo.
    GPU registrada(current("Intel\xC2\xAE UHD Graphics 630"));
    GPU marca(current("NVIDIA\xE2\x84\xA2 GeForce RTX 3060"));

    CHECK(GPUTestAccess::brand(registrada) == GPU::Brand::INTEL);
    CHECK(GPUTestAccess::brand(marca) == GPU::Brand::NVIDIA);
}

TEST_CASE("la busqueda no distingue mayusculas") {
    GPU minusculas(current("nvidia geforce rtx 3060"));
    GPU mayusculas(current("NVIDIA GEFORCE RTX 3060"));
    CHECK(GPUTestAccess::brand(minusculas) == GPU::Brand::NVIDIA);
    CHECK(GPUTestAccess::brand(mayusculas) == GPU::Brand::NVIDIA);
}

TEST_CASE("una cadena sin coincidencias deja la marca intacta") {
    // Documenta que buscarEnRenderMap solo escribe `brand` cuando matchea: es la
    // razon por la que el constructor puede encadenar vendor y luego renderer.
    GPU gpu = makeDetectableGpu();
    REQUIRE(GPUTestAccess::brand(gpu) == GPU::Brand::NVIDIA);

    GPUTestAccess::buscarEnRenderMap(gpu, "Placa Generica Sin Marca");

    CHECK(GPUTestAccess::brand(gpu) == GPU::Brand::NVIDIA);
}

TEST_CASE("el renderer se conserva tal cual, sin normalizar") {
    const std::string renderer = "NVIDIA GeForce RTX 3060";
    GPU gpu(current(renderer));
    CHECK(GPUTestAccess::renderer(gpu) == renderer);
}

} // TEST_SUITE

TEST_SUITE("GPU workarounds") {

TEST_CASE("sin workarounds previos se usa la clave de la marca") {
    GPU gpu(current("NVIDIA GeForce RTX 3060"));
    CHECK(GPUTestAccess::brWorkarounds(gpu) == "4000000");
}

TEST_CASE("la clave base a evitar se reemplaza por la de la marca") {
    // 2501001 es brandBaseAvoid de NVIDIA: es la que rompe el rendimiento y la
    // que el programa existe para sustituir.
    GPU gpu(current("NVIDIA GeForce RTX 3060", "", "2501001"));
    CHECK(GPUTestAccess::brWorkarounds(gpu) == "4000000");
}

TEST_CASE("un workaround ya personalizado se respeta") {
    GPU gpu(current("NVIDIA GeForce RTX 3060", "", "12000001"));
    CHECK(GPUTestAccess::brWorkarounds(gpu) == "12000001");
}

TEST_CASE("cada marca aporta su propia clave por defecto") {
    GPU nvidia(current("NVIDIA GeForce RTX 3060"));
    GPU amd(current("AMD Radeon RX 6800"));
    GPU intel(current("Intel(R) UHD Graphics 630"));

    CHECK(GPUTestAccess::brWorkarounds(nvidia) == "4000000");
    CHECK(GPUTestAccess::brWorkarounds(amd) == "4000085");
    CHECK(GPUTestAccess::brWorkarounds(intel) == "4080080");
}

} // TEST_SUITE

TEST_SUITE("GPU::completarContenidoReg") {

TEST_CASE("con una ruta base emite las dos secciones esperadas") {
    GPU gpu(current("NVIDIA GeForce RTX 3060", "NVIDIA Corporation"));

    const std::vector<std::string> reg = gpu.completarContenidoReg({kAllowList});

    REQUIRE(reg.size() == 9);

    SUBCASE("cabecera de datos base") {
        CHECK(reg[0].find("; - **Renderer:** NVIDIA GeForce RTX 3060") != std::string::npos);
        CHECK(reg[0].find("; - **Vendor:** NVIDIA Corporation") != std::string::npos);
    }

    SUBCASE("seccion RealView Enabler bajo Gl2Shaders") {
        CHECK(reg[1].find(";## RealView Enabler: ##") != std::string::npos);
        CHECK(reg[2] == kAllowList + "\\Gl2Shaders\\NV40\\NVIDIA GeForce RTX 3060]");
        CHECK(reg[3] == "\"Workarounds\"=dword:32408");
    }

    SUBCASE("seccion Performance bajo la carpeta de la marca") {
        CHECK(reg[5].find(";## Performance ##") != std::string::npos);
        CHECK(reg[6] == kAllowList + "\\NVIDIA Corporation\\NVIDIA GeForce RTX 3060]");
        CHECK(reg[7] == "\"Workarounds\"=dword:4000000");
    }
}

TEST_CASE("cada ruta base agrega un par clave/valor por seccion") {
    GPU gpu(current("NVIDIA GeForce RTX 3060"));

    const std::vector<std::string> una = gpu.completarContenidoReg({kAllowList});
    const std::vector<std::string> dos = gpu.completarContenidoReg({kAllowList, kPorAnio});

    CHECK(una.size() == 9);
    CHECK(dos.size() == 13);
}

TEST_CASE("el modo generico escribe ambas rutas base") {
    GPU gpu(current("AMD Radeon RX 6800"));

    const std::vector<std::string> reg = gpu.completarContenidoReg({kAllowList, kPorAnio});

    REQUIRE(reg.size() == 13);
    CHECK(reg[2] == kAllowList + "\\Gl2Shaders\\R420\\AMD Radeon RX 6800]");
    CHECK(reg[4] == kPorAnio + "\\Gl2Shaders\\R420\\AMD Radeon RX 6800]");
    CHECK(reg[8] == kAllowList + "\\Advanced Micro Devices\\AMD Radeon RX 6800]");
    CHECK(reg[10] == kPorAnio + "\\Advanced Micro Devices\\AMD Radeon RX 6800]");
}

TEST_CASE("las rutas de Intel usan glPath Other") {
    GPU gpu(current("Intel(R) UHD Graphics 630"));

    const std::vector<std::string> reg = gpu.completarContenidoReg({kAllowList});

    CHECK(reg[2] == kAllowList + "\\Gl2Shaders\\Other\\Intel(R) UHD Graphics 630]");
    CHECK(reg[3] == "\"Workarounds\"=dword:30008");
    CHECK(reg[6] == kAllowList + "\\Intel\\Intel(R) UHD Graphics 630]");
    CHECK(reg[7] == "\"Workarounds\"=dword:4080080");
}

TEST_CASE("toda linea de clave cierra el corchete abierto por la ruta base") {
    GPU gpu(current("NVIDIA GeForce RTX 3060"));

    const std::vector<std::string> reg = gpu.completarContenidoReg({kAllowList, kPorAnio});

    for (const std::string& linea : reg) {
        if (linea.rfind('[', 0) == 0) {
            CAPTURE(linea);
            CHECK(linea.back() == ']');
        }
    }
}

TEST_CASE("los comentarios de alternativas quedan comentados con ';'") {
    // Todo lo que no sea una clave o un valor debe ir comentado, porque el
    // archivo se ejecuta tal cual en regedit.
    GPU gpu(current("NVIDIA GeForce RTX 3060"));

    const std::vector<std::string> reg = gpu.completarContenidoReg({kAllowList});

    CHECK(reg[4].find(";GL2Shaders Alternative Workarounds: ") != std::string::npos);
    CHECK(reg[8].find(";Brand Alternative Workarounds: ") != std::string::npos);
}

} // TEST_SUITE

TEST_SUITE("GPU datos base") {

TEST_CASE("solo se listan los campos presentes") {
    GPU completo(current("NVIDIA GeForce RTX 3060", "NVIDIA Corporation", "2501001",
                         "SolidWorks 2025"));
    const std::string texto = GPUTestAccess::baseDataToString(completo);

    CHECK(texto.find("; - **Renderer:** NVIDIA GeForce RTX 3060") != std::string::npos);
    CHECK(texto.find("; - **Vendor:** NVIDIA Corporation") != std::string::npos);
    CHECK(texto.find("; - **Workarounds:** 2501001") != std::string::npos);
    CHECK(texto.find("; - **Origin:** SolidWorks 2025") != std::string::npos);
}

TEST_CASE("los campos vacios se omiten en lugar de quedar en blanco") {
    GPU soloRenderer(current("NVIDIA GeForce RTX 3060"));
    const std::string texto = GPUTestAccess::baseDataToString(soloRenderer);

    CHECK(texto.find("; - **Renderer:**") != std::string::npos);
    CHECK(texto.find("; - **Vendor:**") == std::string::npos);
    CHECK(texto.find("; - **Workarounds:**") == std::string::npos);
    CHECK(texto.find("; - **Origin:**") == std::string::npos);
}

} // TEST_SUITE
