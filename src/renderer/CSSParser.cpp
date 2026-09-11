#include "CSSParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

namespace tarplex::renderer {

// Static member initialization
const std::vector<std::string> CSSParser::VALID_PROPERTIES = {
    "color", "background", "background-color", "font-size", "font-family",
    "margin", "padding", "border", "width", "height", "display",
    "position", "top", "left", "right", "bottom", "z-index",
    "opacity", "transform", "transition", "animation", "box-shadow",
    "text-align", "text-decoration", "font-weight", "line-height",
    "letter-spacing", "text-transform", "white-space", "overflow",
    "border-radius", "box-sizing", "flex", "grid", "justify-content",
    "align-items", "flex-direction", "gap", "cursor", "visibility"
};

const std::vector<std::string> CSSParser::CSS_COLOR_KEYWORDS = {
    "red", "green", "blue", "white", "black", "yellow", "cyan", "magenta",
    "gray", "grey", "orange", "purple", "pink", "brown", "transparent"
};

// ============================================================================
// CSSSelector Implementation
// ============================================================================

CSSSelector::CSSSelector(SelectorType type, const std::string& value)
    : type_(type), value_(value) {}

// ============================================================================
// CSSProperty Implementation
// ============================================================================

CSSProperty::CSSProperty(const std::string& name, const std::string& value)
    : name_(name), value_(value), important_(false) {}

// ============================================================================
// CSSRule Implementation
// ============================================================================

void CSSRule::CalculateSpecificity() {
    specificity_ = 0;
    for (const auto& selector : selectors_) {
        switch (selector.GetType()) {
            case CSSSelector::SelectorType::ID:
                specificity_ += 100;
                break;
            case CSSSelector::SelectorType::CLASS:
            case CSSSelector::SelectorType::PSEUDO_CLASS:
            case CSSSelector::SelectorType::ATTRIBUTE:
                specificity_ += 10;
                break;
            case CSSSelector::SelectorType::ELEMENT:
                specificity_ += 1;
                break;
            default:
                break;
        }
    }
}

// ============================================================================
// CSSParser Implementation
// ============================================================================

CSSParser::CSSParser() {}

std::vector<CSSRule> CSSParser::Parse(const std::string& css) {
    errors_.clear();
    std::vector<CSSRule> rules;

    std::vector<Token> tokens = Tokenize(css);

    size_t pos = 0;
    while (pos < tokens.size() && tokens[pos].type != Token::Type::EOF_TOKEN) {
        CSSRule rule = ParseRule(tokens, pos);
        if (!rule.GetSelectors().empty()) {
            rule.CalculateSpecificity();
            rules.push_back(rule);
        }
    }

    return rules;
}

// ============================================================================
// Tokenization
// ============================================================================

std::vector<CSSParser::Token> CSSParser::Tokenize(const std::string& css) {
    std::vector<Token> tokens;
    size_t pos = 0;

    while (pos < css.length()) {
        // Skip whitespace and comments
        while (pos < css.length() && std::isspace(css[pos])) {
            pos++;
        }

        if (pos >= css.length()) break;

        // Skip comments
        if (pos + 1 < css.length() && css[pos] == '/' && css[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < css.length()) {
                if (css[pos] == '*' && css[pos + 1] == '/') {
                    pos += 2;
                    break;
                }
                pos++;
            }
            continue;
        }

        Token token = ReadToken(css, pos);
        if (token.type != Token::Type::EOF_TOKEN) {
            tokens.push_back(token);
        }
    }

    Token eof_token;
    eof_token.type = Token::Type::EOF_TOKEN;
    tokens.push_back(eof_token);

    return tokens;
}

CSSParser::Token CSSParser::ReadToken(const std::string& css, size_t& pos) {
    if (pos >= css.length()) {
        Token token;
        token.type = Token::Type::EOF_TOKEN;
        return token;
    }

    Token token;

    if (css[pos] == '{') {
        token.type = Token::Type::LBRACE;
        token.value = "{";
        pos++;
    } else if (css[pos] == '}') {
        token.type = Token::Type::RBRACE;
        token.value = "}";
        pos++;
    } else if (css[pos] == ';') {
        token.type = Token::Type::SEMICOLON;
        token.value = ";";
        pos++;
    } else if (css[pos] == ':') {
        token.type = Token::Type::COLON;
        token.value = ":";
        pos++;
    } else if (css[pos] == ',') {
        token.type = Token::Type::COMMA;
        token.value = ",";
        pos++;
    } else {
        // Read selector or value
        std::string content;
        while (pos < css.length() && css[pos] != '{' && css[pos] != '}' &&
               css[pos] != ';' && css[pos] != ':' && css[pos] != ',') {
            content += css[pos++];
        }
        content = TrimWhitespace(content);
        token.value = content;
        token.type = Token::Type::SELECTOR;
    }

    return token;
}

// ============================================================================
// Rule Parsing
// ============================================================================

CSSRule CSSParser::ParseRule(std::vector<Token>& tokens, size_t& pos) {
    CSSRule rule;

    // Read selectors until {
    std::string selector_str;
    while (pos < tokens.size() && tokens[pos].type != Token::Type::LBRACE) {
        if (tokens[pos].type == Token::Type::SELECTOR) {
            selector_str += tokens[pos].value;
        }
        pos++;
    }

    if (!selector_str.empty()) {
        auto selectors = ParseSelectors(selector_str);
        for (const auto& sel : selectors) {
            rule.AddSelector(sel);
        }
    }

    // Skip {
    if (pos < tokens.size() && tokens[pos].type == Token::Type::LBRACE) {
        pos++;
    }

    // Read declarations until }
    std::string decl_str;
    while (pos < tokens.size() && tokens[pos].type != Token::Type::RBRACE) {
        if (tokens[pos].type == Token::Type::SELECTOR ||
            tokens[pos].type == Token::Type::COLON) {
            decl_str += tokens[pos].value;
        }
        pos++;
    }

    if (!decl_str.empty()) {
        auto properties = ParseDeclarations(decl_str);
        for (const auto& prop : properties) {
            rule.AddProperty(prop);
        }
    }

    // Skip }
    if (pos < tokens.size() && tokens[pos].type == Token::Type::RBRACE) {
        pos++;
    }

    return rule;
}

std::vector<CSSSelector> CSSParser::ParseSelectors(const std::string& selector_str) {
    std::vector<CSSSelector> selectors;

    // Split by comma for multiple selectors
    size_t start = 0;
    size_t comma_pos = selector_str.find(',');

    while (comma_pos != std::string::npos) {
        std::string single_sel = selector_str.substr(start, comma_pos - start);
        single_sel = TrimWhitespace(single_sel);

        CSSSelector sel = ParseSelector(single_sel);
        selectors.push_back(sel);

        start = comma_pos + 1;
        comma_pos = selector_str.find(',', start);
    }

    // Last selector
    std::string single_sel = selector_str.substr(start);
    single_sel = TrimWhitespace(single_sel);
    if (!single_sel.empty()) {
        CSSSelector sel = ParseSelector(single_sel);
        selectors.push_back(sel);
    }

    return selectors;
}

CSSSelector CSSParser::ParseSelector(const std::string& selector_str) {
    std::string trimmed = TrimWhitespace(selector_str);

    if (trimmed == "*") {
        return CSSSelector(CSSSelector::SelectorType::UNIVERSAL, "*");
    } else if (trimmed[0] == '#') {
        return CSSSelector(CSSSelector::SelectorType::ID, trimmed.substr(1));
    } else if (trimmed[0] == '.') {
        return CSSSelector(CSSSelector::SelectorType::CLASS, trimmed.substr(1));
    } else if (trimmed[0] == '[') {
        return CSSSelector(CSSSelector::SelectorType::ATTRIBUTE, trimmed);
    } else if (trimmed[0] == ':') {
        return CSSSelector(CSSSelector::SelectorType::PSEUDO_CLASS, trimmed.substr(1));
    } else {
        return CSSSelector(CSSSelector::SelectorType::ELEMENT, trimmed);
    }
}

std::vector<CSSProperty> CSSParser::ParseDeclarations(const std::string& decl_str) {
    std::vector<CSSProperty> properties;

    size_t start = 0;
    size_t semi_pos = decl_str.find(';');

    while (semi_pos != std::string::npos) {
        std::string single_prop = decl_str.substr(start, semi_pos - start);
        single_prop = TrimWhitespace(single_prop);

        if (!single_prop.empty()) {
            CSSProperty prop = ParseProperty(single_prop);
            if (!prop.GetName().empty()) {
                properties.push_back(prop);
            }
        }

        start = semi_pos + 1;
        semi_pos = decl_str.find(';', start);
    }

    // Last property (if no trailing semicolon)
    std::string single_prop = decl_str.substr(start);
    single_prop = TrimWhitespace(single_prop);
    if (!single_prop.empty()) {
        CSSProperty prop = ParseProperty(single_prop);
        if (!prop.GetName().empty()) {
            properties.push_back(prop);
        }
    }

    return properties;
}

CSSProperty CSSParser::ParseProperty(const std::string& prop_str) {
    size_t colon_pos = prop_str.find(':');

    if (colon_pos == std::string::npos) {
        return CSSProperty("", "");
    }

    std::string name = prop_str.substr(0, colon_pos);
    std::string value = prop_str.substr(colon_pos + 1);

    name = NormalizePropertyName(TrimWhitespace(name));
    value = TrimWhitespace(value);

    // Check for !important
    bool important = false;
    size_t important_pos = value.find("!important");
    if (important_pos != std::string::npos) {
        important = true;
        value = value.substr(0, important_pos);
        value = TrimWhitespace(value);
    }

    CSSProperty prop(name, value);
    prop.SetImportant(important);

    return prop;
}

// ============================================================================
// Utility Methods
// ============================================================================

std::string CSSParser::TrimWhitespace(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::string CSSParser::NormalizePropertyName(const std::string& name) {
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool CSSParser::IsValidPropertyName(const std::string& name) const {
    return std::find(VALID_PROPERTIES.begin(), VALID_PROPERTIES.end(), 
                     name) != VALID_PROPERTIES.end();
}

int CSSParser::CalculateSelectorSpecificity(
    const std::vector<CSSSelector>& selectors) {
    int specificity = 0;
    for (const auto& selector : selectors) {
        switch (selector.GetType()) {
            case CSSSelector::SelectorType::ID:
                specificity += 100;
                break;
            case CSSSelector::SelectorType::CLASS:
            case CSSSelector::SelectorType::PSEUDO_CLASS:
            case CSSSelector::SelectorType::ATTRIBUTE:
                specificity += 10;
                break;
            case CSSSelector::SelectorType::ELEMENT:
                specificity += 1;
                break;
            default:
                break;
        }
    }
    return specificity;
}

// ============================================================================
// StyleSheet Implementation
// ============================================================================

std::vector<CSSRule> StyleSheet::GetMatchingRules(
    const std::string& element_name,
    const std::vector<std::string>& class_names,
    const std::string& id) const {

    std::vector<CSSRule> matching_rules;

    for (const auto& rule : rules_) {
        bool matches = false;

        for (const auto& selector : rule.GetSelectors()) {
            if (selector.GetType() == CSSSelector::SelectorType::UNIVERSAL) {
                matches = true;
                break;
            } else if (selector.GetType() == CSSSelector::SelectorType::ELEMENT) {
                if (selector.GetValue() == element_name) {
                    matches = true;
                    break;
                }
            } else if (selector.GetType() == CSSSelector::SelectorType::CLASS) {
                auto it = std::find(class_names.begin(), class_names.end(),
                                   selector.GetValue());
                if (it != class_names.end()) {
                    matches = true;
                    break;
                }
            } else if (selector.GetType() == CSSSelector::SelectorType::ID) {
                if (selector.GetValue() == id) {
                    matches = true;
                    break;
                }
            }
        }

        if (matches) {
            matching_rules.push_back(rule);
        }
    }

    // Sort by specificity
    std::sort(matching_rules.begin(), matching_rules.end(),
              [](const CSSRule& a, const CSSRule& b) {
                  return a.GetSpecificity() < b.GetSpecificity();
              });

    return matching_rules;
}

} // namespace tarplex::renderer
