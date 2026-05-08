#ifndef INFERENCE_RULE_HPP
#define INFERENCE_RULE_HPP

#include <string>
#include <vector>

struct InferenceRule {
    std::vector<std::string> triggerVerbs;        // formas base que activan la regla
    std::vector<std::string> triggerNouns;        // sustantivos que activan la regla
    std::string consequentTemplate;               // plantilla con slots {nombre}
    std::vector<std::pair<std::string, std::string>> slotMappings;
    float confidence;                             // [0..1]
    bool isQuestion;                              // ¿genera pregunta?
};

#endif // INFERENCE_RULE_HPP
