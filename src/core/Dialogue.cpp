/**==============================================================================
    Admin8.2.4 - Dialogue.cpp
    Proposito: Implementación de historial de diálogos y generación de hipótesis
               avanzada usando reglas de inferencia y correladores contextuales.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

// ===== Dialogue.cpp (versión modificada) =====

#include "Dialogue.hpp"
#include "../dialogue/PatternCorrelator.hpp"
#include "../dialogue/ContextualCorrelator.hpp"
#include "../utils/SlotFiller.hpp"
#include "../utils/ResponseTemplates.hpp"
#include "../db/WordRepository.hpp"
#include "../nlp/Tokenizer.hpp"
#include "../core/Command.hpp"
#include <algorithm>
#include <regex>
#include <unordered_map>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <memory>
#include <iostream>

// ----------------------------------------------------------------------------
// Inicialización única del generador aleatorio
// ----------------------------------------------------------------------------
static const bool s_randInit = []() -> bool {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    return true;
}();

// Estructura de premisa analizada
struct ParsedPremise {
    std::string subject = "";
    std::string verb = "";
    std::string object = "";
    std::vector<std::string> keywords;
    TipoPatron patternType = TipoPatron::SENTENCIAS;
};

// ----------------------------------------------------------------------------
// Funciones auxiliares privadas
// ----------------------------------------------------------------------------
namespace {

// Extracción mejorada de sujeto, verbo, objeto
ParsedPremise parsePremise(const Sentence& premise) {
    ParsedPremise result;
    const auto& blocks = premise.getBlocks();
    bool afterPrep = false;

    // 1. Sujeto tras preposición "a"
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].type == TipoPalabra::PREP /*&& blocks[i].text == "a"*/) {
            afterPrep = true;
            continue;
        }
        if (afterPrep && (blocks[i].type == TipoPalabra::SUST || blocks[i].type == TipoPalabra::PRON)) {
            result.subject = blocks[i].text;
            afterPrep = false;
            break;
        }
    }
    // 2. Sujeto general (primer sustantivo si no se encontró antes)
    if (result.subject.empty()) {
        for (const auto& b : blocks) {
            if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON) {
                result.subject = b.text;
                break;
            }
        }
    }

    // 3. Verbo (el primero)
    for (const auto& b : blocks) {
        if (b.type == TipoPalabra::VERB) {
            result.verb = b.text;
            break;
        }
    }

    // 4. Objeto (primer sustantivo después del verbo)
    bool verbPassed = false;
    for (const auto& b : blocks) {
        if (!verbPassed && b.type == TipoPalabra::VERB) {
            verbPassed = true;
            continue;
        }
        if (verbPassed && (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON)) {
            result.object = b.text;
            break;
        }
    }

    // 5. Palabras clave (sustantivos + adjetivos)
    for (const auto& b : blocks) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON || b.type == TipoPalabra::ADJT) {
            result.keywords.push_back(b.text);
        }
    }

    result.patternType = classifySentencePattern(premise.getTypeSequence());
    return result;
}

