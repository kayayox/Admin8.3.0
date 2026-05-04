/**==============================================================================
    Admin8.2.1 - NLPEngine.hpp
    Proposito: Fachada principal del motor NLP para aplicaciones interactivas.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_NLP_ENGINE_HPP
#define ADMIN821_NLP_ENGINE_HPP

#include <string>
#include <vector>
#include <memory>

// Estructuras de datos públicas
struct WordInfo {
    std::string word;
    std::string tipo;           // "Sustantivo", "Verbo", etc.
    float confianza;            // 0..1
    std::string significado;
    std::string cantidad;       // "Singular", "Plural", ""
    std::string tiempo;         // "Pasado", "Presente", "Futuro", ""
    std::string genero;         // "Masculino", "Femenino", "Neutro", ""
    std::string persona;        // "Primera", "Segunda", "Tercera", ""
    std::string grado;          // "Positivo", "Superlativo", ...
};

struct Prediction {
    std::string word;
    double probability;
};

class NLPEngine {
public:
    NLPEngine();
    ~NLPEngine();

    // Inicialización: rutas de las tres bases de datos.
    // La base temporal es opcional; si no se provee, se usa una en memoria (":memory:").
    bool initialize(const std::string& semanticDbPath,
                    const std::string& patternDbPath,
                    const std::string& temporalDbPath = "");

    // Cierra todas las conexiones y libera recursos.
    void shutdown();

    void setDebugMode(bool enable);
    void reprocessLastSentence();

    // Aprendizaje: procesa un texto completo (útil para cargar corpus inicial).
    void learnText(const std::string& text);

    // Procesa una oración completa: tokeniza, clasifica, guarda en BD semántica,
    // aprende patrones contextuales, y actualiza el estado interno de conversación.
    // Devuelve información detallada de cada palabra.
    std::vector<WordInfo> processSentence(const std::string& sentence);

    // Predicción de la(s) siguiente(s) palabra(s):
    // - Si se proporciona previousWords, se usa ese contexto.
    // - Si no, se usa el contexto interno (últimas palabras procesadas en la conversación).
    std::vector<Prediction> predictNext(const std::string& currentWord);

    // Genera una respuesta (hipótesis) a partir de una premisa dada.
    // Utiliza el historial de diálogos almacenado en la BD semántica.
    std::string generateResponse(const std::string& premise);

    // Corrección manual: cambia la clasificación de una palabra y actualiza
    // la base de datos y las estadísticas de transición.
    void correctWord(const std::string& word, const std::string& correctType);

    // Obtiene información actual de una palabra (sin modificar contexto).
    WordInfo getWordInfo(const std::string& word);

    // Reinicia el contexto conversacional (borra el historial de palabras recientes).
    void resetContext();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // ADMIN821_NLP_ENGINE_HPP
