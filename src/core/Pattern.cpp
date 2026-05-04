/**==============================================================================
    Admin8.2.1 - Pattern.cpp
    Proposito: Implementación de utilidades para patrones gramaticales.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Clasificación basada en presencia de negación/pregunta y longitud.
==============================================================================*/

#include "Pattern.hpp"

TipoPatron classifySentencePattern(const std::vector<TipoPalabra>& sequence) {
    bool hasNegation = false;
    bool hasQuestion = false;
    for (TipoPalabra t : sequence) {
        // Asumimos que 'no', 'nunca', etc. son adverbios (ADV)
        // En un sistema más preciso se necesitaría la palabra real,no es el caso,no debe ser presiso.
        // Por simplicidad, si hay adverbio se considera posible negación.
        if (t == TipoPalabra::ADV) {
            hasNegation = true;
        }
        if (t == TipoPalabra::PREG) {
            hasQuestion = true;
        }
    }
    if (hasQuestion && sequence.size() > 3) return TipoPatron::PREGUNTA_COMP;
    if (hasQuestion) return TipoPatron::PREGUNTA_SIMP;
    if (hasNegation && sequence.size() > 3) return TipoPatron::NEGACION_COMP;
    if (hasNegation) return TipoPatron::NEGACION_SIMP;
    if (sequence.size() > 3) return TipoPatron::AFIRMACION_COMP;
    return TipoPatron::AFIRMACION_SIMP;
}

Pattern patternFromSequence(const std::vector<TipoPalabra>& sequence) {
    return Pattern(sequence, classifySentencePattern(sequence));
}
