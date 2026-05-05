/**==============================================================================
    Admin8.3.0 - types.hpp
    Proposito: Definiciones de tipos enumerados compartidos por todo el motor NLP
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Parte del motor NLP del proyecto Admin8.2.1
==============================================================================*/

#pragma once

#include <cstdint>

// Enumeraciones principales
enum class TipoPalabra : uint8_t {
    PRON, ART, ADJT, SUST, VERB, PREG, ADV, SENS, PREP, RELT, NUM, CONJ, CONT, CUANT, DEMS, DATE, INDEFINIDO
};

enum class Cantidad : uint8_t { SING, PLUR, NONE };
enum class Tiempo : uint8_t { PASS, PRES, FUTR, INDETERMINADO };
enum class Genero : uint8_t { MASC, FEME, NEUT };
enum class Grado : uint8_t { COMPARA, SUPERLA, POSIT, INTENS, INTERRG, NEGAT, RELAT, CUANTI, NON };
enum class Persona : uint8_t { PRIM, SEGU, TERC, NIN };

// Tipos de patron
enum class TipoPatron : uint8_t {
    AFIRMACION_SIMP, AFIRMACION_COMP, NEGACION_SIMP, NEGACION_COMP,
    PREGUNTA_SIMP, PREGUNTA_COMP, MIXTO, SENTENCIAS
};

// Tipos de token
enum class TokenType : uint8_t { WORD, NUMBER, DATE };

// Tipos de comando
enum class TipoCommand {
    DO, ANSWER, ASK, TASK, BID, GO, REPORT, MOVE, REPEAT, FIND,
    CREATE, DELETE, UPDATE, SEND, CALL, PLAY, PAUSE, STOP, START,
    SCHEDULE, SET, SHOW, HELP, OPEN, CLOSE, LEARN
};

enum class State { START, NOMINAL, VERBAL, PREPOSITIONAL, CONJUNCTION };
