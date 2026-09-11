#ifndef TARPLEX_CSS_PARSER_H
#define TARPLEX_CSS_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace tarplex::renderer {

/**
 * @class CSSSelector
 * @brief Represents a CSS selector
 */
class CSSSelector {
public:
    enum class SelectorType {
        UNIVERSAL,      // *
        ELEMENT,        // div, p, etc.
        CLASS,          // .classname
        ID,             // #id
        ATTRIBUTE,      // [attr], [attr=value]
        PSEUDO_CLASS,   // :hover, :focus, etc.
        COMBINATORS     // >, +, ~, space
    };

    CSSSelector(SelectorType type, const std::string& value = "");

    SelectorType GetType() const { return type_; }
    const std::string& GetValue() const { return value_; }
    void SetValue(const std::string& value) { value_ = value; }

private:
    SelectorType type_;
    std::string value_;
};

/**
 * @class CSSProperty
 * @brief Represents a single CSS property
 */
class CSSProperty {
public:
    CSSProperty(const std::string& name, const std::string& value);

    const std::string& GetName() const { return name_; }
    const std::string& GetValue() const { return value_; }
    bool IsImportant() const { return important_; }

    void SetImportant(bool important) { important_ = important; }

private:
    std::string name_;
    std::string value_;
    bool important_;
};

/**
 * @class CSSRule
 * @brief Represents a complete CSS rule (selector + declarations)
 */
class CSSRule {
public:
    CSSRule() : specificity_(0) {}

    void AddSelector(const CSSSelector& selector) {
        selectors_.push_back(selector);
    }

    void AddProperty(const CSSProperty& property) {
        properties_.push_back(property);
    }

    const std::vector<CSSSelector>& GetSelectors() const { return selectors_; }
    const std::vector<CSSProperty>& GetProperties() const { return properties_; }
    int GetSpecificity() const { return specificity_; }

    void CalculateSpecificity();

private:
    std::vector<CSSSelector> selectors_;
    std::vector<CSSProperty> properties_;
    int specificity_;
};

/**
 * @class CSSParser
 * @brief Parses CSS strings into rules
 */
class CSSParser {
public:
    CSSParser();
    ~CSSParser() = default;

    /**
     * @brief Parse CSS string
     * @param css CSS content to parse
     * @return Vector of CSS rules
     */
    std::vector<CSSRule> Parse(const std::string& css);

    /**
     * @brief Get parsing errors
     * @return Vector of error messages
     */
    const std::vector<std::string>& GetErrors() const { return errors_; }

    /**
     * @brief Clear errors
     */
    void ClearErrors() { errors_.clear(); }

private:
    struct Token {
        enum class Type {
            SELECTOR,
            LBRACE,          // {
            RBRACE,          // }
            SEMICOLON,       // ;
            COLON,            // :
            COMMA,            // ,
            PROPERTY_NAME,
            PROPERTY_VALUE,
            EOF_TOKEN
        };

        Type type;
        std::string value;
    };

    // Tokenization
    std::vector<Token> Tokenize(const std::string& css);
    Token ReadToken(const std::string& css, size_t& pos);

    // Parsing
    CSSRule ParseRule(std::vector<Token>& tokens, size_t& pos);
    std::vector<CSSSelector> ParseSelectors(const std::string& selector_str);
    CSSSelector ParseSelector(const std::string& selector_str);
    std::vector<CSSProperty> ParseDeclarations(const std::string& decl_str);
    CSSProperty ParseProperty(const std::string& prop_str);

    // Utility functions
    std::string TrimWhitespace(const std::string& str);
    std::string NormalizePropertyName(const std::string& name);
    bool IsValidPropertyName(const std::string& name) const;
    int CalculateSelectorSpecificity(const std::vector<CSSSelector>& selectors);

    std::vector<std::string> errors_;
    static const std::vector<std::string> VALID_PROPERTIES;
    static const std::vector<std::string> CSS_COLOR_KEYWORDS;
};

/**
 * @class StyleSheet
 * @brief Container for CSS rules
 */
class StyleSheet {
public:
    StyleSheet() = default;
    ~StyleSheet() = default;

    void AddRule(const CSSRule& rule) {
        rules_.push_back(rule);
    }

    const std::vector<CSSRule>& GetRules() const { return rules_; }

    /**
     * @brief Get matching rules for an element
     * @param element_name Tag name of the element
     * @param class_names Class names of the element
     * @param id ID of the element
     * @return Vector of matching CSS rules (sorted by specificity)
     */
    std::vector<CSSRule> GetMatchingRules(
        const std::string& element_name,
        const std::vector<std::string>& class_names,
        const std::string& id) const;

private:
    std::vector<CSSRule> rules_;
};

} // namespace tarplex::renderer

#endif // TARPLEX_CSS_PARSER_H
