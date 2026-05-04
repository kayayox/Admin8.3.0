#ifndef RESPONSE_TEMPLATES_HPP
#define RESPONSE_TEMPLATES_HPP

#include "../core/Pattern.hpp"    // para TipoPatron
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

struct ResponseTemplate {
    int id = -1;
    TipoPatron patternType;
    std::vector<std::string> slots;          // extraídos automáticamente
    std::string templateStr;
    int priority = 0;
    std::vector<std::string> contextKeywords; // palabras clave que activan este template

    // Extraer slots de templateStr (palabras entre {})
    static std::vector<std::string> extractSlots(const std::string& tmpl);
};

class TemplateMatcher {
public:
    // Registrar un template (en memoria)
    void registerTemplate(const ResponseTemplate& tmpl);
    // Buscar el mejor template según patrón de entrada y palabras clave de la premisa
    const ResponseTemplate* matchTemplate(TipoPatron inputPattern,
                                          const std::vector<std::string>& keywords) const;
    // Rellenar el template con valores concretos
    std::string fillTemplate(const ResponseTemplate& tmpl,
                             const std::unordered_map<std::string, std::string>& slotValues) const;
    // Cargar templates predefinidos (para arranque)
    void loadDefaultTemplates();

    // Acceso a todos
    const std::vector<ResponseTemplate>& getAll() const { return templates_; }

private:
    std::vector<ResponseTemplate> templates_;
};

#endif
