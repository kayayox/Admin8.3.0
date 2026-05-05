/**==============================================================================
    Admin8.2.1 - Classifier.cpp
    Proposito: Implementación del clasificador.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "Classifier.hpp"
#include "Morphology.hpp"
#include "Refiner.hpp"
#include "TagStats.hpp"
#include "../db/WordRepository.hpp"
#include <iostream>

constexpr float HIGH_CONF_THRESHOLD = 0.8f;
constexpr float LOW_CONF_FOR_REEVAL = 0.2f;

Classifier::Classifier() {}

void Classifier::updateMorphAttributes(Word& w, TipoPalabra tag) {
    const std::string& pal = w.getPalabra();
    switch (tag) {
        case TipoPalabra::VERB:
            w.setTiempo(morphology::detectTense(pal));
            w.setPersona(morphology::detectPerson(pal));
            w.setCantidad(morphology::endsWith(pal, "n") || morphology::endsWith(pal, "mos") ? Cantidad::PLUR : Cantidad::SING);
            break;
        case TipoPalabra::SUST:
        case TipoPalabra::ADJT:
            w.setCantidad(morphology::isPlural(pal) ? Cantidad::PLUR : Cantidad::SING);
            w.setGenero(morphology::detectGender(pal));
            if (tag == TipoPalabra::ADJT)
                w.setGrado(morphology::detectAdjectiveDegree(pal));
            break;
        case TipoPalabra::ART:
            w.setCantidad(morphology::isPlural(pal) ? Cantidad::PLUR : Cantidad::SING);
            break;
        default:
            break;
    }
    w.generateStructuredMeaning();
}

void Classifier::classifySentence(std::vector<Word>& words) {
    if (words.empty()) return;

    // Paso 1: Clasificación inicial + carga desde BD
    for (auto& w : words) {
        WordRepository::load(w.getPalabra(), w); // carga atributos existentes

        if (w.getConfianza() >= HIGH_CONF_THRESHOLD)
            continue;

        TipoPalabra commonTag;
        float commonConf;
        if (morphology::isCommonWord(w.getPalabra(), commonTag, commonConf)) {
            w.setTipo(commonTag);
            w.setConfianza(commonConf);
            updateMorphAttributes(w, commonTag);
            WordRepository::save(w);
            continue;
        }

        TipoPalabra guessTag = morphology::guessInitialTag(w.getPalabra());
        float morphConf = morphology::validateTag(w.getPalabra(), guessTag);
        if (morphConf >= HIGH_CONF_THRESHOLD) {
            w.setTipo(guessTag);
            w.setConfianza(morphConf);
            updateMorphAttributes(w, guessTag);
            WordRepository::save(w);
        } else {
            w.setTipo(TipoPalabra::INDEFINIDO);
            w.setConfianza(LOW_CONF_FOR_REEVAL);
        }
    }

    // Paso 2: Refinamiento contextual iterativo
    for (size_t i = 0; i < words.size(); ++i) {
        Word& w = words[i];
        if (w.getConfianza() >= HIGH_CONF_THRESHOLD) continue;

        TipoPalabra prev2 = (i >= 2) ? words[i-2].getTipo() : TipoPalabra::INDEFINIDO;
        TipoPalabra prev  = (i >= 1) ? words[i-1].getTipo() : TipoPalabra::INDEFINIDO;
        TipoPalabra next  = (i+1 < words.size()) ? words[i+1].getTipo() : TipoPalabra::INDEFINIDO;

        TagConfidence refined = refineTag(prev2, prev, w.getTipo(), next, w.getConfianza());
        float morphScore = morphology::validateTag(w.getPalabra(), refined.tag);
        float combined = 0.6f * refined.confidence + 0.4f * morphScore;

        w.setTipo(refined.tag);
        w.setConfianza(std::min(1.0f, combined));
        updateMorphAttributes(w, refined.tag);
        WordRepository::save(w);
    }

    // Paso 3: Actualizar estadísticas de transición solo si toda la oración tiene alta confianza
    bool sentenceHighConf = true;
    for (const auto& w : words)
        if (w.getConfianza() < HIGH_CONF_THRESHOLD) { sentenceHighConf = false; break; }

    if (sentenceHighConf) {
        for (size_t i = 1; i < words.size(); ++i)
            TagStats::updateUnigram(words[i-1].getTipo(), words[i].getTipo());
        for (size_t i = 2; i < words.size(); ++i)
            TagStats::updateBigram(words[i-2].getTipo(), words[i-1].getTipo(), words[i].getTipo());
        for (size_t i = 0; i + 2 < words.size(); ++i)
            TagStats::updateTrigram(words[i].getTipo(), words[i+1].getTipo(), words[i+2].getTipo());
    }
}

void Classifier::updateConfidence(Word& word, bool wasCorrect) {
    float conf = word.getConfianza();
    if (wasCorrect) {
        conf += (1.0f - conf) * 0.3f;
    } else {
        conf *= 0.8f;
    }
    if (conf > 0.99f) conf = 0.99f;
    if (conf < 0.1f) conf = 0.1f;
    word.setConfianza(conf);
    WordRepository::save(word);
}
