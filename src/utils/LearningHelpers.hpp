/**==============================================================================
    Admin8.2.1 - LearningHelpers.hpp
    Proposito: Funciones auxiliares para aprendizaje de texto en correladores.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_LEARNING_HELPERS_HPP
#define ADMIN821_LEARNING_HELPERS_HPP

#include "../dialogue/ContextualCorrelator.hpp"
#include "../dialogue/PatternCorrelator.hpp"
#include <string>

// ============================================================================
// Funciones públicas auxiliares
// ============================================================================

#include "../core/Word.hpp"


// Vector "Word" no clasificado ni cargado de db inicializados por defecto
bool createWordVector(std::vector<Word> &words, const std::string &input);
std::vector<Word> createWordVector(const std::string &input);

// Aprende usando contexto de hasta dos palabras anteriores
void learnContextual(ContextualCorrelator& ctx, const std::string& text);

// Aprende asociaciones directas (sin contexto)
void learnDirect(ContextualCorrelator& ctx, const std::string& text);

void learnTextWithContext(ContextualCorrelator& ctx, PatternCorrelator& corr, const std::string& text);

// Limpiar buffer de entrada (utilidad de consola, quizás no necesaria en biblioteca)
void clearInputBuffer();

// Eliminar espacios al inicio y final de una línea
void trimString(std::string& line);

#endif
