/**==============================================================================
    Admin8.2.1 - StringConversions.hpp
    Proposito: Conversión de tipos enumerados a cadenas legibles.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_STRING_CONVERSIONS_HPP
#define ADMIN821_STRING_CONVERSIONS_HPP

#include "../common/types.hpp"
#include <string>

std::string tipoToString(TipoPalabra tipo);
std::string tiempoToString(Tiempo tiempo);
std::string generoToString(Genero genero);
std::string personaToString(Persona persona);
std::string gradoToString(Grado grado);
std::string cantidadToString(Cantidad cant);
std::string tipoPatronToString(TipoPatron tp);

#endif
