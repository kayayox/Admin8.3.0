/**==============================================================================
    Admin8.2.1 - StringConversions.cpp
    Proposito: Implementación de conversiones a string.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "StringConversions.hpp"

std::string tipoToString(TipoPalabra tipo) {
    static const char* textos[] = {
        "Pronombre", "Artículo", "Adjetivo", "Sustantivo", "Verbo",
        "Pregunta", "Adverbio", "Sentencia", "Preposición",
        "Relativo", "Numeral", "Conjunción", "Contracción", "Cuantitativo",
        "Determinante", "Fecha", "Indefinido"
    };
    return textos[static_cast<int>(tipo)];
}

std::string tiempoToString(Tiempo tiempo) {
    static const char* textos[] = {"Pasado", "Presente", "Futuro", "Indeterminado"};
    return textos[static_cast<int>(tiempo)];
}

std::string generoToString(Genero genero) {
    static const char* textos[] = {"Masculino", "Femenino", "Neutro"};
    return textos[static_cast<int>(genero)];
}

std::string personaToString(Persona persona) {
    static const char* textos[] = {"Primera", "Segunda", "Tercera", "Ninguna"};
    return textos[static_cast<int>(persona)];
}

std::string gradoToString(Grado grado) {
    static const char* textos[] = {
        "Comparativo", "Superlativo", "Positivo", "Intensivo",
        "Interrogativo", "Negativo", "Relativo", "Cuantitativo", "Ninguno"
    };
    return textos[static_cast<int>(grado)];
}

std::string cantidadToString(Cantidad cant) {
    if (cant == Cantidad::SING) return "Singular";
    if (cant == Cantidad::PLUR) return "Plural";
    return "Ninguno";
}

std::string tipoPatronToString(TipoPatron tp) {
    static const char* textos[] = {
        "Afirmación simple", "Afirmación compuesta", "Negación simple", "Negación compuesta",
        "Pregunta simple", "Pregunta compuesta", "Mixto", "Sentencia"
    };
    return textos[static_cast<int>(tp)];
}
