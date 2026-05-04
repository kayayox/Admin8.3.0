/**==============================================================================
    Admin8.2.1 - Pattern.hpp
    Proposito: Representa un patrón gramatical como una secuencia de tipos de
               palabra, junto con su clasificación (afirmación, negación, etc.).
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Sin dependencias de persistencia.
==============================================================================*/

#ifndef ADMIN821_PATTERN_HPP
#define ADMIN821_PATTERN_HPP

#include "../common/types.hpp"
#include <vector>

struct Pattern {
    std::vector<TipoPalabra> sequence;
    TipoPatron type;

    Pattern() : type(TipoPatron::SENTENCIAS) {}
    explicit Pattern(const std::vector<TipoPalabra>& seq, TipoPatron tp = TipoPatron::SENTENCIAS)
        : sequence(seq), type(tp) {}
};

// Funciones libres para trabajar con patrones
TipoPatron classifySentencePattern(const std::vector<TipoPalabra>& sequence);
Pattern patternFromSequence(const std::vector<TipoPalabra>& sequence);

#endif // ADMIN821_PATTERN_HPP
