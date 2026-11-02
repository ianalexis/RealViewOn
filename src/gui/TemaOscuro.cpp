#include "TemaOscuro.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <vssym32.h>
#include <cwchar>

namespace tema {
namespace {

constexpr COLORREF kFondo = RGB(32, 32, 32);
constexpr COLORREF kCampo = RGB(45, 45, 45);
constexpr COLORREF kTexto = RGB(255, 255, 255);
constexpr COLORREF kDeshabilitado = RGB(110, 110, 110);

bool modoOscuro = false;
bool forzado = false; // --dark, --light o el casillero de la GUI: no seguir al sistema.
HBRUSH pincelFondo = nullptr;
HBRUSH pincelCampo = nullptr;

DWORD compilacionWindows() {
    // GetVersionEx está obsoleta; RtlGetVersion devuelve la versión real.
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    const auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
        reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")));
    RTL_OSVERSIONINFOW version = {sizeof(version)};
    return (rtlGetVersion != nullptr && rtlGetVersion(&version) == 0) ? version.dwBuildNumber : 0;
}

bool sistemaOscuro() {
    if (!disponible()) {
        return false;
    }
    DWORD claro = 1;
    DWORD tamano = sizeof(claro);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &claro, &tamano) != ERROR_SUCCESS) {
        return false;
    }
    return claro == 0;
}

bool esClase(HWND control, const wchar_t* clase) {
    wchar_t nombre[32] = {};
    GetClassNameW(control, nombre, 32);
    return _wcsicmp(nombre, clase) == 0;
}

void temaControl(HWND control) {
    const bool combo = esClase(control, WC_COMBOBOXW);
    const wchar_t* subApp = nullptr; // nullptr: el tema normal.
    if (modoOscuro) {
        if (combo) {
            subApp = L"DarkMode_CFD";
        } else if (esClase(control, WC_BUTTONW) || esClase(control, WC_EDITW)) {
            subApp = L"DarkMode_Explorer";
        }
    }
    SetWindowTheme(control, subApp, nullptr);
    if (combo) {
        // Al cambiarle el tema, el combo selecciona su texto y lo deja resaltado
        // aunque no tenga el foco.
        SendMessageW(control, CB_SETEDITSEL, 0, MAKELPARAM(-1, -1));
        // La lista desplegable es una ventana aparte, con su propia barra.
        COMBOBOXINFO info = {sizeof(info)};
        if (GetComboBoxInfo(control, &info)) {
            SetWindowTheme(info.hwndList, modoOscuro ? L"DarkMode_Explorer" : nullptr, nullptr);
        }
    }
}

} // namespace

void inicializar(bool forzarOscuro, bool forzarClaro) {
    forzado = forzarOscuro || forzarClaro;
    modoOscuro = forzarOscuro || (!forzarClaro && sistemaOscuro());
    pincelFondo = CreateSolidBrush(kFondo);
    pincelCampo = CreateSolidBrush(kCampo);
}

void liberar() {
    DeleteObject(pincelFondo);
    DeleteObject(pincelCampo);
    pincelFondo = nullptr;
    pincelCampo = nullptr;
}

bool oscuro() {
    return modoOscuro;
}

bool disponible() {
    return compilacionWindows() >= 17763; // Windows 10 1809: primera con los temas DarkMode_*.
}

void forzar(bool activar) {
    forzado = true;
    modoOscuro = activar;
}

bool actualizarDesdeSistema() {
    if (forzado) {
        return false;
    }
    const bool anterior = modoOscuro;
    modoOscuro = sistemaOscuro();
    return modoOscuro != anterior;
}

