// Tests de guardarArchivoReg: el archivo .reg es el unico entregable del
// programa, asi que su cabecera y su formato son contrato.
//
// Cada test corre dentro de un directorio temporal propio (guardarArchivoReg
// escribe siempre en el directorio actual) y lo borra al terminar.
//
// guardarBackUp() no se cubre aca: invoca `reg export` via system() sobre la
// instalacion real de SolidWorks.

#include "doctest.h"

#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "Registro.h"

namespace {

const std::string kVersionRvo = "9.9.9.9";

class DirectorioTemporal {
public:
    DirectorioTemporal() {
        static int contador = 0;
        anterior_ = std::filesystem::current_path();
        ruta_ = std::filesystem::temp_directory_path() /
                ("rvo_tests_" + std::to_string(::GetCurrentProcessId()) + "_" +
                 std::to_string(++contador));
        std::filesystem::create_directories(ruta_);
        std::filesystem::current_path(ruta_);
    }

    ~DirectorioTemporal() {
        std::error_code ec;
        std::filesystem::current_path(anterior_, ec);
        std::filesystem::remove_all(ruta_, ec);
    }

    DirectorioTemporal(const DirectorioTemporal&) = delete;
    DirectorioTemporal& operator=(const DirectorioTemporal&) = delete;

    const std::filesystem::path& ruta() const { return ruta_; }

private:
    std::filesystem::path anterior_;
    std::filesystem::path ruta_;
};

// Lee en binario y normaliza CRLF a LF: ofstream en modo texto traduce los "\n"
// que escribe guardarArchivoReg, y aca interesa el contenido logico.
std::string leerNormalizado(const std::filesystem::path& archivo) {
    std::ifstream in(archivo, std::ios::binary);
    std::string contenido((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
    std::string salida;
    salida.reserve(contenido.size());
    for (std::size_t i = 0; i < contenido.size(); ++i) {
        if (contenido[i] == '\r' && i + 1 < contenido.size() && contenido[i + 1] == '\n') {
            continue;
        }
        salida += contenido[i];
    }
    return salida;
}

} // namespace

TEST_SUITE("guardarArchivoReg") {

TEST_CASE("crea RealViewOn<version>.reg en el directorio actual") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"[HKEY_CURRENT_USER\\Test]"}, kVersionRvo);

    CHECK(std::filesystem::exists(tmp.ruta() / "RealViewOn2025.reg"));
}

TEST_CASE("la primera linea es la cabecera que exige regedit") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"[HKEY_CURRENT_USER\\Test]"}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    CHECK(contenido.rfind("Windows Registry Editor Version 5.00\n", 0) == 0);
}

TEST_CASE("registra la version de RealViewOn y la version de SolidWorks objetivo") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    CHECK(contenido.find(";# Created with: RealViewOn v" + kVersionRvo + " #") != std::string::npos);
    CHECK(contenido.find("; - **SW Target:** 2025") != std::string::npos);
}

TEST_CASE("cada linea de contenido queda en su propio renglon y en orden") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"primera", "segunda", "tercera"}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    const std::size_t p1 = contenido.find("\nprimera\n");
    const std::size_t p2 = contenido.find("\nsegunda\n");
    const std::size_t p3 = contenido.find("\ntercera");

    CHECK(p1 != std::string::npos);
    CHECK(p2 != std::string::npos);
    CHECK(p3 != std::string::npos);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
}

TEST_CASE("el archivo no termina en salto de linea") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"ultima"}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    REQUIRE_FALSE(contenido.empty());
    CHECK(contenido.back() != '\n');
    CHECK(contenido.back() != '\r');
}

TEST_CASE("los saltos de linea sobrantes al final se recortan") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"ultima", "", ""}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    REQUIRE(contenido.size() >= 6);
    CHECK(contenido.substr(contenido.size() - 6) == "ultima");
}

TEST_CASE("sin contenido igual escribe una cabecera valida") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    CHECK(contenido.rfind("Windows Registry Editor Version 5.00\n", 0) == 0);
    CHECK(contenido.back() != '\n');
}

TEST_CASE("volver a guardar sobreescribe en lugar de acumular") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2025", {"vieja"}, kVersionRvo);
    guardarArchivoReg("2025", {"nueva"}, kVersionRvo);
    const std::string contenido = leerNormalizado(tmp.ruta() / "RealViewOn2025.reg");

    CHECK(contenido.find("nueva") != std::string::npos);
    CHECK(contenido.find("vieja") == std::string::npos);
}

TEST_CASE("la version forma parte del nombre del archivo") {
    DirectorioTemporal tmp;

    guardarArchivoReg("2019", {"x"}, kVersionRvo);
    guardarArchivoReg("2026", {"x"}, kVersionRvo);

    CHECK(std::filesystem::exists(tmp.ruta() / "RealViewOn2019.reg"));
    CHECK(std::filesystem::exists(tmp.ruta() / "RealViewOn2026.reg"));
}

} // TEST_SUITE
