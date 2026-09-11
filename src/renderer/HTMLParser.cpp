#include "HTMLParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

namespace tarplex::renderer {

// Static member initialization
const std::vector<std::string> HTMLParser::VOID_ELEMENTS = {
    "area", "base", "br", "col", "embed", "hr", "img", "input",
    "link", "meta", "param", "source", "track", "wbr"
};

const std::vector<std::string> HTMLParser::BLOCK_ELEMENTS = {
    "address", "article", "aside", "blockquote", "body", "canvas",
    "dd", "div", "dl", "dt", "fieldset", "figcaption", "figure",
    "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6",
    "header", "hgroup", "hr", "html", "li", "main", "nav",
    "ol", "p", "pre", "section", "table", "ul", "video"
};

// ============================================================================
// HTMLNode Implementation
// ============================================================================

HTMLNode::HTMLNode(NodeType type, const std::string& name)
    : type_(type), name_(name), parent_(nullptr) {}

std::string HTMLNode::GetAttribute(const std::string& key) const {
    auto it = attributes_.find(key);
    if (it != attributes_.end()) {
        return it->second;
    }
    return "";\n}\n\nbool HTMLNode::HasAttribute(const std::string& key) const {
    return attributes_.find(key) != attributes_.end();
}

std::vector<std::shared_ptr<HTMLNode>> HTMLNode::GetChildrenByName(
    const std::string& name) const {
    std::vector<std::shared_ptr<HTMLNode>> result;
    for (const auto& child : children_) {
        if (child->GetName() == name) {
            result.push_back(child);
        }
    }
    return result;
}

// ============================================================================
// HTMLParser Implementation
// ============================================================================

HTMLParser::HTMLParser() {}

std::shared_ptr<HTMLNode> HTMLParser::Parse(const std::string& html) {
    errors_.clear();

    // Tokenize the HTML
    std::vector<Token> tokens = Tokenize(html);

    // Build the DOM tree
    auto root = BuildDOM(tokens);

    return root;
}

// ============================================================================
// Tokenization Methods
// ============================================================================

std::vector<HTMLParser::Token> HTMLParser::Tokenize(const std::string& html) {
    std::vector<Token> tokens;
    size_t pos = 0;

    while (pos < html.length()) {
        Token token = ReadToken(html, pos);
        if (token.type != Token::Type::EOF_TOKEN) {
            tokens.push_back(token);
        }
    }

    // Add EOF token
    Token eof_token;
    eof_token.type = Token::Type::EOF_TOKEN;
    tokens.push_back(eof_token);

    return tokens;
}

HTMLParser::Token HTMLParser::ReadToken(const std::string& html, size_t& pos) {
    if (pos >= html.length()) {
        Token token;
        token.type = Token::Type::EOF_TOKEN;
        return token;
    }

    // Check for comment
    if (pos + 3 < html.length() && html.substr(pos, 4) == "<!--") {
        return ReadComment(html, pos);
    }

    // Check for DOCTYPE
    if (pos + 8 < html.length() && 
        html.substr(pos, 9) == "<!DOCTYPE" || 
        html.substr(pos, 9) == "<!doctype") {
        return ReadDoctype(html, pos);
    }

    // Check for opening tag
    if (html[pos] == '<') {
        if (pos + 1 < html.length() && html[pos + 1] == '/') {
            return ReadCloseTag(html, pos);
        } else {
            return ReadOpenTag(html, pos);
        }
    }

    // Otherwise, it's text content
    return ReadTextContent(html, pos);
}

HTMLParser::Token HTMLParser::ReadOpenTag(const std::string& html, size_t& pos) {
    Token token;
    token.type = Token::Type::TAG_OPEN;

    pos++; // Skip '<'

    // Read tag name
    std::string tag_name;
    while (pos < html.length() && !std::isspace(html[pos]) && 
           html[pos] != '>' && html[pos] != '/') {
        tag_name += html[pos++];
    }

    token.value = NormalizeTagName(tag_name);

    // Skip whitespace
    while (pos < html.length() && std::isspace(html[pos])) {
        pos++;
    }

    // Read attributes
    std::string attr_str;
    while (pos < html.length() && html[pos] != '>' && html[pos] != '/') {
        attr_str += html[pos++];
    }

    token.attributes = ParseAttributes(attr_str);

    // Check for self-closing tag
    if (pos < html.length() && html[pos] == '/') {
        pos++;
        token.type = Token::Type::TAG_SELF_CLOSE;
    }

    // Skip '>'
    if (pos < html.length() && html[pos] == '>') {
        pos++;
    }

    return token;
}

HTMLParser::Token HTMLParser::ReadCloseTag(const std::string& html, size_t& pos) {
    Token token;
    token.type = Token::Type::TAG_CLOSE;

    pos += 2; // Skip '</'

    std::string tag_name;
    while (pos < html.length() && html[pos] != '>') {
        if (!std::isspace(html[pos])) {
            tag_name += html[pos];
        }
        pos++;
    }

    token.value = NormalizeTagName(tag_name);

    // Skip '>'
    if (pos < html.length() && html[pos] == '>') {
        pos++;
    }

    return token;
}

HTMLParser::Token HTMLParser::ReadTextContent(const std::string& html, size_t& pos) {
    Token token;
    token.type = Token::Type::TEXT;

    while (pos < html.length() && html[pos] != '<') {
        token.value += html[pos++];
    }

    token.value = DecodeEntities(token.value);
    token.value = TrimWhitespace(token.value);

    return token;
}

HTMLParser::Token HTMLParser::ReadComment(const std::string& html, size_t& pos) {
    Token token;
    token.type = Token::Type::COMMENT;

    pos += 4; // Skip '<!--'

    while (pos + 2 < html.length()) {
        if (html.substr(pos, 3) == "-->") {
            pos += 3;
            break;
        }
        token.value += html[pos++];
    }

    return token;
}

HTMLParser::Token HTMLParser::ReadDoctype(const std::string& html, size_t& pos) {
    Token token;
    token.type = Token::Type::DOCTYPE;

    while (pos < html.length() && html[pos] != '>') {
        token.value += html[pos++];
    }

    if (pos < html.length() && html[pos] == '>') {
        pos++;
    }

    return token;
}

// ============================================================================
// Attribute Parsing
// ============================================================================

std::unordered_map<std::string, std::string> HTMLParser::ParseAttributes(
    const std::string& attr_str) {
    std::unordered_map<std::string, std::string> attributes;

    size_t pos = 0;
    while (pos < attr_str.length()) {
        // Skip whitespace
        while (pos < attr_str.length() && std::isspace(attr_str[pos])) {
            pos++;
        }

        if (pos >= attr_str.length()) break;

        // Read attribute name
        std::string attr_name;
        while (pos < attr_str.length() && attr_str[pos] != '=' && 
               !std::isspace(attr_str[pos]) && attr_str[pos] != '>' && 
               attr_str[pos] != '/') {
            attr_name += std::tolower(attr_str[pos++]);
        }

        // Skip whitespace and '='
        while (pos < attr_str.length() && (std::isspace(attr_str[pos]) || 
               attr_str[pos] == '=')) {
            pos++;
        }

        // Read attribute value
        std::string attr_value;
        if (pos < attr_str.length() && (attr_str[pos] == '"' || 
            attr_str[pos] == '\'')) {
            char quote = attr_str[pos++];
            while (pos < attr_str.length() && attr_str[pos] != quote) {
                attr_value += attr_str[pos++];
            }
            if (pos < attr_str.length()) pos++; // Skip closing quote
        } else {
            while (pos < attr_str.length() && !std::isspace(attr_str[pos]) && 
                   attr_str[pos] != '>' && attr_str[pos] != '/') {
                attr_value += attr_str[pos++];
            }
        }

        if (!attr_name.empty()) {
            attributes[attr_name] = attr_value;
        }
    }

    return attributes;
}

// ============================================================================
// DOM Building
// ============================================================================

std::shared_ptr<HTMLNode> HTMLParser::BuildDOM(const std::vector<Token>& tokens) {
    auto root = std::make_shared<HTMLNode>(HTMLNode::NodeType::DOCUMENT, "document");
    std::vector<std::shared_ptr<HTMLNode>> stack = {root};

    for (const auto& token : tokens) {
        if (token.type == Token::Type::TAG_OPEN ||
            token.type == Token::Type::TAG_SELF_CLOSE) {
            auto node = std::make_shared<HTMLNode>(HTMLNode::NodeType::ELEMENT, 
                                                   token.value);

            // Add attributes
            for (const auto& [key, value] : token.attributes) {
                node->AddAttribute(key, value);
            }

            // Add to current parent
            if (!stack.empty()) {
                stack.back()->AddChild(node);
            }

            // Push to stack if not self-closing
            if (token.type == Token::Type::TAG_OPEN && 
                !IsVoidElement(token.value)) {
                stack.push_back(node);
            }

        } else if (token.type == Token::Type::TAG_CLOSE) {
            // Pop from stack
            if (stack.size() > 1) {
                // Find matching opening tag
                for (int i = stack.size() - 1; i >= 1; i--) {
                    if (stack[i]->GetName() == token.value) {
                        stack.erase(stack.begin() + i);
                        break;
                    }
                }
            }

        } else if (token.type == Token::Type::TEXT && !token.value.empty()) {
            auto text_node = std::make_shared<HTMLNode>(
                HTMLNode::NodeType::TEXT, "text");
            text_node->SetContent(token.value);

            if (!stack.empty()) {
                stack.back()->AddChild(text_node);
            }

        } else if (token.type == Token::Type::COMMENT) {
            auto comment_node = std::make_shared<HTMLNode>(
                HTMLNode::NodeType::COMMENT, "comment");
            comment_node->SetContent(token.value);

            if (!stack.empty()) {
                stack.back()->AddChild(comment_node);
            }
        }
    }

    return root;
}

bool HTMLParser::IsVoidElement(const std::string& tag_name) const {
    std::string lower_tag = tag_name;
    std::transform(lower_tag.begin(), lower_tag.end(), lower_tag.begin(),
                   ::tolower);
    return std::find(VOID_ELEMENTS.begin(), VOID_ELEMENTS.end(), 
                     lower_tag) != VOID_ELEMENTS.end();
}

bool HTMLParser::IsBlockElement(const std::string& tag_name) const {
    std::string lower_tag = tag_name;
    std::transform(lower_tag.begin(), lower_tag.end(), lower_tag.begin(),
                   ::tolower);
    return std::find(BLOCK_ELEMENTS.begin(), BLOCK_ELEMENTS.end(), 
                     lower_tag) != BLOCK_ELEMENTS.end();
}

std::string HTMLParser::NormalizeTagName(const std::string& tag) const {
    std::string result = tag;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// ============================================================================
// Utility Methods
// ============================================================================

std::string HTMLParser::DecodeEntities(const std::string& text) {
    std::string result;
    size_t pos = 0;

    while (pos < text.length()) {
        if (text[pos] == '&' && pos + 1 < text.length()) {
            size_t semi_pos = text.find(';', pos);
            if (semi_pos != std::string::npos) {
                std::string entity = text.substr(pos + 1, semi_pos - pos - 1);

                // Decode common HTML entities
                if (entity == "lt") {
                    result += '<';
                } else if (entity == "gt") {
                    result += '>';
                } else if (entity == "amp") {
                    result += '&';
                } else if (entity == "quot") {
                    result += '"';
                } else if (entity == "apos") {
                    result += '\'';
                } else if (entity == "nbsp") {
                    result += ' ';
                } else {
                    result += text.substr(pos, semi_pos - pos + 1);
                }

                pos = semi_pos + 1;
                continue;
            }
        }

        result += text[pos++];
    }

    return result;
}

std::string HTMLParser::TrimWhitespace(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

bool HTMLParser::IsWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || 
           c == '\f' || c == '\v';
}

} // namespace tarplex::renderer