// Aplica reglas de inferencia
std::string applyInferenceRules(const ParsedPremise& parsed,
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

// Generación a partir de plantillas sofisticadas
static std::string generateFromTemplate(const Sentence& originalSentence,
                                        const ParsedPremise& parsed,
                                        DialogueContext* ctx,
                                        TipoPatron targetPattern,
                                        float creativity) {
    if (!ctx || !ctx->templateMatcher || !ctx->slotFiller) return "";

    // Inyectar el contexto semántico de la premisa
    ctx->slotFiller->setPremiseContext(parsed.subject, parsed.verb, parsed.object);

    const ResponseTemplate* tmpl = ctx->templateMatcher->matchTemplate(targetPattern, parsed.keywords);
    if (!tmpl) {
        ctx->slotFiller->clearPremiseContext();
        return "";
    }

    std::vector<TipoPalabra> tagContext;
    std::vector<std::string> wordContext;
    for (const auto& block : originalSentence.getBlocks()) {
        tagContext.push_back(block.type);
        wordContext.push_back(block.text);
    }

    std::unordered_map<std::string, std::string> slotValues;
    for (const auto& slot : tmpl->slots) {
        std::string predicted = ctx->slotFiller->predictForSlot(slot, tagContext, wordContext);
        slotValues[slot] = predicted.empty() ? "" : predicted;
    }

    ctx->slotFiller->clearPremiseContext();
    return ctx->templateMatcher->fillTemplate(*tmpl, slotValues);
}

// Generación de continuación palabra a palabra
std::string generateContinuation(const Sentence& premise,
                                 ContextualCorrelator& ctxCorr,
                                 float creativity,
                                 int maxWords = 5) {
    // Construir la cadena actual
    std::string text = premise.toString();
    if (text.empty()) return "";

    // Función auxiliar para obtener última palabra
    auto lastWord = [](const std::string& s) -> std::string {
        size_t lastSpace = s.rfind(' ');
        if (lastSpace == std::string::npos) return s;
        return s.substr(lastSpace + 1);
    };

    // Función auxiliar para obtener palabras anteriores
    auto previousWords = [](const std::string& s) -> std::vector<std::string> {
        std::vector<std::string> res;
        std::istringstream iss(s);
        std::string w;
        while (iss >> w) res.push_back(w);
        if (res.size() > 1) res.pop_back();
        return res;
    };

    std::string current = lastWord(text);
    for (int i = 0; i < maxWords; ++i) {
        std::vector<std::string> prev = previousWords(text);
        std::vector<std::pair<WordPattern, double>> outcomes;
        if (!ctxCorr.queryNext(current, prev, outcomes)) break;

        // Seleccionar la palabra siguiente
        std::string next;
        if (creativity > 0.7f && outcomes.size() > 1) {
            // elegir aleatoriamente entre las dos más probables
            int idx = (rand() % 2 == 0) ? 0 : 1;
            if (idx < (int)outcomes.size())
                next = outcomes[idx].first.begin()->first;
            else
                next = outcomes[0].first.begin()->first;
        } else {
            next = outcomes[0].first.begin()->first;
        }
        text += " " + next;
        current = next;
        if (next.find_first_of(".!?") != std::string::npos) break;
    }
    return text;
}

// Inyecta creatividad léxica y estilística
void applyCreativity(std::string& hypothesis, float creativity) {
    if (creativity < 0.3f) return;

    // Toque de duda para creatividad muy alta
    if (creativity > 0.8f &&
        hypothesis.find("no") == std::string::npos &&
        hypothesis.find('?') == std::string::npos) {
        hypothesis = "Quizás " + hypothesis;
    }

    // Sustitución aleatoria por sinónimos/relacionados
    if (creativity > 0.5f) {
        std::vector<std::string> words;
        std::istringstream iss(hypothesis);
        std::string w;
        while (iss >> w) words.push_back(w);

        for (auto& w : words) {
            Word wordObj;
            if (WordRepository::load(w, wordObj)) {
                const auto& rels = wordObj.getRelated();
                if (!rels.empty() && (rand() % 100) < static_cast<int>(creativity * 100)) {
                    w = rels[0].first;
                    break;  // solo un reemplazo por hipótesis
                }
            }
        }

        std::string rebuilt;
        for (const auto& word : words) {
            if (!rebuilt.empty()) rebuilt += " ";
            rebuilt += word;
        }
        hypothesis = rebuilt;
    }
}

} // namespace anónimo

// ----------------------------------------------------------------------------
// DialogueHistory
// ----------------------------------------------------------------------------
void DialogueHistory::addDialogue(const Sentence& premise,
                                  const Sentence& hypothesis,
                                  const Pattern& pattern,
                                  float creativity) {
    history_.push_back({premise, hypothesis, pattern, creativity});
    updateThreshold();
}

