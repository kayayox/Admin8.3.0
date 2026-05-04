#ifndef CHUNKER_HPP
#define CHUNKER_HPP

#include "../core/Word.hpp"
#include <vector>
#include <string>

class Chunker {
public:
    // Convierte una secuencia de palabras clasificadas en una secuencia de frases (chunks)
    static std::vector<std::string> chunk(const std::vector<Word>& words);
};

#endif
