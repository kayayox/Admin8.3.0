/**==============================================================================
    Admin8.3.0 - NLPEngine.cpp
    Propósito: Implementación de la fachada principal del motor NLP.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "NLPEngine.hpp"

// Core
#include "../core/Word.hpp"
#include "../core/Sentence.hpp"
#include "../core/Pattern.hpp"
#include "../core/Command.hpp"
#include "../core/Dialogue.hpp"

// Database
#include "../db/DatabaseManager.hpp"
#include "../db/WordRepository.hpp"
#include "../db/SentenceRepository.hpp"
#include "../db/PatternRepository.hpp"
#include "../db/DialogueRepository.hpp"
#include "../db/TemplateRepository.hpp"

// NLP
#include "../nlp/Tokenizer.hpp"
#include "../nlp/Morphology.hpp"
#include "../nlp/TagStats.hpp"
#include "../nlp/Classifier.hpp"
#include "../nlp/Refiner.hpp"

// Dialogue
#include "../dialogue/PatternCorrelator.hpp"
#include "../dialogue/ContextualCorrelator.hpp"
#include "../dialogue/ChunkCorrelator.hpp"

// Utils
#include "../utils/StringConversions.hpp"
#include "../utils/LearningHelpers.hpp"
#include "../utils/ResponseTemplates.hpp"
#include "../utils/SlotFiller.hpp"
#include "../utils/SentenceUtils.hpp"

// STL
#include <deque>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace utils;
// ============================================================================
// Implementación interna (Pimpl)
// ============================================================================

class NLPEngine::Impl {
public:
    bool initialized = false;
    bool debugMode = false;
    std::deque<std::string> contextWords;        // hasta 15 palabras
    std::string lastProcessedSentenceText;
    Sentence lastProcessedSentence;

    std::unique_ptr<PatternCorrelator> patternCorrW;
    std::unique_ptr<PatternCorrelator> patternCorrC;
    std::unique_ptr<ContextualCorrelator> ctxCorr;
    std::unique_ptr<ChunkCorrelator> chcCorr;
    std::unique_ptr<TemplateMatcher> templateMatcher;
    std::unique_ptr<SlotFiller> slotFiller;
    Classifier classifier;

    DialogueContext dialogueContext;            // contexto de diálogo para hipótesis

    // Rutas de bases de datos
    std::string semanticDbPath;
    std::string patternDbPath;
    std::string temporalDbPath;

    // Última premisa y respuesta generada (para feedback)
    std::string lastPremiseText;
    std::string lastResponseText;

    // ------------------------------------------------------------------------
    // Inicialización
    // ------------------------------------------------------------------------
    bool initialize(const std::string& semPath, const std::string& patPath, const std::string& tempPath) {
        semanticDbPath = semPath;
        patternDbPath = patPath;
        temporalDbPath = tempPath.empty() ? ":memory:" : tempPath;

        // 1. Inicializar DatabaseManager con las tres rutas
        if (!DatabaseManager::instance().init(semanticDbPath)) {
            if (debugMode) std::cerr << "[ERROR] No se pudo abrir BD semántica: " << semanticDbPath << std::endl;
            return false;
        }
        if (!DatabaseManager::instance().init(patternDbPath)) {
            if (debugMode) std::cerr << "[ERROR] No se pudo abrir BD de patrones: " << patternDbPath << std::endl;
            return false;
        }
        if (!DatabaseManager::instance().init(temporalDbPath)) {
            if (debugMode) std::cerr << "[ERROR] No se pudo abrir BD temporal: " << temporalDbPath << std::endl;
            return false;
        }

        // 2. Configurar repositorios con sus respectivas rutas
        WordRepository::setDatabasePath(semanticDbPath);
        SentenceRepository::setDatabasePath(semanticDbPath);
        DialogueRepository::setDatabasePath(semanticDbPath);
        PatternRepository::setDatabasePath(patternDbPath);
        TagStats::setDatabasePath(patternDbPath);
        TemplateRepository::setDatabasePath(patternDbPath);

        // 3. Crear tablas si no existen
        WordRepository::initializeTables();
        SentenceRepository::initializeTables();
        TagStats::initializeTables();
        DialogueRepository::initializeTables();
        TemplateRepository::initializeTables();

        // 4. Cargar datos estáticos
        TagStats::loadDefaultFromStatic();
        TemplateRepository::loadDefaultIfEmpty();

        // 5. Inicializar correladores
        try {
            patternCorrW = std::make_unique<PatternCorrelator>(patternDbPath, "");
            patternCorrC = std::make_unique<PatternCorrelator>(patternDbPath, "_chunk");
            ctxCorr = std::make_unique<ContextualCorrelator>(patternDbPath);
            chcCorr = std::make_unique<ChunkCorrelator>(patternDbPath);
        } catch (const std::exception& e) {
            if (debugMode) std::cerr << "[ERROR] Correlators: " << e.what() << std::endl;
            return false;
        }
        PatternRepository::initializeTables();

        // 6. Inicializar plantillas y slot filler
        templateMatcher = std::make_unique<TemplateMatcher>();
        templateMatcher->loadDefaultTemplates();
        slotFiller = std::make_unique<SlotFiller>(*ctxCorr);

        // 7. Configurar el contexto de diálogo para generateHypothesis
        dialogueContext.patternCorr = patternCorrW.get();
        dialogueContext.ctxCorr = ctxCorr.get();
        dialogueContext.chcCorr = chcCorr.get();
        dialogueContext.templateMatcher = templateMatcher.get();
        dialogueContext.slotFiller = slotFiller.get();
        loadDefaultInferenceRules(dialogueContext);

        initialized = true;
        return true;
    }

    void shutdown() {
        if (initialized) {
            patternCorrW.reset();
            patternCorrC.reset();
            ctxCorr.reset();
            chcCorr.reset();
            templateMatcher.reset();
            slotFiller.reset();
            DatabaseManager::instance().closeAll();
            initialized = false;
        }
    }

    void setDebugMode(bool enable) { debugMode = enable; }

    // ------------------------------------------------------------------------
    // Procesamiento de oraciones
    // ------------------------------------------------------------------------
    std::vector<WordInfo> processSentence(const std::string& sentence) {
        if (!initialized || sentence.empty()) return {};

        std::vector<Word> words;
        if(!createWordVector(words, sentence)) return {};

        // 3. Clasificar todas las palabras
        classifier.classifySentence(words);

        // 4. Aprender la oración en los correladores
        learnTextWithContext(*ctxCorr, *patternCorrW, sentence);
        chcCorr->learnFromClassifiedSentence(words);
        std::vector<std::string> chunks = Chunker::chunk(words);
        chcCorr->learnNextChunkDirect(chunks);

        // 5. Construir Sentence y guardar en BD semántica
        Sentence sent(words);
        Pattern p = patternFromSequence(sent.getTypeSequence());
        PatternRepository::save(p);
        SentenceRepository::save(sent);
        lastProcessedSentence = sent;
        lastProcessedSentenceText = sentence;

        std::vector<std::string> wordStrings;
        for (const auto& w : words) wordStrings.push_back(w.getPalabra());

        for (auto& w : words) {
            w.learnRelationsFromCorrelator(*dialogueContext.patternCorr, wordStrings);
            WordRepository::save(w);
        }

        // 6. Actualizar contexto interno (hasta 15 palabras)
        for (const auto& w : words) {
            contextWords.push_back(w.getPalabra());
            if (contextWords.size() > 15) contextWords.pop_front();
        }

        // 7. Convertir a WordInfo para retorno
        std::vector<WordInfo> result;
        for (auto& w : words) {
            result.push_back(wordToInfo(w));
        }
        return result;
    }

    // ------------------------------------------------------------------------
    // Predicción de siguiente palabra
    // ------------------------------------------------------------------------
    std::vector<Prediction> predictNext(const std::string& currentWords) {
        if (!initialized) return {};

        std::vector<Word> words;
        std::vector<std::string> wordsCorrelator;
        std::stringstream ss(currentWords);
        std::string w;
        while (ss >> w) {
            wordsCorrelator.push_back(w);
            Word wo(w);
            WordRepository::load(w, wo);
            words.push_back(wo);
        }

        std::vector<std::string> chunks = Chunker::chunk(words);
        std::vector<std::pair<WordPattern, double>> outcomes;
        bool hasPrediction = false;

        if (chunks.size() >= 3) {
            std::string current = chunks.back();
            std::string prev1 = chunks[chunks.size() - 2];
            std::string prev2 = chunks[chunks.size() - 3];
            hasPrediction = chcCorr->queryNextWithTwoPrev(current, prev1, prev2, outcomes);
        } else if (chunks.size() == 2) {
            std::string current = chunks.back();
            std::string prev = chunks[0];
            hasPrediction = chcCorr->queryNextWithOnePrev(current, prev, outcomes);
        } else if (chunks.size() == 1) {
            std::string current = chunks.back();
            hasPrediction = chcCorr->queryNext(current, {"__NO_CONTEXT__"}, outcomes);
        }

        // Fallback a contexto de palabras
        if (!hasPrediction || outcomes.empty()) {
            std::string currWord;
            std::string prev;
            if (wordsCorrelator.size() > 1) {
                currWord = wordsCorrelator.back();
                prev = wordsCorrelator[wordsCorrelator.size() - 2];
            } else if (wordsCorrelator.size() == 1) {
                currWord = wordsCorrelator.back();
                prev = "__NO_CONTEXT__";
            }
            if (!currWord.empty()) {
                outcomes.clear();
                ctxCorr->queryNext(currWord, {prev}, outcomes);
            }
        }

        std::vector<Prediction> preds;
        for (const auto& p : outcomes) {
            if (!p.first.empty()) {
                Prediction pred;
                pred.word = p.first.begin()->first;
                pred.probability = p.second;
                preds.push_back(pred);
            }
        }
        return preds;
    }

    // ------------------------------------------------------------------------
    // Generación de respuesta (hipótesis)
    // ------------------------------------------------------------------------
    std::string generateResponse(const std::string& premise) {
        if (!initialized || premise.empty()) return "";

        // Tokenizar y clasificar la premisa
        std::vector<Word> words = createWordVector(premise);
        if(words.empty()) return "";
        classifier.classifySentence(words);
        Sentence premiseSent(words);

        // Guardar la premisa en BD (para poder referenciarla en el diálogo)
        SentenceRepository::save(premiseSent);

        Pattern p;
        p.sequence = premiseSent.getTypeSequence();
        p.type = classifySentencePattern(p.sequence);

        // Usar el historial para ajustar la creatividad
        DialogueHistory history = DialogueRepository::loadHistory();
        float creativity = history.getThresholdCreativity();

        // Inyectar algo de aleatoriedad para evitar monotonía
        static bool srandInit = false;
        if (!srandInit) {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            srandInit = true;
        }
        float randomFactor = (std::rand() % 100) / 100.0f * 0.1f;  // entre 0 y 0.1
        creativity = std::min(0.95f, creativity + randomFactor - 0.05f);

        Sentence hypothesis = generateHypothesis(premiseSent, dialogueContext, &p,
                                                  words.back().getPalabra(), creativity);

        std::string responseText = hypothesis.toString();

        // Guardar el diálogo (premisa + hipótesis) en la BD
        SentenceRepository::save(hypothesis);   // almacenar la hipótesis como oración
        DialogueRepository::saveDialogue(premiseSent, hypothesis, p, creativity);

        // Guardar última interacción para posible feedback
        lastPremiseText = premise;
        lastResponseText = responseText;

        return responseText;
    }

    // Proporcionar feedback sobre el diálogo
    void provideDialogueFeedback(bool positive) {
        if (!initialized || lastPremiseText.empty() || lastResponseText.empty()) return;

        Sentence premSent = buildSentenceFromText(lastPremiseText);
        Sentence respSent = buildSentenceFromText(lastResponseText);

        // Si no existen en BD, guardarlas (por si acaso)
        if (premSent.getId() <= 0) SentenceRepository::save(premSent);
        if (respSent.getId() <= 0) SentenceRepository::save(respSent);

        Pattern p = patternFromSequence(premSent.getTypeSequence());
        float creativity = computeCreativity(premSent, respSent, p);
        DialogueRepository::saveDialogue(premSent, respSent, p, creativity);

        // Registrar feedback a nivel de palabra (todas como correctas si positive, sino omitimos)
        if (positive) {
            for (const auto& block : respSent.getBlocks()) {
                DialogueRepository::registerFeedback(block.text, block.type, block.type, true);
            }
        }
        // Si el feedback es negativo podríamos hacer algo más sofisticado:
        //   - penalizar palabras, etc. Por ahora no se implementa.
    }

    // ------------------------------------------------------------------------
    // Corrección y reprocesamiento
    // ------------------------------------------------------------------------
    void correctWord(const std::string& word, const std::string& correctType) {
        if (!initialized) return;
        Word w(word);
        if (!WordRepository::load(word, w)) return;

        TipoPalabra newType = TipoPalabra::INDEFINIDO;
        if (correctType == "Sustantivo") newType = TipoPalabra::SUST;
        else if (correctType == "Verbo") newType = TipoPalabra::VERB;
        else if (correctType == "Adjetivo") newType = TipoPalabra::ADJT;
        else if (correctType == "Adverbio") newType = TipoPalabra::ADV;
        else if (correctType == "Preposición") newType = TipoPalabra::PREP;
        else if (correctType == "Conjunción") newType = TipoPalabra::CONJ;
        else if (correctType == "Artículo") newType = TipoPalabra::ART;
        else if (correctType == "Pronombre") newType = TipoPalabra::PRON;

        if (newType != TipoPalabra::INDEFINIDO) {
            w.setTipo(newType);
            w.setConfianza(0.95f);
            w.setSignificado("Corregido por usuario a " + correctType);
            WordRepository::save(w);
        } else if (debugMode) {
            std::cerr << "[WARN] Tipo incorrecto para corrección: " << correctType << std::endl;
        }
    }

    void reprocessLastSentence() {
        if (!initialized || lastProcessedSentenceText.empty()) return;
        processSentence(lastProcessedSentenceText);
    }

    void resetContext() {
        contextWords.clear();
        lastProcessedSentenceText.clear();
        lastProcessedSentence = Sentence();
        lastPremiseText.clear();
        lastResponseText.clear();
    }

    WordInfo getWordInfo(std::string& word) {
        if (!initialized) return {};
        while (!word.empty() && ispunct(static_cast<unsigned char>(word.back()))) {
            word.pop_back();
        }
        Word w(word);
        WordRepository::load(word, w);
        return wordToInfo(w);
    }

private:
    WordInfo wordToInfo(const Word& w) {
        WordInfo info;
        info.word = w.getPalabra();
        info.tipo = tipoToString(w.getTipo());
        info.confianza = w.getConfianza();
        info.significado = w.getSignificado();
        info.cantidad = cantidadToString(w.getCantidad());
        info.tiempo = tiempoToString(w.getTiempo());
        info.genero = generoToString(w.getGenero());
        info.persona = personaToString(w.getPersona());
        info.grado = gradoToString(w.getGrado());
        info.relacionadas = w.getRelated();
        return info;
    }
};

// ============================================================================
// Implementación de la fachada pública
// ============================================================================

NLPEngine::NLPEngine() : pImpl(std::make_unique<Impl>()) {}
NLPEngine::~NLPEngine() { shutdown(); }

bool NLPEngine::initialize(const std::string& semanticDbPath,
                           const std::string& patternDbPath,
                           const std::string& temporalDbPath) {
    return pImpl->initialize(semanticDbPath, patternDbPath, temporalDbPath);
}

void NLPEngine::shutdown() { pImpl->shutdown(); }
void NLPEngine::setDebugMode(bool enable) { pImpl->setDebugMode(enable); }
void NLPEngine::learnText(const std::string& text) {
    if (!pImpl->initialized) return;
    learnTextWithContext(*pImpl->ctxCorr, *pImpl->patternCorrW, text);
}

std::vector<WordInfo> NLPEngine::processSentence(const std::string& sentence) {
    return pImpl->processSentence(sentence);
}

std::vector<Prediction> NLPEngine::predictNext(const std::string& currentWord) {
    return pImpl->predictNext(currentWord);
}

std::string NLPEngine::generateResponse(const std::string& premise) {
    return pImpl->generateResponse(premise);
}

void NLPEngine::correctWord(const std::string& word, const std::string& correctType) {
    pImpl->correctWord(word, correctType);
}

void NLPEngine::reprocessLastSentence() {
    pImpl->reprocessLastSentence();
}

void NLPEngine::resetContext() {
    pImpl->resetContext();
}

WordInfo NLPEngine::getWordInfo(std::string& word) {
    return pImpl->getWordInfo(word);
}

void NLPEngine::provideDialogueFeedback(bool positive) {
    pImpl->provideDialogueFeedback(positive);
}