void aplicar(HWND ventana) {
    const BOOL activar = modoOscuro;
    // 20 desde Windows 10 2004; 19 en 1809-1909. En Windows 7 falla y no pasa nada.
    if (FAILED(DwmSetWindowAttribute(ventana, 20, &activar, sizeof(activar)))) {
        DwmSetWindowAttribute(ventana, 19, &activar, sizeof(activar));
    }
    for (HWND hijo = GetWindow(ventana, GW_CHILD); hijo != nullptr; hijo = GetWindow(hijo, GW_HWNDNEXT)) {
        temaControl(hijo);
    }
    RedrawWindow(ventana, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
}

COLORREF color(Tono tono) {
    if (modoOscuro) {
        switch (tono) {
        case Tono::Tenue: return RGB(160, 160, 160);
        case Tono::Exito: return RGB(108, 203, 95);
        case Tono::Info: return RGB(96, 205, 255);
        case Tono::Error: return RGB(255, 153, 164);
        default: return kTexto;
        }
    }
    switch (tono) {
    case Tono::Tenue: return GetSysColor(COLOR_GRAYTEXT);
    case Tono::Exito: return RGB(0, 128, 0);
    case Tono::Info: return RGB(0, 70, 170);
    case Tono::Error: return RGB(192, 0, 0);
    default: return GetSysColor(COLOR_WINDOWTEXT);
    }
}

COLORREF fondo() {
    return modoOscuro ? kFondo : GetSysColor(COLOR_BTNFACE);
}

HBRUSH colorear(UINT msg, HDC dc, HWND control) {
    if (!modoOscuro) {
        return nullptr;
    }
    // Los edits de solo lectura mandan WM_CTLCOLORSTATIC, pero llevan fondo de campo.
    const bool campo = msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX ||
                       (msg == WM_CTLCOLORSTATIC && esClase(control, WC_EDITW));
    if (msg != WM_CTLCOLORDLG) {
        SetTextColor(dc, kTexto);
        SetBkColor(dc, campo ? kCampo : kFondo);
    }
    return campo ? pincelCampo : pincelFondo;
}

LRESULT dibujarCasilla(const NMCUSTOMDRAW& cd) {
    if (!modoOscuro || cd.dwDrawStage != CDDS_PREPAINT) {
        return CDRF_DODEFAULT;
    }
    HWND control = cd.hdr.hwndFrom;
    const LONG tipo = GetWindowLongW(control, GWL_STYLE) & BS_TYPEMASK;
    const bool radio = tipo == BS_RADIOBUTTON || tipo == BS_AUTORADIOBUTTON;
    if (!radio && tipo != BS_CHECKBOX && tipo != BS_AUTOCHECKBOX) {
        return CDRF_DODEFAULT;
    }

    const bool habilitada = IsWindowEnabled(control) != FALSE;
    // RBS_* tienen los mismos valores que CBS_*: normal, hot, pressed, disabled.
    int estado = SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
    if (!habilitada) {
        estado += 3;
    } else if (cd.uItemState & CDIS_SELECTED) {
        estado += 2;
    } else if (cd.uItemState & CDIS_HOT) {
        estado += 1;
    }
    const int parte = radio ? BP_RADIOBUTTON : BP_CHECKBOX;

    FillRect(cd.hdc, &cd.rc, pincelFondo);
    HTHEME temaBoton = OpenThemeData(control, L"Button");
    SIZE caja = {13, 13};
    GetThemePartSize(temaBoton, cd.hdc, parte, estado, nullptr, TS_DRAW, &caja);
    RECT rcCaja = cd.rc;
    rcCaja.top += (cd.rc.bottom - cd.rc.top - caja.cy) / 2;
    rcCaja.right = rcCaja.left + caja.cx;
    rcCaja.bottom = rcCaja.top + caja.cy;
    DrawThemeBackground(temaBoton, cd.hdc, parte, estado, &rcCaja, nullptr);
    CloseThemeData(temaBoton);

    wchar_t texto[256] = {};
    GetWindowTextW(control, texto, 256);
    RECT rcTexto = cd.rc;
    rcTexto.left = rcCaja.right + caja.cx / 3;
    HGDIOBJ anterior = SelectObject(cd.hdc, reinterpret_cast<HFONT>(SendMessageW(control, WM_GETFONT, 0, 0)));
    SetBkMode(cd.hdc, TRANSPARENT);
    SetTextColor(cd.hdc, habilitada ? kTexto : kDeshabilitado);
    const LRESULT ui = SendMessageW(control, WM_QUERYUISTATE, 0, 0);
    UINT formato = DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS;
    if (ui & UISF_HIDEACCEL) {
        formato |= DT_HIDEPREFIX;
    }
    DrawTextW(cd.hdc, texto, -1, &rcTexto, formato);
    if ((cd.uItemState & CDIS_FOCUS) && !(ui & UISF_HIDEFOCUS)) {
        RECT foco = rcTexto;
        DrawTextW(cd.hdc, texto, -1, &foco, formato | DT_CALCRECT);
        const LONG alto = foco.bottom - foco.top;
        foco.top = rcTexto.top + (rcTexto.bottom - rcTexto.top - alto) / 2;
        foco.bottom = foco.top + alto;
        InflateRect(&foco, 1, 0);
        DrawFocusRect(cd.hdc, &foco);
    }
    SelectObject(cd.hdc, anterior);
    return CDRF_SKIPDEFAULT;
}

} // namespace tema
