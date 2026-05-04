/**==============================================================================
    Admin8.2.1 - Morphology.hpp
    Proposito: Funciones de análisis morfológico: detección de género, número,
               tiempo verbal, grado del adjetivo, y clasificación inicial por sufijos.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_MORPHOLOGY_HPP
#define ADMIN821_MORPHOLOGY_HPP

#include "../common/types.hpp"
#include <string>

namespace morphology {

    // Detección de propiedades
    bool isPlural(const std::string& palabra);
    Genero detectGender(const std::string& palabra);
    Tiempo detectTense(const std::string& palabra);
    Persona detectPerson(const std::string& palabra);
    Grado detectAdjectiveDegree(const std::string& palabra);

    // Clasificación inicial basada en sufijos y listas
    TipoPalabra guessInitialTag(const std::string& palabra);
    float getSuffixProb(const std::string& palabra, TipoPalabra tag);
    float validateTag(const std::string& palabra, TipoPalabra tag);

    // Palabras comunes (diccionario estático)
    bool isCommonWord(const std::string& word, TipoPalabra& outTag, float& outConf);

    // Reconocimiento de palabras funcionales
    bool isArticle(const std::string& palabra);
    bool isPreposition(const std::string& palabra);
    bool isConjunction(const std::string& palabra);
    bool isInterrogative(const std::string& palabra);
    bool isDemonstrative(const std::string& palabra);
    bool isNumeral(const std::string& palabra);
    bool isRelativePronoun(const std::string& palabra);
    bool isQuantifier(const std::string& palabra);

    // Utilidad
    bool endsWith(const std::string& palabra, const std::string& sufijo);
}

#endif // ADMIN821_MORPHOLOGY_HPP
