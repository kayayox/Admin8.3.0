/**==============================================================================
    Admin8.2.1 - Word.cpp
    Proposito: Implementación de la entidad Word.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Sin dependencias de persistencia.
==============================================================================*/

#include "Word.hpp"

Word::Word(const std::string& palabra) : palabra_(palabra) {}

void Word::setSignificado(const std::string& sig) {
    significado_ = sig;
}

void Word::setTipo(TipoPalabra tipo) {
    tipo_ = tipo;
}

void Word::setCantidad(Cantidad cant) {
    cantidad_ = cant;
}

void Word::setTiempo(Tiempo tiempo) {
    tiempo_ = tiempo;
}

void Word::setGenero(Genero gen) {
    genero_ = gen;
}

void Word::setGrado(Grado grado) {
    grado_ = grado;
}

void Word::setPersona(Persona pers) {
    persona_ = pers;
}

void Word::setConfianza(float conf) {
    confianza_ = conf;
}

void Word::generateDefaultMeaning(const std::string& contexto) {
    // Simplificación: usar conversión a string del tipo
    // En un sistema real se podría hacer más elaborado.
    if (!contexto.empty()) {
        significado_ = "Palabra clasificada automáticamente en contexto: \"" + contexto + "\"";
    } else {
        significado_ = "Palabra clasificada automáticamente.";
    }
}

void Word::addRelated(const std::string& palabra, double valor) {
    relacionadas_.emplace_back(palabra, valor);
}
