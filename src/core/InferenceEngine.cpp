#include "../common/DialogueContext.hpp"
#include "../utils/SentenceUtils.hpp" // para utils::ParsedPremise
#include <algorithm>
#include <string>

std::string applyInferenceRules(const utils::ParsedPremise& parsed,
                                float creativity,
                                const std::vector<InferenceRule>& rules) {
    for (const auto& rule : rules) {
        // Comprobación de verbo
        bool verbMatch = !rule.triggerVerbs.empty() &&
                         std::find(rule.triggerVerbs.begin(), rule.triggerVerbs.end(),
                                   parsed.verb) != rule.triggerVerbs.end();

        // Comprobación de sustantivo (sujeto u objeto)
        bool nounMatch = false;
        if (!rule.triggerNouns.empty()) {
            nounMatch = std::find(rule.triggerNouns.begin(), rule.triggerNouns.end(),
                                  parsed.subject) != rule.triggerNouns.end();
            if (!nounMatch && !parsed.object.empty()) {
                nounMatch = std::find(rule.triggerNouns.begin(), rule.triggerNouns.end(),
                                      parsed.object) != rule.triggerNouns.end();
            }
        }

        if ((verbMatch || nounMatch) && rule.confidence >= (1.0f - creativity)) {
            std::string result = rule.consequentTemplate;

            // Reemplazar slots con los valores mapeados (estáticos o dinámicos)
            for (const auto& mapping : rule.slotMappings) {
                std::string placeholder = "{" + mapping.first + "}";
                size_t pos = result.find(placeholder);
                if (pos == std::string::npos) continue;

                std::string value = mapping.second;

                // Resolución de referencias dinámicas
                if (value == "$subject") {
                    value = parsed.subject.empty() ? mapping.first : parsed.subject;
                } else if (value == "$verb") {
                    value = parsed.verb.empty() ? mapping.first : parsed.verb;
                } else if (value == "$object") {
                    value = parsed.object.empty() ? mapping.first : parsed.object;
                }

                result.replace(pos, placeholder.length(), value);
            }

            if (rule.isQuestion && result.find('?') == std::string::npos) {
                result = "¿" + result + "?";
            }
            return result;
        }
    }
    return "";
}

void loadDefaultInferenceRules(DialogueContext& ctx) {
    ctx.inferenceRules.push_back({
        {"llover", "llueve", "lloviendo"},
        {"lluvia"},
        "el suelo estará {adjetivo}",
        {{"adjetivo", "mojado"}},
        0.9f,
        false
    });

    ctx.inferenceRules.push_back({
        {"comprar", "compró", "compraste"},
        {"coche", "auto", "vehículo"},
        "{sujeto} tiene un {objeto}",
        {{"sujeto", "$subject"}, {"objeto", "$object"}},
        0.8f,
        false
    });

    ctx.inferenceRules.push_back({
        {"gusta", "encanta", "apetece"},
        {"pizza", "helado", "chocolate"},
        "¿Te gusta {objeto} con {ingrediente}?",
        {{"objeto", "$object"}, {"ingrediente", "queso"}},
        0.7f,
        true
    });

    ctx.inferenceRules.push_back({
        {"duerme", "dormir", "durmiendo"},
        {"gato", "perro", "bebé"},
        "{sujeto} está descansando",
        {{"sujeto", "$subject"}},
        0.85f,
        false
    });
}