void DialogueHistory::updateThreshold() {
    if (history_.empty()) return;
    float sum = 0.0f;
    for (const auto& d : history_) sum += d.creativity;
    thresholdCreativity_ = sum / static_cast<float>(history_.size());
}
// ----------------------------------------------------------------------------
// Aplicar creatividad léxica y sintáctica avanzada
// ----------------------------------------------------------------------------
static void advancedCreativeTransform(std::string& text, float creativity,
                                      const ParsedPremise& premiseInfo) {
    if (creativity < 0.2f) return;

    // 1. Añadir muletilla al inicio si no existe y creatividad alta
    if (creativity > 0.7f && text.find("quizás") == std::string::npos &&
        text.find("tal vez") == std::string::npos && text.find("¿") != 0) {
        if (rand() % 100 < 40) {
            text = "Quizás " + text;
        }
    }

    // 2. Reemplazar palabras por sinónimos (usando WordRepository)
    if (creativity > 0.4f) {
        std::vector<std::string> words;
        std::istringstream iss(text);
        std::string w;
        while (iss >> w) words.push_back(w);

        for (auto& w : words) {
            Word wordObj;
            if (WordRepository::load(w, wordObj)) {
                const auto& rels = wordObj.getRelated();
                if (!rels.empty() && (rand() % 100) < static_cast<int>(creativity * 40)) {
                    // Elegir palabra relacionada con mayor peso aleatorio
                    int idx = rand() % rels.size();
                    w = rels[idx].first;
                    break;  // un solo cambio por hipótesis
                }
            }
        }

        std::string rebuilt;
        for (const auto& word : words) {
            if (!rebuilt.empty()) rebuilt += " ";
            rebuilt += word;
        }
        text = rebuilt;
    }

    // 3. Cambiar signos de puntuación: si es afirmación y creatividad alta, convertir a pregunta
    if (creativity > 0.8f && text.find('?') == std::string::npos &&
        text.find("no") == std::string::npos && text.back() != '?') {
        text = "¿" + text + "?";
    } else if (text.back() != '.' && text.back() != '?' && text.back() != '!') {
        text += ".";
    }

    // 4. Capitalizar primera letra
    if (!text.empty()) {
        text[0] = std::toupper(text[0]);
    }
}

// ----------------------------------------------------------------------------
// Construir Sentence desde texto con tipos gramaticales
// ----------------------------------------------------------------------------
Sentence buildSentenceFromText(const std::string& text) {
    auto tokens = tokenize(text);
    std::vector<Word> words;
    for (const auto& tok : tokens) {
        Word w(tok.text);
        WordRepository::load(tok.text, w);  // carga tipo, etc. si existe
        words.push_back(w);
    }
    return Sentence(words);
}

// ----------------------------------------------------------------------------
// Respuesta por defecto racional
// ----------------------------------------------------------------------------
std::string fallbackHypothesis(const ParsedPremise& parsed, float creativity) {
    if (!parsed.verb.empty() && !parsed.subject.empty()) {
        std::string base = "Entiendo que " + parsed.subject + " " + parsed.verb;
        if (!parsed.object.empty()) base += " " + parsed.object;
        if (creativity > 0.5f) {
            base = "¿" + base + "?";
        } else {
            base += ".";
        }
        return base;
    } else if (!parsed.verb.empty()) {
        return "¿" + parsed.verb + "?";
    } else {
        return "No he comprendido la premisa. ¿Podrías reformularla?";
    }
}

