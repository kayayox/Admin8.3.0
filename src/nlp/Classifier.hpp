/**==============================================================================
    Admin8.2.1 - Classifier.hpp
    Proposito: Clasificador de palabras que combina morfología y contexto.
               Evalúa una oración completa, asigna etiquetas y actualiza
               estadísticas de transición.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_CLASSIFIER_HPP
#define ADMIN821_CLASSIFIER_HPP

#include "../core/Word.hpp"
#include <vector>

class Classifier {
public:
    Classifier();

    // Clasifica todas las palabras de una oración (modifica los objetos Word in situ)
    void classifySentence(std::vector<Word>& words);

    // Actualiza la confianza de una palabra basada en feedback
    void updateConfidence(Word& word, bool wasCorrect);

private:
    void updateMorphAttributes(Word& word, TipoPalabra tag);
};

#endif // ADMIN821_CLASSIFIER_HPP
