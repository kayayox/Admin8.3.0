/**==============================================================================
    Admin8.2.1 - Tokenizer.hpp
    Proposito: Tokenización de texto en palabras/números/fechas, y segmentación en oraciones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_TOKENIZER_HPP
#define ADMIN821_TOKENIZER_HPP

#include "../common/types.hpp"
#include <string>
#include <vector>

struct Token {
    std::string text;
    TokenType type;
};

// Tokeniza una cadena de texto en una secuencia de tokens (palabras, números, fechas)
std::vector<Token> tokenize(const std::string& input);

// Divide un texto en oraciones (separando por puntuación seguida de mayúscula)
std::vector<std::string> splitIntoSentences(const std::string& input);

#endif // ADMIN821_TOKENIZER_HPP
