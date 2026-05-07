/**==============================================================================
    Admin8.2.4 - Dialogue.hpp
    Proposito: Estructuras para representar un diálogo (premisa, hipótesis, patrón)
               e historial de diálogos. Funciones para generar hipótesis (versión
               avanzada con contexto semántico) y calcular creatividad.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_DIALOGUE_HPP
#define ADMIN821_DIALOGUE_HPP

#include "Sentence.hpp"
#include "Pattern.hpp"
#include <vector>
#include <string>

// ------------------------------------------------
// Forward declarations
// ------------------------------------------------
class PatternCorrelator;
class ContextualCorrelator;
class ChunkCorrelator;
class TemplateMatcher;
class SlotFiller;

// ============================================================================
// Regla de inferencia
// ============================================================================
struct InferenceRule {
    std::vector<std::string> triggerVerbs;        // formas base que activan la regla
    std::vector<std::string> triggerNouns;        // sustantivos que activan la regla
    std::string consequentTemplate;               // plantilla con slots {nombre}
    std::vector<std::pair<std::string, std::string>> slotMappings;
    float confidence;                             // [0..1]
    bool isQuestion;                              // ¿genera pregunta?
};

// ============================================================================
// Contexto de diálogo – agrupa todos los recursos necesarios
// ============================================================================
struct DialogueContext {
    PatternCorrelator* patternCorr = nullptr;
    ContextualCorrelator* ctxCorr = nullptr;
    ChunkCorrelator* chcCorr = nullptr;
    TemplateMatcher* templateMatcher = nullptr;
    SlotFiller* slotFiller = nullptr;

    std::vector<InferenceRule> inferenceRules;

    DialogueContext() = default;
};

struct ParsedPremise;

// Funciones auxiliares públicas para generación
std::string applyCreativityToText(const std::string& text, float creativity);
std::string fallbackHypothesis(const ParsedPremise& parsed, float creativity);
Sentence buildSentenceFromText(const std::string& text);

// ============================================================================
// Historial de diálogos
// ============================================================================
struct Dialogue {
    Sentence premise;
    Sentence hypothesis;
    Pattern pattern;
    float creativity = 0.0f;
};

class DialogueHistory {
public:
    void addDialogue(const Sentence& premise, const Sentence& hypothesis,
                     const Pattern& pattern, float creativity);
    const std::vector<Dialogue>& getHistory() const { return history_; }
    float getThresholdCreativity() const { return thresholdCreativity_; }

private:
    std::vector<Dialogue> history_;
    float thresholdCreativity_ = 0.5f;
    void updateThreshold();
};

// ============================================================================
// Generación de hipótesis
// ============================================================================
Sentence generateHypothesis(const Sentence& premise,
                            DialogueContext& ctx,
                            Pattern* pattern = nullptr,
                            const std::string& keyword = "",
                            float creativity = 0.5f);

// ============================================================================
// Función auxiliar pública
// ============================================================================
float computeCreativity(const Sentence& premise, const Sentence& hypothesis,
                        const Pattern& pattern);

// Carga reglas de inferencia por defecto
void loadDefaultInferenceRules(DialogueContext& ctx);

#endif // ADMIN821_DIALOGUE_HPP
