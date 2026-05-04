#include "ResponseTemplates.hpp"
#include <algorithm>
#include <regex>
#include <sstream>

std::vector<std::string> ResponseTemplate::extractSlots(const std::string& tmpl) {
    std::vector<std::string> slots;
    std::regex re(R"(\{(\w+)\})");
    std::sregex_iterator it(tmpl.begin(), tmpl.end(), re);
    std::sregex_iterator end;
    for (; it != end; ++it) {
        slots.push_back((*it)[1].str());
    }
    return slots;
}

void TemplateMatcher::registerTemplate(const ResponseTemplate& tmpl) {
    templates_.push_back(tmpl);
}

const ResponseTemplate* TemplateMatcher::matchTemplate(TipoPatron inputPattern,
                                                       const std::vector<std::string>& keywords) const {
    std::vector<const ResponseTemplate*> candidates;
    for (const auto& tmpl : templates_) {
        if (tmpl.patternType == inputPattern || tmpl.patternType == TipoPatron::SENTENCIAS) {
            candidates.push_back(&tmpl);
        }
    }
    if (candidates.empty()) return nullptr;

    // Ordenar por prioridad descendente y luego por coincidencia de keywords
    std::sort(candidates.begin(), candidates.end(),
        [&](const ResponseTemplate* a, const ResponseTemplate* b) {
            if (a->priority != b->priority) return a->priority > b->priority;
            // Calcular score de keywords: cuántas de las keywords están en contextKeywords
            int scoreA = 0, scoreB = 0;
            for (const auto& kw : keywords) {
                for (const auto& ckw : a->contextKeywords) {
                    if (kw.find(ckw) != std::string::npos || ckw.find(kw) != std::string::npos)
                        scoreA++;
                }
                for (const auto& ckw : b->contextKeywords) {
                    if (kw.find(ckw) != std::string::npos || ckw.find(kw) != std::string::npos)
                        scoreB++;
                }
            }
            return scoreA > scoreB;
        });
    return candidates.front();
}

std::string TemplateMatcher::fillTemplate(const ResponseTemplate& tmpl,
                                          const std::unordered_map<std::string, std::string>& slotValues) const {
    std::string result = tmpl.templateStr;
    for (const auto& slot : tmpl.slots) {
        std::string placeholder = "{" + slot + "}";
        auto it = slotValues.find(slot);
        std::string replacement = (it != slotValues.end()) ? it->second : "";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), replacement);
            pos += replacement.length();
        }
    }
    // Limpiar placeholders no reemplazados (dejar vacío)
    std::regex re(R"(\{\w+\})");
    result = std::regex_replace(result, re, "");
    // Limpiar espacios múltiples
    result = std::regex_replace(result, std::regex(" +"), " ");
    // Recortar espacios inicial/final
    result = std::regex_replace(result, std::regex("^ +| +$"), "");
    return result;
}

void TemplateMatcher::loadDefaultTemplates() {
    // Afirmaciones simples y compuestas
    registerTemplate({-1, TipoPatron::AFIRMACION_SIMP, {}, "{sujeto} {verbo} {objeto}", 10, {}});
    registerTemplate({-1, TipoPatron::AFIRMACION_SIMP, {}, "{sujeto} {verbo}", 9, {}});
    registerTemplate({-1, TipoPatron::AFIRMACION_COMP, {}, "{sujeto} {verbo} {complemento}", 8, {}});

    // Negaciones
    registerTemplate({-1, TipoPatron::NEGACION_SIMP, {}, "{sujeto} no {verbo} {objeto}", 10, {"no", "nunca"}});
    registerTemplate({-1, TipoPatron::NEGACION_COMP, {}, "Aunque {sujeto} {verbo} {objeto}, no estoy seguro", 7, {}});

    // Preguntas
    registerTemplate({-1, TipoPatron::PREGUNTA_SIMP, {}, "¿{verbo} {sujeto}?", 10, {"?", "qué", "cómo"}});
    registerTemplate({-1, TipoPatron::PREGUNTA_COMP, {}, "¿{verbo} {sujeto} {objeto}?", 8, {}});

    // Templates más naturales
    registerTemplate({-1, TipoPatron::SENTENCIAS, {}, "Aún no estoy seguro de {algo}", 5, {"seguro", "duda"}});
    registerTemplate({-1, TipoPatron::SENTENCIAS, {}, "Háblame más acerca de {articulo} {sujeto}", 4, {"habla", "cuéntame"}});
    registerTemplate({-1, TipoPatron::SENTENCIAS, {}, "¿Podrías explicar {articulo} {objeto}?", 6, {"explica", "por qué"}});
    registerTemplate({-1, TipoPatron::AFIRMACION_SIMP, {}, "Entonces {sujeto} {verbo} {objeto}", 7, {"entonces"}});

    // Para cada template, extraemos slots automáticamente
    for (auto& tmpl : templates_) {
        tmpl.slots = ResponseTemplate::extractSlots(tmpl.templateStr);
    }
}