// ----------------------------------------------------------------------------
// FUNCIÓN PRINCIPAL generateHypothesis
// ----------------------------------------------------------------------------
Sentence generateHypothesis(const Sentence& premise,
                            DialogueContext& ctx,
                            Pattern* pattern,
                            const std::string& keyword,
                            float creativity) {
    // 1. Analizar la premisa
    ParsedPremise parsed = parsePremise(premise);

    // 2. Determinar tipo de patrón objetivo
    TipoPatron targetType = (pattern) ? pattern->type : parsed.patternType;
    if (creativity > 0.6f) {
        // Transformar aleatoriamente el tipo para mayor variedad
        int r = rand() % 3;
        if (r == 0 && targetType != TipoPatron::PREGUNTA_SIMP)
            targetType = TipoPatron::PREGUNTA_SIMP;
        else if (r == 1 && targetType != TipoPatron::NEGACION_SIMP)
            targetType = TipoPatron::NEGACION_SIMP;
        // else mantener
    }

    std::string generatedText;

    // 3. INTENTO 1: Reglas de inferencia (conocimiento fuerte)
    if (!ctx.inferenceRules.empty()) {
        generatedText = applyInferenceRules(parsed, creativity, ctx.inferenceRules);
        // Si la regla da una pregunta o afirmación con alta confianza, la aceptamos
        if (!generatedText.empty()) {
            advancedCreativeTransform(generatedText, creativity, parsed);
            return buildSentenceFromText(generatedText);
        }
    }

    // 4. INTENTO 2: Plantillas semánticas (con SlotFiller mejorado)
    if (ctx.templateMatcher && ctx.slotFiller) {
        generatedText = generateFromTemplate(premise, parsed, &ctx, targetType, creativity);
        if (!generatedText.empty()) {
            advancedCreativeTransform(generatedText, creativity, parsed);
            return buildSentenceFromText(generatedText);
        }
    }

    // 5. INTENTO 3: Correlación contextual (generar continuación a partir de semilla)
    if (ctx.ctxCorr && creativity > 0.4f) {
        // Construir una semilla inicial: puede ser "Entonces", "Quizás", o parte de la premisa
        std::string seed;
        if (!parsed.subject.empty() && !parsed.verb.empty()) {
            seed = parsed.subject + " " + parsed.verb;
        } else {
            seed = premise.toString();
        }
        // Limitar longitud de semilla
        if (seed.length() > 20) seed = seed.substr(0, 20);
        // Generar continuación
        generatedText = generateContinuation(buildSentenceFromText(seed), *ctx.ctxCorr, creativity, 6);
        if (!generatedText.empty()) {
            advancedCreativeTransform(generatedText, creativity, parsed);
            return buildSentenceFromText(generatedText);
        }
    }

    // 6. ÚLTIMO RECURSO: respuesta por defecto
    generatedText = fallbackHypothesis(parsed, creativity);
    advancedCreativeTransform(generatedText, creativity, parsed);
    return buildSentenceFromText(generatedText);
}

// ----------------------------------------------------------------------------
// computeCreativity
// ----------------------------------------------------------------------------
float computeCreativity(const Sentence& premise,
                        const Sentence& hypothesis,
                        const Pattern& /*pattern*/) {
    std::vector<std::string> premiseWords, hypoWords;
    for (const auto& b : premise.getBlocks()) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::VERB)
            premiseWords.push_back(b.text);
    }
    for (const auto& b : hypothesis.getBlocks()) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::VERB)
            hypoWords.push_back(b.text);
    }

    int common = 0;
    for (const auto& pw : premiseWords) {
        if (std::find(hypoWords.begin(), hypoWords.end(), pw) != hypoWords.end())
            common++;
    }

    if (premiseWords.empty() && hypoWords.empty()) return 0.5f;
    float base = static_cast<float>(common) /
                 std::max(premiseWords.size(), hypoWords.size());

    int diff = static_cast<int>(premise.getBlocks().size()) -
               static_cast<int>(hypothesis.getBlocks().size());
    int maxSize = static_cast<int>(std::max(premise.getBlocks().size(),
                                            hypothesis.getBlocks().size()));
    float lenRatio = (maxSize == 0) ? 1.0f
                     : 1.0f - static_cast<float>(std::abs(diff)) / static_cast<float>(maxSize);

    return std::min(0.95f, base * 0.7f + lenRatio * 0.3f);
}

// ----------------------------------------------------------------------------
// Carga de reglas por defecto
// ----------------------------------------------------------------------------
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
