#ifndef RECURSOS_H
#define RECURSOS_H

// Recursos propios de la GUI. El ícono (IDI_ICON1) y la música (IDR_MIDI_FILE)
// son los de la consola, en include/resource.h.

#define IDD_PRINCIPAL           201
#define IDD_VISTA_PREVIA        202
#define IDD_ACERCA_DE           203
#define IDR_ANIMACION           301

#define IDC_ACERCA              1000 // La barra de color de arriba.
#define IDC_VERSION             1001
#define IDC_VERSION_ESTADO      1002
#define IDC_RENDERER            1003
#define IDC_RENDERER_ORIGEN     1004
// Los tres radios tienen que ser consecutivos.
#define IDC_NVIDIA              1005
#define IDC_AMD                 1006
#define IDC_INTEL               1007
#define IDC_MARCA_ESTADO        1008
#define IDC_AVANZADO            1009
// Un casillero por opción avanzada, consecutivos desde IDC_OPCION0.
#define IDC_OPCION0             1020
#define CANTIDAD_OPCIONES       7
#define IDC_ESTADO              1030
#define IDC_VISTA_PREVIA        1031
#define IDC_GENERAR             1032
#define IDC_ABRIR_CARPETA       1033
#define IDC_MUSICA              1034
#define IDC_BACKUP              1035
#define IDC_NOTA_RECOMENDADA    1036
#define IDC_ANIMACION           1037
#define IDC_ACERCA_NOMBRE       1050
#define IDC_ACERCA_ETIQUETAS    1051
#define IDC_ACERCA_VALORES      1052
#define IDC_ACERCA_REPO         1053
#define IDC_TEXTO_REG           1040

#endif // RECURSOS_H
