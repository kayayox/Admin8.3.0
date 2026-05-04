/**==============================================================================
    Admin8.2.1 - Sentence.cpp
    Proposito: Implementación de la entidad Sentence.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: El constructor desde vector<Word> necesita la definición completa de Word.
==============================================================================*/

#include "Sentence.hpp"
#include "Word.hpp"

#include <algorithm>

Sentence::Sentence(const std::vector<Word>& words) {
    for (const auto& w : words) {
        Block b;
        b.text = w.getPalabra();
        b.type = w.getTipo();
        blocks_.push_back(b);

        // Lógica de clave: si es verbo, establecer tiempo y posible clave
        if (w.getTipo() == TipoPalabra::VERB && tense_ == Tiempo::INDETERMINADO) {
            tense_ = w.getTiempo();
            key_.text = w.getPalabra();
            key_.type = TipoPalabra::VERB;
            frequency_ = 1.0f;
        }
        // Si es sustantivo, también puede ser clave (sobrescribe si verbo no apareció)
        if (w.getTipo() == TipoPalabra::SUST) {
            key_.text = w.getPalabra();
            key_.type = TipoPalabra::SUST;
            frequency_ = 1.0f;
        }
    }
    // Si no se encontró sustantivo, usar el primer verbo como clave
    if (key_.type != TipoPalabra::SUST) {
        for (const auto& b : blocks_) {
            if (b.type == TipoPalabra::VERB) {
                key_ = b;
                frequency_ = 1.2f;
                break;
            }
        }
    }
}

std::vector<TipoPalabra> Sentence::getTypeSequence() const {
    std::vector<TipoPalabra> res;
    res.reserve(blocks_.size());
    for (const auto& b : blocks_) {
        res.push_back(b.type);
    }
    return res;
}

void Sentence::addBlock(const std::string& text, TipoPalabra type) {
    blocks_.push_back({text, type});
}

void Sentence::insertBlockAtStart(const std::string& text, TipoPalabra type) {
    blocks_.insert(blocks_.begin(), {text, type});
}

void Sentence::insertNegation() {
    for (size_t i = 0; i < blocks_.size(); ++i) {
        if (blocks_[i].type == TipoPalabra::VERB) {
            blocks_.insert(blocks_.begin() + i + 1, {"no", TipoPalabra::ADV});
            break;
        }
    }
}

void Sentence::replaceNoun(const std::string& newWord) {
    for (auto& b : blocks_) {
        if (b.type == TipoPalabra::SUST) {
            b.text = newWord;
            break;
        }
    }
}

std::string Sentence::toString() const {
    std::string res;
    for (const auto& b : blocks_) {
        if (!res.empty()) res += " ";
        res += b.text;
    }
    return res;
}
