/**==============================================================================
    Admin8.3.0 - Dialogue.cpp
    Proposito: Implementación de historial de diálogos y generación de hipótesis
               avanzada usando reglas de inferencia y correladores contextuales.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "Dialogue.hpp"
#include "InferenceEngine.hpp"
#include "../utils/SentenceUtils.hpp"
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

using namespace utils;   // para parsePremise, buildSentenceFromText, etc.

// ----------------------------------------------------------------------------
// Funciones auxiliares privadas
// ----------------------------------------------------------------------------

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

// Generación de continuación palabra a palabra (usando ContextualCorrelator)
static std::string generateContinuation(const Sentence& premise,
                                        ContextualCorrelator& ctxCorr,
                                        float creativity,
                                        int maxWords = 5) {
    std::string text = premise.toString();
    if (text.empty()) return "";

    auto lastWord = [](const std::string& s) -> std::string {
        size_t lastSpace = s.rfind(' ');
        if (lastSpace == std::string::npos) return s;
        return s.substr(lastSpace + 1);
    };

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

        std::string next;
        if (creativity > 0.7f && outcomes.size() > 1) {
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

// Respuesta por defecto racional (fallback)
static std::string fallbackHypothesis(const ParsedPremise& parsed, float creativity) {
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
    // 1. Analizar la premisa usando el parser de utils
    ParsedPremise parsed = parsePremise(premise);

    // 2. Determinar tipo de patrón objetivo
    TipoPatron targetType = (pattern) ? pattern->type : parsed.patternType;
    if (creativity > 0.6f) {
        int r = rand() % 3;
        if (r == 0 && targetType != TipoPatron::PREGUNTA_SIMP)
            targetType = TipoPatron::PREGUNTA_SIMP;
        else if (r == 1 && targetType != TipoPatron::NEGACION_SIMP)
            targetType = TipoPatron::NEGACION_SIMP;
    }

    std::string generatedText;

    // 3. INTENTO 1: Reglas de inferencia (conocimiento fuerte)
    if (!ctx.inferenceRules.empty()) {
        generatedText = applyInferenceRules(parsed, creativity, ctx.inferenceRules);
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

    // 5. INTENTO 3: Correlación contextual (generar continuación)
    if (ctx.ctxCorr && creativity > 0.4f) {
        std::string seed;
        if (!parsed.subject.empty() && !parsed.verb.empty()) {
            seed = parsed.subject + " " + parsed.verb;
        } else {
            seed = premise.toString();
        }
        if (seed.length() > 20) seed = seed.substr(0, 20);
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
