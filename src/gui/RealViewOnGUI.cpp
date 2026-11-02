// Interfaz gráfica de RealViewOn: el mismo recorrido que la consola, en una sola
// ventana. Win32 puro, sin dependencias: el diseño está en RealViewOnGUI.rc.
//
// Sigue el modo claro/oscuro de Windows (ver TemaOscuro.h). --dark y --light lo
// fuerzan.

#include <Windows.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <cstddef>
#include <cwchar>
#include <cwctype>
#include <exception>
#include <string>
#include <utility>
#include <vector>
#include "AdvanceMode.h"
#include "Animacion.h"
#include "GPU.h"
#include "Musica.h"
#include "Registro.h"
#include "SolidWorks.h"
#include "TemaOscuro.h"
#include "Version.h"
#include "recursos.h"
#include "resource.h"

// Commit del que salió el binario. Lo define RealViewOnGUI.vcxproj al compilar
// (target HashDeGit), sin comillas; sin git, o fuera de MSBuild, queda "unknown".
#ifndef RVO_GIT_HASH
#define RVO_GIT_HASH unknown
#endif
#define RVO_GIT_HASH_TEXTO TOSTRING(RVO_GIT_HASH)

namespace {

constexpr const wchar_t* kRepositorio = L"https://github.com/ianalexis/RealViewOn";

// Mensajes propios.
// CBN_SELCHANGE llega antes de que el combo actualice su texto, así que se
// posterga el procesamiento hasta que lo haya hecho.
constexpr UINT WM_APP_VERSION = WM_APP + 1;
constexpr UINT WM_APP_RENDERER = WM_APP + 2;
constexpr UINT WM_APP_MUSICA = WM_APP + 3; // La música terminó (ver Musica.h).
constexpr UINT WM_APP_DPI = WM_APP + 4;    // Ya se reescaló la ventana tras WM_DPICHANGED.

// Valores de SolidWorks::esCompatible().
constexpr int kNoCompatible = 0;
constexpr int kFutura = 2;

constexpr int kRadios[] = {IDC_NVIDIA, IDC_AMD, IDC_INTEL};
constexpr GPU::Brand kMarcas[] = {GPU::Brand::NVIDIA, GPU::Brand::AMD, GPU::Brand::INTEL};

struct Estado {
    SolidWorks sw;
    std::vector<std::pair<std::string, std::string>> candidatos;
    std::vector<AdvanceMode::Opcion> opciones;
    int ano = 0;
    int compatibilidad = kNoCompatible;
    bool generico = false;
    GPU::Current detectado; // Lo que encontró el core para la versión elegida.
    GPU::Brand marcaDetectada = GPU::Brand::UNKNOWN;
    tema::Tono tonoVersion = tema::Tono::Tenue;
    tema::Tono tonoEstado = tema::Tono::Normal;
    std::string rutaGenerada;
    bool backupElegido = false; // Lo que eligió el usuario para cuando no hay modo avanzado.
    COLORREF fondoBarra = 0;
    HFONT fuenteBarra = nullptr;
    Musica musica;
    Animacion animacion;
    bool actualizando = false; // Cambios de texto hechos desde el código.
};

Estado g;

// El core usa la página de códigos ANSI (las funciones A del registro); la
// ventana, UTF-16.
std::wstring ancho(const std::string& s) {
    if (s.empty()) {
        return {};
    }
    const int n = MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

std::string angosto(const std::wstring& w) {
    if (w.empty()) {
        return {};
    }
    const int n = WideCharToMultiByte(CP_ACP, 0, w.data(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_ACP, 0, w.data(), static_cast<int>(w.size()), s.data(), n, nullptr, nullptr);
    return s;
}

std::wstring textoDe(HWND hDlg, int id) {
    HWND control = GetDlgItem(hDlg, id);
    std::wstring texto(GetWindowTextLengthW(control), L'\0');
    GetWindowTextW(control, texto.data(), static_cast<int>(texto.size()) + 1);
    return texto;
}

bool marcado(HWND hDlg, int id) {
    return IsDlgButtonChecked(hDlg, id) == BST_CHECKED;
}

std::wstring titulo() {
    return ancho("RealViewOn v" + std::string(RVO_VERSION) + " c" + std::string(RVO_COMPILATION) + " - " + releaseType());
}

// Copia la fuente del diálogo con algún cambio (negrita, tamaño, subrayado).
HFONT variarFuente(HWND hDlg, LONG peso, int escalaPorCien, bool subrayada) {
    LOGFONTW lf = {};
    GetObjectW(reinterpret_cast<HFONT>(SendMessageW(hDlg, WM_GETFONT, 0, 0)), sizeof(lf), &lf);
    lf.lfWeight = peso;
    lf.lfHeight = lf.lfHeight * escalaPorCien / 100;
    lf.lfUnderline = subrayada ? TRUE : FALSE;
    return CreateFontIndirectW(&lf);
}

// Mano sobre lo que se puede cliquear y que no parece un botón.
bool manoSobre(HWND hDlg, WPARAM ventanaBajoCursor, int id) {
    if (reinterpret_cast<HWND>(ventanaBajoCursor) != GetDlgItem(hDlg, id)) {
        return false;
    }
    SetCursor(LoadCursor(nullptr, IDC_HAND));
    SetWindowLongPtrW(hDlg, DWLP_MSGRESULT, TRUE);
    return true;
}

// La barra de color de arriba es el botón About: mismos colores que el
// encabezado de la consola, azul en las estables y rojo en las prerelease (ver
// terminalColorAuto()), igual en los dos modos.
void dibujarBarra(const DRAWITEMSTRUCT& item) {
    COLORREF fondo = g.fondoBarra;
    if (item.itemState & ODS_SELECTED) {
        fondo = RGB(GetRValue(fondo) * 3 / 4, GetGValue(fondo) * 3 / 4, GetBValue(fondo) * 3 / 4);
    }
    HBRUSH pincel = CreateSolidBrush(fondo);
    FillRect(item.hDC, &item.rcItem, pincel);
    DeleteObject(pincel);

    const COLORREF color = FILE_VERSION_STABLE == 1 ? RGB(255, 255, 255) : RGB(255, 255, 0);
    wchar_t texto[64] = {};
    GetWindowTextW(item.hwndItem, texto, 64);
    HGDIOBJ anterior = SelectObject(item.hDC, g.fuenteBarra);
    SetBkMode(item.hDC, TRANSPARENT);
    SetTextColor(item.hDC, color);
    RECT rc = item.rcItem;
    DrawTextW(item.hDC, texto, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    if ((item.itemState & ODS_FOCUS) && !(item.itemState & ODS_NOFOCUSRECT)) {
        RECT foco = item.rcItem;
        InflateRect(&foco, -2, -2);
        SetBkColor(item.hDC, fondo);
        DrawFocusRect(item.hDC, &foco);
    }
    SelectObject(item.hDC, anterior);
}

GPU::Brand marcaElegida(HWND hDlg) {
    for (int i = 0; i < 3; i++) {
        if (marcado(hDlg, kRadios[i])) {
            return kMarcas[i];
        }
    }
    return GPU::Brand::UNKNOWN;
}

std::vector<bool> opcionesElegidas(HWND hDlg) {
    std::vector<bool> elegidas;
    for (std::size_t i = 0; i < g.opciones.size() && i < CANTIDAD_OPCIONES; i++) {
        elegidas.push_back(marcado(hDlg, IDC_OPCION0 + static_cast<int>(i)));
    }
    return elegidas;
}

// El renderer sale del texto del combo. Si es el detectado conserva vendor y
// workarounds; si es otro de la lista o uno escrito a mano, se descartan, igual
// que en la consola cuando se rechaza el detectado.
GPU::Current currentElegido(HWND hDlg) {
    const std::string texto = angosto(textoDe(hDlg, IDC_RENDERER));
    if (texto.empty()) {
        return {};
    }
    if (texto == g.detectado.renderer) {
        return g.detectado;
    }
    for (const auto& [nombre, origen] : g.candidatos) {
        if (nombre == texto) {
            return {texto, "", "", origen};
        }
    }
    return {texto, "", "", "Manual"};
}

void mostrarEstado(HWND hDlg, const std::wstring& texto, tema::Tono tono) {
    g.tonoEstado = tono;
    SetDlgItemTextW(hDlg, IDC_ESTADO, texto.c_str());
}

void actualizarBotones(HWND hDlg) {
    std::wstring falta;
    if (g.compatibilidad == kNoCompatible) {
        falta = L"Choose a compatible SolidWorks version.";
    } else if (textoDe(hDlg, IDC_RENDERER).empty()) {
        falta = L"Choose or type the renderer.";
    } else if (marcaElegida(hDlg) == GPU::Brand::UNKNOWN) {
        falta = L"Choose the GPU brand.";
    }
    const bool listo = falta.empty();
    EnableWindow(GetDlgItem(hDlg, IDC_VISTA_PREVIA), listo);
    EnableWindow(GetDlgItem(hDlg, IDC_GENERAR), listo);
    // Cualquier cambio deja viejo el archivo generado.
    g.rutaGenerada.clear();
    ShowWindow(GetDlgItem(hDlg, IDC_ABRIR_CARPETA), SW_HIDE);
    mostrarEstado(hDlg, listo ? L"Ready." : falta, listo ? tema::Tono::Normal : tema::Tono::Tenue);
}

void actualizarEstadoMarca(HWND hDlg) {
    const GPU::Brand elegida = marcaElegida(hDlg);
    const wchar_t* texto = L"";
    if (elegida == GPU::Brand::UNKNOWN) {
        texto = L"Not detected: pick one";
    } else if (elegida == g.marcaDetectada) {
        texto = L"Auto-detected";
    } else {
        texto = L"Manual";
    }
    SetDlgItemTextW(hDlg, IDC_MARCA_ESTADO, texto);
    actualizarBotones(hDlg);
}

void actualizarOpciones(HWND hDlg) {
    const bool avanzado = marcado(hDlg, IDC_AVANZADO);
    // El modo avanzado siempre hace el backup, como en la consola: la casilla
    // queda marcada y bloqueada. Sin él vuelve a lo que había elegido el usuario.
    EnableWindow(GetDlgItem(hDlg, IDC_BACKUP), !avanzado);
    CheckDlgButton(hDlg, IDC_BACKUP, (avanzado || g.backupElegido) ? BST_CHECKED : BST_UNCHECKED);
    for (int i = 0; i < CANTIDAD_OPCIONES; i++) {
        HWND casilla = GetDlgItem(hDlg, IDC_OPCION0 + i);
        if (i >= static_cast<int>(g.opciones.size())) {
            ShowWindow(casilla, SW_HIDE);
            continue;
        }
        const AdvanceMode::Opcion& opcion = g.opciones[i];
        const bool disponible = !(opcion.soloInstalada && g.generico);
        std::wstring texto = ancho(opcion.nombre);
        if (opcion.recomendada) {
            texto += L" *"; // Ver la nota "* Recommended" debajo de la lista.
        }
        if (!disponible) {
            texto += L" (not available in generic mode)";
        }
        SetWindowTextW(casilla, texto.c_str());
        EnableWindow(casilla, avanzado && disponible);
    }
}

void alCambiarRenderer(HWND hDlg) {
    const GPU::Current current = currentElegido(hDlg);
    std::wstring origen = L"Not detected. Pick one from the list or type it.";
    if (!current.renderer.empty()) {
        origen = L"Source: " + ancho(current.origin);
        if (!current.vendor.empty()) {
            origen += L"   Vendor: " + ancho(current.vendor);
        }
    }
    SetDlgItemTextW(hDlg, IDC_RENDERER_ORIGEN, origen.c_str());

    g.marcaDetectada = current.renderer.empty() ? GPU::Brand::UNKNOWN : GPU::detectarMarca(current.vendor, current.renderer);
    for (int i = 0; i < 3; i++) {
        CheckDlgButton(hDlg, kRadios[i], kMarcas[i] == g.marcaDetectada ? BST_CHECKED : BST_UNCHECKED);
    }
    actualizarEstadoMarca(hDlg);
}

void alCambiarVersion(HWND hDlg) {
    const std::wstring texto = textoDe(hDlg, IDC_VERSION);
    bool valido = texto.size() == 4;
    for (wchar_t c : texto) {
        valido = valido && std::iswdigit(c);
    }
    g.ano = valido ? std::stoi(texto) : 0;
    g.compatibilidad = valido ? g.sw.esCompatible(g.ano) : kNoCompatible;
    g.generico = g.compatibilidad != kNoCompatible && g.sw.requiereGenerico(g.ano);

    const wchar_t* estado = L"";
    if (!valido) {
        estado = L"Enter a 4-digit year";
        g.tonoVersion = tema::Tono::Tenue;
    } else if (g.compatibilidad == kNoCompatible) {
        estado = L"Not compatible";
        g.tonoVersion = tema::Tono::Error;
    } else if (!g.generico) {
        estado = L"Installed";
        g.tonoVersion = tema::Tono::Exito;
    } else if (g.compatibilidad == kFutura) {
        estado = L"Future version: generic mode";
        g.tonoVersion = tema::Tono::Info;
    } else {
        estado = L"Not installed: generic mode";
        g.tonoVersion = tema::Tono::Info;
    }
    SetDlgItemTextW(hDlg, IDC_VERSION_ESTADO, estado);

    // Cada versión tiene su propio renderer detectado: se reemplaza el del combo.
    g.detectado = GPU::Current{};
    if (g.compatibilidad != kNoCompatible) {
        g.sw.setVersion(g.ano, g.generico);
        g.detectado = g.sw.detectarCurrent();
    }
    g.actualizando = true;
    SetDlgItemTextW(hDlg, IDC_RENDERER, ancho(g.detectado.renderer).c_str());
    g.actualizando = false;

    actualizarOpciones(hDlg);
    alCambiarRenderer(hDlg);
}

// Los mismos pasos que main() en la consola, con lo elegido en la ventana.
std::string armarReg(HWND hDlg) {
    g.sw.setVersion(g.ano, g.generico);
    GPU gpu(currentElegido(hDlg));
    // La consola pide la marca solo si no la pudo detectar; acá también se puede
    // corregir la detectada. En los dos casos queda como "Manual selected".
    const GPU::Brand marca = marcaElegida(hDlg);
    if (gpu.getBrand() != marca) {
        gpu.setMarcaManual(marca);
    }
    std::vector<std::string> contenido = gpu.completarContenidoReg(g.sw.obtenerRegBase());
    if (marcado(hDlg, IDC_AVANZADO)) {
        AdvanceMode advMode;
        advMode.setSwVersion(g.ano, g.generico);
        contenido.push_back(advMode.armarOpciones(opcionesElegidas(hDlg)));
    }
    return armarArchivoReg(std::to_string(g.ano), contenido, RVO_VERSION);
}

// Pone el texto en el control y lo reacomoda cada vez que cambia el tamaño.
void acomodarVistaPrevia(HWND hDlg) {
    RECT cliente;
    GetClientRect(hDlg, &cliente);
    // Márgenes y botón en unidades de diálogo, como en el .rc.
    RECT medidas = {6, 6, 50, 14};
    MapDialogRect(hDlg, &medidas);
    const int margenX = medidas.left;
    const int margenY = medidas.top;
    const int anchoBoton = medidas.right;
    const int altoBoton = medidas.bottom;
    MoveWindow(GetDlgItem(hDlg, IDOK), cliente.right - margenX - anchoBoton, cliente.bottom - margenY - altoBoton,
               anchoBoton, altoBoton, TRUE);
    MoveWindow(GetDlgItem(hDlg, IDC_TEXTO_REG), margenX, margenY, cliente.right - 2 * margenX,
               cliente.bottom - 3 * margenY - altoBoton, TRUE);
}

INT_PTR CALLBACK DialogoVistaPrevia(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT monoespaciada = nullptr;
    static SIZE minimo = {};
    switch (msg) {
    case WM_INITDIALOG: {
        // Misma altura que la fuente del diálogo (ya escalada por DPI), en Consolas.
        LOGFONTW lf = {};
        GetObjectW(reinterpret_cast<HFONT>(SendMessageW(hDlg, WM_GETFONT, 0, 0)), sizeof(lf), &lf);
        wcscpy_s(lf.lfFaceName, L"Consolas");
        monoespaciada = CreateFontIndirectW(&lf);
        SendDlgItemMessageW(hDlg, IDC_TEXTO_REG, WM_SETFONT, reinterpret_cast<WPARAM>(monoespaciada), FALSE);
        SetDlgItemTextW(hDlg, IDC_TEXTO_REG, reinterpret_cast<const wchar_t*>(lParam));
        RECT ventana;
        GetWindowRect(hDlg, &ventana);
        minimo = {ventana.right - ventana.left, ventana.bottom - ventana.top};
        tema::aplicar(hDlg);
        SetFocus(GetDlgItem(hDlg, IDOK));
        return FALSE; // El foco ya está puesto.
    }
    case WM_SIZE:
        acomodarVistaPrevia(hDlg);
        return TRUE;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = {minimo.cx, minimo.cy};
        return TRUE;
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        if (HBRUSH pincel = tema::colorear(msg, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam))) {
            return reinterpret_cast<INT_PTR>(pincel);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    case WM_DESTROY:
        DeleteObject(monoespaciada);
        monoespaciada = nullptr;
        break;
    }
    return FALSE;
}

void mostrarVistaPrevia(HWND hDlg) {
    try {
        // El control de edición necesita CRLF para cortar las líneas.
        std::wstring texto;
        for (wchar_t c : ancho(armarReg(hDlg))) {
            if (c == L'\n') {
                texto += L'\r';
            }
            texto += c;
        }
        DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_VISTA_PREVIA), hDlg, DialogoVistaPrevia,
                        reinterpret_cast<LPARAM>(texto.c_str()));
    } catch (const std::exception& e) {
        MessageBoxW(hDlg, ancho(e.what()).c_str(), L"RealViewOn", MB_OK | MB_ICONERROR);
    }
}

void generar(HWND hDlg) {
    try {
        // Mismo orden que la consola: primero el backup, después el .reg. Si el
        // backup falla se avisa y se sigue, como en modoAvanzado().
        std::wstring nota;
        tema::Tono tono = tema::Tono::Exito;
        if (marcado(hDlg, IDC_BACKUP)) {
            try {
                nota = L"\nBackup: " + ancho(guardarBackUp());
            } catch (const std::exception& e) {
                nota = L"\nBackup: " + ancho(e.what());
                tono = tema::Tono::Error;
            }
        }
        const std::string ruta = guardarArchivoReg(std::to_string(g.ano), armarReg(hDlg));
        // El resultado va en la ventana y no en un MessageBox: ese no tiene modo oscuro.
        g.rutaGenerada = ruta;
        mostrarEstado(hDlg, L"Created " + ancho(ruta.substr(ruta.find_last_of('\\') + 1)) + nota, tono);
        ShowWindow(GetDlgItem(hDlg, IDC_ABRIR_CARPETA), SW_SHOW);
    } catch (const std::exception& e) {
        MessageBoxW(hDlg, ancho(e.what()).c_str(), L"RealViewOn", MB_OK | MB_ICONERROR);
    }
}

void abrirCarpeta(HWND hDlg) {
    if (!g.rutaGenerada.empty()) {
        const std::wstring argumentos = L"/select,\"" + ancho(g.rutaGenerada) + L"\"";
        ShellExecuteW(hDlg, nullptr, L"explorer.exe", argumentos.c_str(), nullptr, SW_SHOWNORMAL);
    }
}

// El enlace al repositorio es un botón dibujado como enlace: así se llega con Tab
// y se abre con Enter, cosa que un estático no permite.
void dibujarEnlace(const DRAWITEMSTRUCT& item, HFONT fuente) {
    HBRUSH pincel = CreateSolidBrush(tema::fondo());
    FillRect(item.hDC, &item.rcItem, pincel);
    DeleteObject(pincel);

    wchar_t texto[128] = {};
    GetWindowTextW(item.hwndItem, texto, 128);
    HGDIOBJ anterior = SelectObject(item.hDC, fuente);
    SetBkMode(item.hDC, TRANSPARENT);
    SetTextColor(item.hDC, tema::color(tema::Tono::Info));
    RECT rc = item.rcItem;
    const UINT formato = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
    DrawTextW(item.hDC, texto, -1, &rc, formato);
    if ((item.itemState & ODS_FOCUS) && !(item.itemState & ODS_NOFOCUSRECT)) {
        RECT foco = item.rcItem;
        DrawTextW(item.hDC, texto, -1, &foco, formato | DT_CALCRECT);
        const LONG alto = foco.bottom - foco.top;
        foco.top = item.rcItem.top + (item.rcItem.bottom - item.rcItem.top - alto) / 2;
        foco.bottom = foco.top + alto;
        InflateRect(&foco, 1, 0);
        DrawFocusRect(item.hDC, &foco);
    }
    SelectObject(item.hDC, anterior);
}

INT_PTR CALLBACK DialogoAcercaDe(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT fuenteNombre = nullptr;
    static HFONT fuenteEnlace = nullptr;
    switch (msg) {
    case WM_INITDIALOG: {
        fuenteNombre = variarFuente(hDlg, FW_BOLD, 150, false);
        fuenteEnlace = variarFuente(hDlg, FW_NORMAL, 100, true);
        SendDlgItemMessageW(hDlg, IDC_ACERCA_NOMBRE, WM_SETFONT, reinterpret_cast<WPARAM>(fuenteNombre), FALSE);
        SetDlgItemTextW(hDlg, IDC_ACERCA_ETIQUETAS, L"Version\nCompilation\nCommit\nBy");
        const std::string valores = std::string(RVO_VERSION) + " (" + releaseType() + ")\nc" + std::string(RVO_COMPILATION) +
                                    "\n" + RVO_GIT_HASH_TEXTO + "\n[RF47] && [IanAlexis]";
        SetDlgItemTextW(hDlg, IDC_ACERCA_VALORES, ancho(valores).c_str());
        SetDlgItemTextW(hDlg, IDC_ACERCA_REPO, kRepositorio);
        tema::aplicar(hDlg);
        SetFocus(GetDlgItem(hDlg, IDOK));
        return FALSE; // El foco ya está puesto.
    }
    case WM_DRAWITEM:
        if (wParam == IDC_ACERCA_REPO) {
            dibujarEnlace(*reinterpret_cast<const DRAWITEMSTRUCT*>(lParam), fuenteEnlace);
            return TRUE;
        }
        break;
    case WM_SETCURSOR:
        if (manoSobre(hDlg, wParam, IDC_ACERCA_REPO)) {
            return TRUE;
        }
        break;
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC: {
        HBRUSH pincel = tema::colorear(msg, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam));
        if (msg == WM_CTLCOLORSTATIC && GetDlgCtrlID(reinterpret_cast<HWND>(lParam)) == IDC_ACERCA_ETIQUETAS) {
            SetTextColor(reinterpret_cast<HDC>(wParam), tema::color(tema::Tono::Tenue));
            if (pincel == nullptr) {
                SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
                pincel = GetSysColorBrush(COLOR_BTNFACE);
            }
        }
        if (pincel != nullptr) {
            return reinterpret_cast<INT_PTR>(pincel);
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ACERCA_REPO:
            ShellExecuteW(hDlg, L"open", kRepositorio, nullptr, nullptr, SW_SHOWNORMAL);
            return TRUE;
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    case WM_DESTROY:
        DeleteObject(fuenteNombre);
        DeleteObject(fuenteEnlace);
        fuenteNombre = nullptr;
        fuenteEnlace = nullptr;
        break;
    }
    return FALSE;
}

void mostrarAcercaDe(HWND hDlg) {
    DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_ACERCA_DE), hDlg, DialogoAcercaDe, 0);
}

void alternarMusica(HWND hDlg) {
    if (marcado(hDlg, IDC_MUSICA)) {
        g.musica.reproducir(hDlg, WM_APP_MUSICA);
    } else {
        g.musica.detener();
    }
}

// La barra va en negrita sobre la fuente del diálogo. Se rehace al cambiar de
// monitor, porque Windows reescala la fuente del diálogo pero no las propias.
void crearFuenteBarra(HWND hDlg) {
    HFONT anterior = g.fuenteBarra;
    g.fuenteBarra = variarFuente(hDlg, FW_BOLD, 100, false);
    InvalidateRect(GetDlgItem(hDlg, IDC_ACERCA), nullptr, FALSE);
    if (anterior != nullptr) {
        DeleteObject(anterior);
    }
}

void inicializar(HWND hDlg) {
    HINSTANCE instancia = GetModuleHandleW(nullptr);
    SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIconW(instancia, MAKEINTRESOURCEW(IDI_ICON1))));
    SendMessageW(hDlg, WM_SETICON, ICON_SMALL,
                 reinterpret_cast<LPARAM>(LoadImageW(instancia, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON,
                                                     GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
    SetWindowTextW(hDlg, titulo().c_str());

    g.fondoBarra = FILE_VERSION_STABLE == 1 ? RGB(0, 0, 128) : RGB(128, 0, 0);
    crearFuenteBarra(hDlg);
    SetDlgItemTextW(hDlg, IDC_MUSICA, L"♪ &Music");
    g.animacion.iniciar(GetDlgItem(hDlg, IDC_ANIMACION), IDR_ANIMACION);

    tema::aplicar(hDlg);

    g.candidatos = g.sw.candidatosRenderer();
    g.opciones = AdvanceMode().opciones();

    HWND version = GetDlgItem(hDlg, IDC_VERSION);
    SendMessageW(version, CB_LIMITTEXT, 4, 0);
    SendMessageW(version, CB_SETCUEBANNER, 0, reinterpret_cast<LPARAM>(L"Year"));
    // Solo dígitos en el año, como en la consola.
    COMBOBOXINFO info = {sizeof(info)};
    if (GetComboBoxInfo(version, &info)) {
        SetWindowLongW(info.hwndItem, GWL_STYLE, GetWindowLongW(info.hwndItem, GWL_STYLE) | ES_NUMBER);
    }
    int elegida = -1;
    for (const auto& [ano, compatibilidad] : g.sw.obtenerVersionesInstaladas()) {
        const LRESULT i = SendMessageW(version, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::to_wstring(ano).c_str()));
        if (compatibilidad == 1) {
            elegida = static_cast<int>(i); // La más nueva compatible.
        }
    }
    SendMessageW(version, CB_SETCURSEL, elegida, 0);

    HWND renderer = GetDlgItem(hDlg, IDC_RENDERER);
    SendMessageW(renderer, CB_SETCUEBANNER, 0, reinterpret_cast<LPARAM>(L"Renderer name"));
    for (const auto& candidato : g.candidatos) {
        SendMessageW(renderer, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ancho(candidato.first).c_str()));
    }

    for (int i = 0; i < static_cast<int>(g.opciones.size()) && i < CANTIDAD_OPCIONES; i++) {
        CheckDlgButton(hDlg, IDC_OPCION0 + i, g.opciones[i].marcadaPorDefecto ? BST_CHECKED : BST_UNCHECKED);
    }
    // Arranca sin modo avanzado: las opciones quedan preparadas, pero bloqueadas
    // hasta que el usuario lo active.
    CheckDlgButton(hDlg, IDC_AVANZADO, BST_UNCHECKED);

    alCambiarVersion(hDlg);

    // Como la consola, con música al abrir.
    CheckDlgButton(hDlg, IDC_MUSICA, BST_CHECKED);
    alternarMusica(hDlg);
}

