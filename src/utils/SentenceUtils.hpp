#ifndef SENTENCE_UTILS_HPP
#define SENTENCE_UTILS_HPP

#include "../core/Pattern.hpp"   // para TipoPatron
#include "../core/Sentence.hpp"  // para Sentence, Block
#include <string>
#include <vector>

namespace utils {

// Estructura que representa una premisa analizada (sujeto, verbo, objeto, keywords)
struct ParsedPremise {
    std::string subject = "";
    std::string verb = "";
    std::string object = "";
    std::vector<std::string> keywords;
    TipoPatron patternType = TipoPatron::SENTENCIAS;
};

// Analiza una Sentence extrayendo sujeto, verbo, objeto y palabras clave
ParsedPremise parsePremise(const Sentence& premise);

// Construye una Sentence a partir de un texto, usando WordRepository para asignar tipos gramaticales
Sentence buildSentenceFromText(const std::string& text);

// Aplica transformaciones creativas básicas (sinónimos, etc.) sobre un string
void applyCreativity(std::string& text, float creativity);

// Transformación creativa avanzada (muletillas, puntuación, cambio de tipo de oración)
void advancedCreativeTransform(std::string& text, float creativity,
                               const ParsedPremise& premiseInfo);

// Calcula cuán creativa es una hipótesis respecto a la premisa original
float computeCreativity(const Sentence& premise, const Sentence& hypothesis,
                        const Pattern& pattern);

// Wrapper para applyCreativity que devuelve un nuevo string (no modifica el original)
std::string applyCreativityToText(const std::string& text, float creativity);

} // namespace utils

#endif // SENTENCE_UTILS_HPP
