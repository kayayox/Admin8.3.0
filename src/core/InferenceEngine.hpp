#ifndef INFERENCE_ENGINE_HPP
#define INFERENCE_ENGINE_HPP

#include "InferenceRule.hpp"
#include <string>
#include <vector>

// Forward declaration de ParsedPremise (definido en utils/SentenceUtils.hpp)
namespace utils {
    struct ParsedPremise;
}

std::string applyInferenceRules(const utils::ParsedPremise& parsed,
                                float creativity,
                                const std::vector<InferenceRule>& rules);

#endif // INFERENCE_ENGINE_HPP
