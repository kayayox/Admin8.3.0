#ifndef DIALOGUE_CONTEXT_HPP
#define DIALOGUE_CONTEXT_HPP

#include "../core/InferenceRule.hpp"
#include <vector>

// Forward declarations
class PatternCorrelator;
class ContextualCorrelator;
class ChunkCorrelator;
class TemplateMatcher;
class SlotFiller;

struct DialogueContext {
    PatternCorrelator* patternCorr = nullptr;
    ContextualCorrelator* ctxCorr = nullptr;
    ChunkCorrelator* chcCorr = nullptr;
    TemplateMatcher* templateMatcher = nullptr;
    SlotFiller* slotFiller = nullptr;

    std::vector<InferenceRule> inferenceRules;

    DialogueContext() = default;
};

#endif // DIALOGUE_CONTEXT_HPP
