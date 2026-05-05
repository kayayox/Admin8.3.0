# Admin8.3.0 – Motor NLP en C++

**Autor:** Soubhi Khayat Najjar
**Año:** 2026

Motor de Procesamiento de Lenguaje Natural (NLP) escrito en C++17, diseñado para aplicaciones interactivas que requieren clasificación morfológica, aprendizaje contextual, generación de respuestas y predicción de palabras. El sistema utiliza tres bases de datos SQLite para persistencia semántica, de patrones y temporal.

---

## 🌟 Características principales

- **Clasificación morfológica** de palabras (sustantivos, verbos, adjetivos, etc.) basada en listas estáticas y sufijos.
- **Refinamiento contextual** mediante modelos de unigramas, bigramas y trigramas de etiquetas gramaticales.
- **Aprendizaje continuo** a partir de oraciones y correcciones del usuario.
- **Predicción de la siguiente palabra** usando correladores contextuales y de chunks (frases).
- **Generación de respuestas** (hipótesis) usando reglas de inferencia, plantillas y correlación contextual.
- **Detección de comandos**, sujetos y objetos en una oración.
- **Persistencia** en tres bases de datos SQLite separadas:
  - *Semántica*: palabras, oraciones, diálogos.
  - *Patrones*: correlaciones, estadísticas de etiquetas, plantillas de respuesta.
  - *Temporal* (opcional, por defecto en memoria).

---

## 🧱 Arquitectura del proyecto

src/
├── api/            # Fachada pública (NLPEngine)
├── common/         # Tipos enumerados compartidos (types.hpp)
├── core/           # Entidades del dominio: Word, Sentence, Pattern, Command, Dialogue
├── db/             # Repositorios y DatabaseManager (SQLite)
├── nlp/            # Tokenizador, morfología, clasificador, estadísticas de etiquetas, refinador
├── dialogue/       # Correladores de patrones, contextos y chunks
└── utils/          # Chunker, helpers de aprendizaje, serialización, plantillas, slot filler


### Flujo de procesamiento de una oración

1. **Tokenización** → `tokenize()` divide la cadena en palabras, números y fechas.
2. **Clasificación inicial** → `Morphology` asigna una etiqueta tentativa (con confianza).
3. **Carga desde BD** → `WordRepository` recupera atributos conocidos.
4. **Refinamiento contextual** → `Classifier` y `Refiner` usan estadísticas de transición.
5. **Aprendizaje** → Se actualizan correladores (`PatternCorrelator`, `ContextualCorrelator`, `ChunkCorrelator`).
6. **Persistencia** → Se guardan la palabra, la oración y el patrón gramatical.
7. **Respuesta** → Si se solicita, se genera una hipótesis mediante (Requiere aprendisaje) `generateHypothesis()`.

---

## 📦 Requisitos de compilación

- **Compilador** con soporte C++17 (GCC 7+, Clang 5+, MSVC 2017+)
- **SQLite3** (biblioteca y cabeceras de desarrollo)
- **CMake** 3.10 o superior

### Instalación de dependencias (ejemplos)

