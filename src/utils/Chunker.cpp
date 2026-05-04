#include "Chunker.hpp"
#include "../common/types.hpp"
#include <sstream>

namespace {
    bool isNominal(TipoPalabra t) {
        return t == TipoPalabra::ART || t == TipoPalabra::SUST || t == TipoPalabra::ADJT ||
               t == TipoPalabra::NUM  || t == TipoPalabra::PRON || t == TipoPalabra::DEMS ||
               t == TipoPalabra::CUANT|| t == TipoPalabra::RELT;
    }

    bool isVerbal(TipoPalabra t) {
        return t == TipoPalabra::VERB || t == TipoPalabra::ADV;
    }

    bool isPreposition(TipoPalabra t) {
        return t == TipoPalabra::PREP;
    }

    bool isConjunction(TipoPalabra t) {
        return t == TipoPalabra::CONJ;
    }
}

std::vector<std::string> Chunker::chunk(const std::vector<Word>& words) {
    if (words.empty()) return {};

    std::vector<std::string> chunks;
    State state = State::START;
    std::string currentChunk;

    for (size_t i = 0; i < words.size(); ++i) {
        const Word& w = words[i];
        const std::string& token = w.getPalabra();
        TipoPalabra tag = w.getTipo();

        // Determinar nuevo estado en función del tag actual y del estado anterior
        State newState = state;
        bool finalizeChunk = false;

        switch (state) {
            case State::START:
                if (isNominal(tag)) newState = State::NOMINAL;
                else if (isVerbal(tag)) newState = State::VERBAL;
                else if (isPreposition(tag)) newState = State::PREPOSITIONAL;
                else if (isConjunction(tag)) newState = State::CONJUNCTION;
                //else newState = State::NOMINAL; // fallback
                break;

            case State::NOMINAL:
                if (isVerbal(tag) || isPreposition(tag) || isConjunction(tag)) {
                    finalizeChunk = true;
                    if (isVerbal(tag)) newState = State::VERBAL;
                    else if (isPreposition(tag)) newState = State::PREPOSITIONAL;
                    else if (isConjunction(tag)) newState = State::CONJUNCTION;
                } else if (isNominal(tag)) {
                    newState = State::NOMINAL; // seguir acumulando
                } else {
                    newState = State::NOMINAL; // por si hay otras etiquetas no contempladas
                }
                break;

            case State::VERBAL:
                if (isPreposition(tag) || isConjunction(tag) || (isNominal(tag) && tag != TipoPalabra::PRON)) {
                    finalizeChunk = true;
                    if (isPreposition(tag)) newState = State::PREPOSITIONAL;
                    else if (isConjunction(tag)) newState = State::CONJUNCTION;
                    else if (isNominal(tag)) newState = State::NOMINAL;
                } else if (isVerbal(tag) || tag == TipoPalabra::PRON) { // pronombres átonos se pegan al verbo
                    newState = State::VERBAL;
                } else {
                    // otros (p.ej. numeral, etc.) - los incluimos en el verbal por ahora
                    newState = State::VERBAL;
                }
                break;

            case State::PREPOSITIONAL:
                // La preposición se come todo hasta encontrar un verbo (no auxiliar) o fin
                if (isVerbal(tag) && tag == TipoPalabra::VERB) {
                    finalizeChunk = true;
                    newState = State::VERBAL;
                } else {
                    newState = State::PREPOSITIONAL; // sigue acumulando
                }
                break;

            case State::CONJUNCTION:
                // La conjunción suele ser su propio chunk excepto si es subordinante que encabeza una cláusula
                finalizeChunk = true;
                if (isNominal(tag)) newState = State::NOMINAL;
                else if (isVerbal(tag)) newState = State::VERBAL;
                else if (isPreposition(tag)) newState = State::PREPOSITIONAL;
                else newState = State::NOMINAL;
                break;
        }

        // Construir el chunk actual
        if (finalizeChunk && !currentChunk.empty()) {
            chunks.push_back(currentChunk);
            currentChunk.clear();
        }

        // Añadir la palabra al chunk actual (con espacio si no es el primero)
        if (currentChunk.empty()) currentChunk = token;
        else currentChunk += " " + token;

        state = newState;
    }

    // Último chunk
    if (!currentChunk.empty()) chunks.push_back(currentChunk);

    return chunks;
}
