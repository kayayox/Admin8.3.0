/**==============================================================================
    Admin8.2.1 - Sentence.hpp
    Proposito: Entidad de dominio que representa una oración como secuencia de
               bloques (palabras o tokens) con sus tipos gramaticales.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Reemplaza la antigua clase Oracion. Sin dependencias de persistencia.
           La construcción desde vector<Word> requiere forward declaration de Word.
==============================================================================*/

#ifndef ADMIN821_SENTENCE_HPP
#define ADMIN821_SENTENCE_HPP

#include "../common/types.hpp"
#include <string>
#include <vector>

// Forward declaration para evitar include circular
class Word;

struct Block {
    std::string text;
    TipoPalabra type;
};

class Sentence {
public:
    Sentence() = default;
    explicit Sentence(const std::vector<Word>& words);  // construye desde palabras clasificadas

    // Getters
    int getId() const { return id_; }
    const std::vector<Block>& getBlocks() const { return blocks_; }
    Tiempo getTense() const { return tense_; }
    const Block& getKey() const { return key_; }
    float getFrequency() const { return frequency_; }
    int getNumBlocks() const { return static_cast<int>(blocks_.size()); }
    std::vector<TipoPalabra> getTypeSequence() const;
    std::string toString() const;

    // Setters (solo modificación en memoria)
    void setId(int id) { id_ = id; }
    void setKey(const Block& key) { key_ = key; }
    void setFrequency(float freq) { frequency_ = freq; }
    void setTense(Tiempo tense) { tense_ = tense; }

    // Modificadores para generación de hipótesis
    void addBlock(const std::string& text, TipoPalabra type);
    void insertBlockAtStart(const std::string& text, TipoPalabra type);
    void insertNegation();  // inserta "no" después del primer verbo
    void replaceNoun(const std::string& newWord);  // reemplaza el primer sustantivo

private:
    int id_ = -1;
    std::vector<Block> blocks_;
    Tiempo tense_ = Tiempo::INDETERMINADO;
    Block key_;      // bloque clave (sustantivo o verbo principal)
    float frequency_ = 1.0f;
};

#endif // ADMIN821_SENTENCE_HPP