- **Ubuntu/Debian**:
  ```bash
  sudo apt install libsqlite3-dev cmake g++

    macOS (Homebrew):
    bash

    brew install sqlite3 cmake

    Windows (MSYS2 o Visual Studio):

        Instalar SQLite3 y CMake, o usar vcpkg.

🔧 Compilación y ejecución
bash

# Clonar / descargar el código fuente
cd admin-nlp-engine

# Crear directorio de compilación
mkdir build && cd build

# Configurar y compilar
cmake ..
make

# Si deseas forzar la descarga de SQLite3
cmake -DFORCE_DOWNLOAD_SQLITE3=ON ..

# Ejecutar el programa de ejemplo interactivo
./nlp_engine

    Nota: El CMakeLists.txt proporcionado asume que main.cpp y la carpeta src/ están en el mismo nivel.

🚀 Uso de la API (NLPEngine)

La clase NLPEngine es la fachada principal. Ejemplo mínimo:
cpp

#include "src/api/NLPEngine.hpp"

int main() {
    NLPEngine engine;
    engine.initialize("semantic.db", "patterns.db", ":memory:");
    engine.setDebugMode(true);

    // Procesar una oración
    auto wordsInfo = engine.processSentence("El perro corre rápido");

    // Obtener información de una palabra
    WordInfo info = engine.getWordInfo("perro");

    // Predecir siguiente palabra
    auto preds = engine.predictNext("El perro");

    // Generar respuesta
    std::string respuesta = engine.generateResponse("¿Lloverá mañana?");

    // Corregir clasificación
    engine.correctWord("corre", "Verbo");

    engine.shutdown();
    return 0;
}

Estructuras de datos públicas

    WordInfo: contiene palabra, tipo gramatical, confianza, significado, atributos morfológicos y palabras relacionadas.

    Prediction: palabra candidata y su probabilidad.

🧪 Programa de demostración interactivo

El main.cpp incluido ofrece un menú con las siguientes opciones:

    Aprender de archivo – Lee un archivo de texto, lo divide en oraciones y las procesa.

    Aprender una frase – Aprende una única oración.

    Procesar una oración – Muestra la clasificación detallada de cada palabra.

    Mostrar información de una palabra – Atributos almacenados en la BD semántica.

    Corregir manualmente una palabra – Cambia su tipo y ajusta confianza.

    Reprocesar última oración – Refresca estadísticas tras correcciones.

    Predicción interactiva – Bucle de predicción palabra a palabra con opción de corrección.

    Generar respuesta – Dada una premisa, produce una hipótesis usando reglas/plantillas.

    Salir

🗄️ Bases de datos

El motor crea automáticamente las tablas necesarias. Los archivos .db pueden inspeccionarse con herramientas como sqlite3 CLI. Principales tablas:

    palabras, relaciones – léxico y asociaciones semánticas.

    oraciones, bloques – oraciones almacenadas y sus tokens.

    dialogos, feedback – historial de diálogos y correcciones.

    patrones – secuencias de tipos gramaticales.

    tag_unigrams, tag_bigrams, tag_trigrams – estadísticas de transición.

    response_templates – plantillas para generación de hipótesis.

    words, patterns, correlations (con sufijos _chunk o vacío) – datos de correladores.

🧠 Modelos de aprendizaje

    Correlador de patrones (PatternCorrelator): almacena trigramas (palabra actual, patrón anterior, patrón siguiente) con pesos.

    Correlador contextual (ContextualCorrelator): usa contexto de hasta dos palabras anteriores para predecir la siguiente.

    Correlador de chunks (ChunkCorrelator): trabaja con frases en lugar de palabras, útil para predicciones de estructura.

    Estadísticas de etiquetas (TagStats): unigramas, bigramas y trigramas entre TipoPalabra con suavizado.

📝 Ejemplo de sesión
text

=== MENÚ PRINCIPAL ===
1. Aprender de archivo
2. Aprender una frase
3. Procesar una oración
...
Opción: 3
Oración a procesar: El gato negro duerme en la cama.
Clasificación:
Palabra: el
  Tipo: Artículo (confianza: 0.99)
  Significado: La palabra "el" es un artículo masculino en singular que determina al sustantivo.
Palabra: gato
  Tipo: Sustantivo (confianza: 0.95)
  ...

🤝 Contribuciones

El proyecto es cerrado (desarrollado por el autor). Sin embargo, se aceptan sugerencias o reportes de problemas a través de los canales establecidos por el equipo de Admin8.3.0.
📜 Licencia

Este software se proporciona “tal cual”, sin garantías de ningún tipo. El autor no se hace responsable de ningún daño derivado de su uso.
📚 Referencias técnicas

    SQLite3: https://www.sqlite.org/

    C++17 Standard Library

    Algoritmos de clasificación morfológica basados en sufijos y listas estáticas (inspirados en enfoques de lingüística computacional).
