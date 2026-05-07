/**==============================================================================
    Admin8.2.1 - LearningHelpers.cpp
    Proposito: Implementación de ayudas de aprendizaje.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "LearningHelpers.hpp"
#include "../nlp/Tokenizer.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <limits>

// ============================================================================
// Implementacion públicas auxiliares
// ============================================================================

bool createWordVector(std::vector<Word> &words, const std::string &input) {
    std::vector<Token> toks = tokenize(input);
    if(toks.empty()) return false;
    size_t i = 0;

    while (i < toks.size()) {
        if((toks.size() - i) < 4) {
            Word a(toks[i].text);
            if(toks[i].type != TokenType::WORD) {
                a.setTipo((toks[i].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                a.setConfianza(0.95f);
            }
            words.push_back(a);
            i+=1;
        }else{
            Word a(toks[i].text);
            if(toks[i].type != TokenType::WORD) {
                a.setTipo((toks[i].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                a.setConfianza(0.95f);
            }
            Word b(toks[i+1].text);
            if(toks[i+1].type != TokenType::WORD) {
                b.setTipo((toks[i+1].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                b.setConfianza(0.95f);
            }
            Word c(toks[i+2].text);
            if(toks[i+2].type != TokenType::WORD) {
                c.setTipo((toks[i+2].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                c.setConfianza(0.95f);
            }
            Word d(toks[i+3].text);
            if(toks[i+3].type != TokenType::WORD) {
                d.setTipo((toks[i+3].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                d.setConfianza(0.95f);
            }
            words.insert(words.end(), {a, b, c, d});
            i+=4;
        }
    }
    return true;
}

std::vector<Word> createWordVector(const std::string &input) {
    std::vector<Word> words;
    std::vector<Token> toks = tokenize(input);
    if(toks.empty()) return {};
    size_t i = 0;

    while (i < toks.size()) {
        if((toks.size() - i) < 4) {
            Word a(toks[i].text);
            if(toks[i].type != TokenType::WORD) {
                a.setTipo((toks[i].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                a.setConfianza(0.95f);
            }
            words.push_back(a);
            i+=1;
        }else{
            Word a(toks[i].text);
            if(toks[i].type != TokenType::WORD) {
                a.setTipo((toks[i].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                a.setConfianza(0.95f);
            }
            Word b(toks[i+1].text);
            if(toks[i+1].type != TokenType::WORD) {
                b.setTipo((toks[i+1].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                b.setConfianza(0.95f);
            }
            Word c(toks[i+2].text);
            if(toks[i+2].type != TokenType::WORD) {
                c.setTipo((toks[i+2].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                c.setConfianza(0.95f);
            }
            Word d(toks[i+3].text);
            if(toks[i+3].type != TokenType::WORD) {
                d.setTipo((toks[i+3].type == TokenType::DATE) ? TipoPalabra::DATE : TipoPalabra::NUM);
                d.setConfianza(0.95f);
            }
            words.insert(words.end(), {a, b, c, d});
            i+=4;
        }
    }
    return words;
}

void learnContextual(ContextualCorrelator& ctx, const std::string& text) {
    if (text.empty()) return;
    ctx.learnWithPreviousTwo(text);
}

void learnDirect(ContextualCorrelator& ctx, const std::string& text) {
    if (text.empty()) return;
    ctx.learnNextWordDirect(text);
}

void learnTextWithContext(ContextualCorrelator& ctx, PatternCorrelator& corr, const std::string& text) {
    if (text.empty()) return;
    corr.learnFromText(text, 1);
    ctx.learnWithPreviousTwo(text);
    ctx.learnNextWordDirect(text);
}

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void trimString(std::string& line) {
    line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), line.end());
}
