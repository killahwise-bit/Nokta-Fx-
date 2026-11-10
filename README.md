# Nokta-Fx

Plugin de efectos simple para FL Studio (VST3 + Standalone) que agrega textura al sonido. 25 presets, una sola pantalla y color personalizable.

## Controles

| Control | Qué hace |
|---|---|
| Texture | Saturación suave (calidez / cuerpo) |
| Grit | Bitcrush + reducción de muestreo (sucio / lo-fi) |
| Drift | Oscilación lenta de tono, tipo cinta o vinilo |
| Noise | Siseo suave + crujidos de polvo |
| Tone | Izquierda: oscurece. Derecha: aclara |
| Space | Reverb |
| Width | Ancho estéreo (0.5 = neutro) |
| Mix | Seco / procesado |
| Output | Volumen de salida (±12 dB) |

Doble clic en un knob lo devuelve a su valor por defecto. También puedes escribir el valor en la caja de abajo.

## Color

Botón de menú (tres líneas, arriba a la derecha) > Color: Morado, Violeta, Azul, Cian, Verde, Lima, Ámbar, Naranja, Rojo, Rosa, Magenta y Plata.
"Matiz personalizado..." abre una barra para elegir cualquier color. El color se guarda con el proyecto de FL Studio.

## Presets

Polvo, Cinta, Vapor, Gris, Bruma, Vacio, Cristal, Oxido, Deriva, Terciopelo, Ceniza, Eclipse, Neblina, Cassette, Radio, Subsuelo, Brasa, Escarcha, Sombra, Pixel, Humo, Ecos, Nostalgia, Noche, Cenit.

## Compilar

Requisitos: CMake 3.22+ y un compilador C++17 (Visual Studio 2022 en Windows). JUCE se descarga solo la primera vez (necesita internet).

```
cmake -B build
cmake --build build --config Release
```

También se compila solo en GitHub Actions (carpeta `.github/workflows`): al subir el código, descarga el artefacto `nokt-Fx-VST3`.

Copia `Nokta-Fx.vst3` a `C:\Program Files\Common Files\VST3` y en FL Studio usa Options > Manage plugins > Find installed plugins.
