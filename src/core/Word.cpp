/**==============================================================================
    Admin8.2.2 - Word.cpp
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

void Word::learnRelationsFromCorrelator(PatternCorrelator& correlator, double minConfidence) {
    // Usamos la palabra actual como clave y un patrón previo vacío (sin contexto)
    WordPattern dummyPrev = {{"__NO_CONTEXT__", 1.0f}};
    std::vector<std::pair<WordPattern, double>> outcomes;
    if (correlator.query(palabra_, dummyPrev, outcomes)) {
        relacionadas_.clear();
        for (const auto& [pattern, conf] : outcomes) {
            if (conf >= minConfidence && !pattern.empty()) {
                // Cada patrón puede contener varias palabras; las tratamos todas como relacionadas
                for (const auto& [relWord, weight] : pattern) {
                    if (relWord != palabra_) {  // Evitar auto‑relación
                        relacionadas_.emplace_back(relWord, conf * weight);
                    }
                }
            }
        }
    }
}

void Word::generateStructuredMeaning() {
    std::string base = "La palabra \"" + palabra_ + "\" es ";
    std::string attrs;

    switch (tipo_) {
        case TipoPalabra::SUST:
            attrs = "un sustantivo";
            if (genero_ == Genero::MASC) attrs += " masculino";
            else if (genero_ == Genero::FEME) attrs += " femenino";
            if (cantidad_ == Cantidad::SING) attrs += " en singular";
            else if (cantidad_ == Cantidad::PLUR) attrs += " en plural";
            attrs += " que designa una entidad.";
            break;

        case TipoPalabra::VERB:
            attrs = "un verbo";
            if (tiempo_ != Tiempo::INDETERMINADO) {
                switch (tiempo_) {
                    case Tiempo::PASS: attrs += " en pasado"; break;
                    case Tiempo::PRES: attrs += " en presente"; break;
                    case Tiempo::FUTR: attrs += " en futuro"; break;
                    default: break;
                }
            }
            if (persona_ != Persona::NIN) {
                switch (persona_) {
                    case Persona::PRIM: attrs += " de primera persona"; break;
                    case Persona::SEGU: attrs += " de segunda persona"; break;
                    case Persona::TERC: attrs += " de tercera persona"; break;
                    default: break;
                }
            }
            attrs += " que expresa una acción, estado o proceso.";
            break;

        case TipoPalabra::ADJT:
            attrs = "un adjetivo calificativo";
            if (grado_ != Grado::NON) {
                switch (grado_) {
                    case Grado::POSIT: attrs += " en grado positivo"; break;
                    case Grado::COMPARA: attrs += " en grado comparativo"; break;
                    case Grado::SUPERLA: attrs += " en grado superlativo"; break;
                    default: break;
                }
            }
            attrs += " que modifica al sustantivo.";
            break;

        case TipoPalabra::ADV:
            attrs = "un adverbio";
            if (grado_ == Grado::INTENS) attrs += " de intensidad";
            else if (grado_ == Grado::NEGAT) attrs += " de negación";
            else attrs += " que modifica al verbo, adjetivo u otro adverbio.";
            break;

        case TipoPalabra::PREP:
            attrs = "una preposición que establece relaciones de dependencia entre palabras.";
            break;

        case TipoPalabra::CONJ:
            attrs = "una conjunción que une oraciones o términos.";
            break;

        case TipoPalabra::PRON:
            attrs = "un pronombre";
            if (persona_ != Persona::NIN) {
                switch (persona_) {
                    case Persona::PRIM: attrs += " de primera persona"; break;
                    case Persona::SEGU: attrs += " de segunda persona"; break;
                    case Persona::TERC: attrs += " de tercera persona"; break;
                    default: break;
                }
            }
            attrs += " que sustituye a un sustantivo.";
            break;

        case TipoPalabra::ART:
            attrs = "un artículo";
            if (genero_ == Genero::MASC) attrs += " masculino";
            else if (genero_ == Genero::FEME) attrs += " femenino";
            if (cantidad_ == Cantidad::SING) attrs += " en singular";
            else if (cantidad_ == Cantidad::PLUR) attrs += " en plural";
            attrs += " que determina al sustantivo.";
            break;

        case TipoPalabra::NUM:
            attrs = "un numeral que indica cantidad u orden.";
            break;

        case TipoPalabra::DEMS:
            attrs = "un demostrativo que señala distancia relativa.";
            break;

        case TipoPalabra::CUANT:
            attrs = "un cuantificador que expresa cantidad imprecisa.";
            break;

        case TipoPalabra::RELT:
            attrs = "un relativo que introduce una cláusula subordinada.";
            break;

        case TipoPalabra::PREG:
            attrs = "un interrogativo usado en preguntas directas o indirectas.";
            break;

        case TipoPalabra::SENS:
            attrs = "una palabra sensorial (onomatopeya o interjección).";
            break;

        case TipoPalabra::CONT:
            attrs = "un contenido textual sin clasificación específica.";
            break;

        case TipoPalabra::DATE:
            attrs = "una fecha o expresión temporal.";
            break;

        default:
            attrs = "una palabra de tipo no especificado.";
            break;
    }

    significado_ = base + attrs;

    // Si hay palabras relacionadas, se añade una nota al final
    if (!relacionadas_.empty()) {
        significado_ += " Además, se relaciona con: ";
        size_t count = 0;
        for (const auto& [rel, conf] : relacionadas_) {
            if (count++ > 0) significado_ += ", ";
            significado_ += rel;
            if (count >= 5) { significado_ += ", etc."; break; }
        }
        significado_ += ".";
    }
}

void Word::addRelated(const std::string& palabra, double valor) {
    relacionadas_.emplace_back(palabra, valor);
}
