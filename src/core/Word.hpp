/**==============================================================================
    Admin8.2.1 - Word.hpp
    Proposito: Entidad de dominio que representa una palabra con sus atributos
               gramaticales y semánticos. Sin métodos de persistencia.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Las operaciones de guardado/carga son responsabilidad de
           WordRepository (capa de persistencia).
==============================================================================*/

#ifndef ADMIN821_WORD_HPP
#define ADMIN821_WORD_HPP

#include "../common/types.hpp"
#include <string>
#include <vector>

class Word {
public:
    // Constructores
    Word() = default;
    explicit Word(const std::string& palabra);

    // Getters
    const std::string& getPalabra() const { return palabra_; }
    const std::string& getSignificado() const { return significado_; }
    TipoPalabra getTipo() const { return tipo_; }
    Cantidad getCantidad() const { return cantidad_; }
    Tiempo getTiempo() const { return tiempo_; }
    Genero getGenero() const { return genero_; }
    Grado getGrado() const { return grado_; }
    Persona getPersona() const { return persona_; }
    float getConfianza() const { return confianza_; }
    const std::vector<std::pair<std::string, double>>& getRelated() const { return relacionadas_; }

    // Setters (modifican la entidad, no persisten automáticamente)
    void setPalabra(const std::string& p) { palabra_ = p; }
    void setSignificado(const std::string& sig);
    void setTipo(TipoPalabra tipo);
    void setCantidad(Cantidad cant);
    void setTiempo(Tiempo tiempo);
    void setGenero(Genero gen);
    void setGrado(Grado grado);
    void setPersona(Persona pers);
    void setConfianza(float conf);

    // Generar significado por defecto basado en tipo y contexto opcional
    void generateDefaultMeaning(const std::string& contexto = "");

    // Relacionadas (solo gestión en memoria; la persistencia la maneja el repositorio)
    void addRelated(const std::string& palabra, double valor);
    void clearRelated() { relacionadas_.clear(); }

private:
    std::string palabra_;
    std::string significado_;
    TipoPalabra tipo_ = TipoPalabra::INDEFINIDO;
    Cantidad cantidad_ = Cantidad::NONE;
    Tiempo tiempo_ = Tiempo::INDETERMINADO;
    Genero genero_ = Genero::NEUT;
    Grado grado_ = Grado::NON;
    Persona persona_ = Persona::NIN;
    float confianza_ = 0.0f;
    std::vector<std::pair<std::string, double>> relacionadas_;
};

#endif // ADMIN821_WORD_HPP