INT_PTR CALLBACK DialogoPrincipal(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        inicializar(hDlg);
        // Lo habitual es aceptar lo detectado: el foco arranca en Generate y no en
        // el año, que quedaría con el texto seleccionado y lo reemplazaría la
        // primera tecla.
        SetFocus(GetDlgItem(hDlg, IDC_GENERAR));
        return FALSE;

    case WM_APP_VERSION:
        alCambiarVersion(hDlg);
        return TRUE;

    case WM_APP_RENDERER:
        alCambiarRenderer(hDlg);
        return TRUE;

    case WM_APP_MUSICA:
        // Terminó la canción: el casillero queda listo para volver a escucharla.
        // Si no pudo sonar (sin salida MIDI) no tiene sentido ofrecerlo.
        CheckDlgButton(hDlg, IDC_MUSICA, BST_UNCHECKED);
        if (wParam == 0) {
            EnableWindow(GetDlgItem(hDlg, IDC_MUSICA), FALSE);
        }
        return TRUE;

    case WM_DPICHANGED:
        // Windows reescala el diálogo después de este mensaje.
        PostMessageW(hDlg, WM_APP_DPI, 0, 0);
        break;

    case WM_APP_DPI:
        crearFuenteBarra(hDlg);
        return TRUE;

    case WM_HELP:
        // F1, como en cualquier programa de Windows.
        mostrarAcercaDe(hDlg);
        return TRUE;

    case WM_SETCURSOR:
        if (manoSobre(hDlg, wParam, IDC_ACERCA)) {
            return TRUE;
        }
        break;

    case WM_SETTINGCHANGE:
        // El usuario cambió entre modo claro y oscuro en la configuración de Windows.
        if (lParam != 0 && wcscmp(reinterpret_cast<const wchar_t*>(lParam), L"ImmersiveColorSet") == 0 &&
            tema::actualizarDesdeSistema()) {
            tema::aplicar(hDlg);
        }
        break;

    case WM_DRAWITEM:
        if (wParam == IDC_ANIMACION) {
            g.animacion.dibujar(*reinterpret_cast<const DRAWITEMSTRUCT*>(lParam), tema::fondo());
            return TRUE;
        }
        if (wParam == IDC_ACERCA) {
            dibujarBarra(*reinterpret_cast<const DRAWITEMSTRUCT*>(lParam));
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        if (reinterpret_cast<NMHDR*>(lParam)->code == NM_CUSTOMDRAW) {
            SetWindowLongPtrW(hDlg, DWLP_MSGRESULT, tema::dibujarCasilla(*reinterpret_cast<NMCUSTOMDRAW*>(lParam)));
            return TRUE;
        }
        break;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        const int id = GetDlgCtrlID(reinterpret_cast<HWND>(lParam));
        HBRUSH pincel = tema::colorear(msg, dc, reinterpret_cast<HWND>(lParam));
        if (msg == WM_CTLCOLORSTATIC) {
            tema::Tono tono = tema::Tono::Normal;
            switch (id) {
            case IDC_VERSION_ESTADO: tono = g.tonoVersion; break;
            case IDC_ESTADO: tono = g.tonoEstado; break;
            case IDC_RENDERER_ORIGEN:
            case IDC_MARCA_ESTADO:
            case IDC_NOTA_RECOMENDADA: tono = tema::Tono::Tenue; break;
            }
            if (tono != tema::Tono::Normal) {
                SetTextColor(dc, tema::color(tono));
                if (pincel == nullptr) {
                    SetBkMode(dc, TRANSPARENT);
                    pincel = GetSysColorBrush(COLOR_BTNFACE);
                }
            }
        }
        if (pincel != nullptr) {
            return reinterpret_cast<INT_PTR>(pincel);
        }
        break;
    }

    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        const int aviso = HIWORD(wParam);
        switch (id) {
        case IDC_VERSION:
            if (aviso == CBN_SELCHANGE) {
                PostMessageW(hDlg, WM_APP_VERSION, 0, 0);
            } else if (aviso == CBN_EDITCHANGE && !g.actualizando) {
                alCambiarVersion(hDlg);
            }
            return TRUE;
        case IDC_RENDERER:
            if (aviso == CBN_SELCHANGE) {
                PostMessageW(hDlg, WM_APP_RENDERER, 0, 0);
            } else if (aviso == CBN_EDITCHANGE && !g.actualizando) {
                alCambiarRenderer(hDlg);
            }
            return TRUE;
        case IDC_NVIDIA:
        case IDC_AMD:
        case IDC_INTEL:
            actualizarEstadoMarca(hDlg);
            return TRUE;
        case IDC_BACKUP:
            // Con modo avanzado está bloqueada, pero un BM_CLICK (automatización,
            // lectores de pantalla) igual la cambia: se vuelve a marcar.
            if (marcado(hDlg, IDC_AVANZADO)) {
                CheckDlgButton(hDlg, IDC_BACKUP, BST_CHECKED);
            } else {
                g.backupElegido = marcado(hDlg, IDC_BACKUP);
            }
            return TRUE;
        case IDC_AVANZADO:
            actualizarOpciones(hDlg);
            return TRUE;
        case IDC_MUSICA:
            alternarMusica(hDlg);
            return TRUE;
        case IDC_VISTA_PREVIA:
            mostrarVistaPrevia(hDlg);
            return TRUE;
        case IDC_GENERAR:
            generar(hDlg);
            return TRUE;
        case IDC_ACERCA:
            mostrarAcercaDe(hDlg);
            return TRUE;
        case IDC_ABRIR_CARPETA:
            abrirCarpeta(hDlg);
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:
        g.musica.detener();
        g.animacion.liberar();
        DeleteObject(g.fuenteBarra);
        break;
    }
    return FALSE;
}

} // namespace

int WINAPI wWinMain(HINSTANCE instancia, HINSTANCE, PWSTR lineaDeComandos, int) {
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    tema::inicializar(wcsstr(lineaDeComandos, L"--dark") != nullptr, wcsstr(lineaDeComandos, L"--light") != nullptr);
    const INT_PTR resultado = DialogBoxParamW(instancia, MAKEINTRESOURCEW(IDD_PRINCIPAL), nullptr, DialogoPrincipal, 0);
    tema::liberar();
    return static_cast<int>(resultado);
}
