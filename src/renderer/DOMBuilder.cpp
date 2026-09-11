#include "DOMBuilder.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>

namespace tarplex::renderer {

// Static member initialization
const std::unordered_map<std::string, std::string> DOMBuilder::DEFAULT_STYLES = {
    {"color", "black"},
    {"background-color", "transparent"},
    {"font-size", "16px"},
    {"font-family", "Arial, sans-serif"},
    {"margin", "0"},
    {"padding", "0"},
    {"border", "none"},
    {"width", "auto"},
    {"height", "auto"},
    {"display", "block"},
    {"position", "static"},
    {"opacity", "1"},
    {"text-align", "left"}
};

// ============================================================================
// ComputedStyle Implementation
// ============================================================================

std::string ComputedStyle::GetDefaultValue(const std::string& name) const {
    auto it = DEFAULT_STYLES.find(name);
    if (it != DEFAULT_STYLES.end()) {
        return it->second;
    }
    return "";
}

// ============================================================================
// DOMElement Implementation
// ============================================================================

DOMElement::DOMElement(const std::string& tag_name)
    : tag_name_(tag_name), parent_(nullptr), 
      display_type_(DisplayType::BLOCK) {
    box_model_ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

std::string DOMElement::GetAttribute(const std::string& key) const {
    auto it = attributes_.find(key);
    if (it != attributes_.end()) {
        return it->second;
    }
    return "";
}

bool DOMElement::HasAttribute(const std::string& key) const {
    return attributes_.find(key) != attributes_.end();
}

bool DOMElement::HasClass(const std::string& class_name) const {
    std::string classes = GetAttribute("class");
    if (classes.empty()) return false;

    std::istringstream iss(classes);
    std::string cls;
    while (iss >> cls) {
        if (cls == class_name) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> DOMElement::GetClasses() const {
    std::vector<std::string> classes;
    std::string class_attr = GetAttribute("class");
    if (class_attr.empty()) return classes;

    std::istringstream iss(class_attr);
    std::string cls;
    while (iss >> cls) {
        classes.push_back(cls);
    }
    return classes;
}

std::string DOMElement::GetId() const {
    return GetAttribute("id");
}

// ============================================================================
// DOMBuilder Implementation
// ============================================================================

DOMBuilder::DOMBuilder() {}

std::shared_ptr<DOMElement> DOMBuilder::Build(
    const std::shared_ptr<HTMLNode>& html_root,
    const StyleSheet& stylesheet) {

    errors_.clear();

    if (!html_root) {
        errors_.push_back("HTML root node is null");
        return nullptr;
    }

    auto dom_root = BuildElement(html_root, stylesheet);
    return dom_root;
}

std::shared_ptr<DOMElement> DOMBuilder::BuildElement(
    const std::shared_ptr<HTMLNode>& html_node,
    const StyleSheet& stylesheet) {

    if (!html_node) return nullptr;

    // Create DOM element only for element nodes
    if (html_node->GetNodeType() == HTMLNode::NodeType::ELEMENT) {
        auto element = std::make_shared<DOMElement>(html_node->GetName());

        // Copy attributes
        for (const auto& [key, value] : html_node->GetAttributes()) {
            element->AddAttribute(key, value);
        }

        // Apply styles
        ApplyStyles(element, stylesheet);

        // Recursively build children
        for (const auto& child : html_node->GetChildren()) {
            if (child->GetNodeType() == HTMLNode::NodeType::TEXT) {
                // Create text node element
                auto text_elem = std::make_shared<DOMElement>("text");
                text_elem->SetContent(child->GetContent());
                element->AddChild(text_elem);
            } else if (child->GetNodeType() == HTMLNode::NodeType::ELEMENT) {
                auto child_elem = BuildElement(child, stylesheet);
                if (child_elem) {
                    element->AddChild(child_elem);
                }
            }
        }

        return element;
    } else if (html_node->GetNodeType() == HTMLNode::NodeType::TEXT) {
        // Text nodes are wrapped in parent elements
        auto text_elem = std::make_shared<DOMElement>("text");
        text_elem->SetContent(html_node->GetContent());
        return text_elem;
    } else if (html_node->GetNodeType() == HTMLNode::NodeType::DOCUMENT) {
        // For document node, create a root container
        auto root = std::make_shared<DOMElement>("html");

        // Build children
        for (const auto& child : html_node->GetChildren()) {
            auto child_elem = BuildElement(child, stylesheet);
            if (child_elem) {
                root->AddChild(child_elem);
            }
        }

        return root;
    }

    return nullptr;
}

void DOMBuilder::ApplyStyles(
    std::shared_ptr<DOMElement> element,
    const StyleSheet& stylesheet) {

    if (!element) return;

    // Get matching CSS rules
    auto classes = element->GetClasses();
    auto id = element->GetId();
    auto matching_rules = stylesheet.GetMatchingRules(
        element->GetTagName(), classes, id);

    // Cascade styles
    ComputedStyle computed_style = CascadeStyles(element, stylesheet);
    element->SetComputedStyle(computed_style);

    // Determine display type
    std::string display = computed_style.GetProperty("display");
    element->SetDisplayType(GetDisplayType(display));

    // Parse box model properties
    auto& box = element->GetBoxModel();

    // Parse margin
    std::string margin_str = computed_style.GetProperty("margin");
    if (!margin_str.empty()) {
        float margin = ParseDimension(margin_str);
        box.margin_top = box.margin_right = 
        box.margin_bottom = box.margin_left = margin;
    }

    // Parse padding
    std::string padding_str = computed_style.GetProperty("padding");
    if (!padding_str.empty()) {
        float padding = ParseDimension(padding_str);
        box.padding_top = box.padding_right = 
        box.padding_bottom = box.padding_left = padding;
    }

    // Parse width and height
    std::string width_str = computed_style.GetProperty("width");
    std::string height_str = computed_style.GetProperty("height");

    if (width_str != "auto" && !width_str.empty()) {
        box.width = ParseDimension(width_str);
    }

    if (height_str != "auto" && !height_str.empty()) {
        box.height = ParseDimension(height_str);
    }
}

DOMElement::DisplayType DOMBuilder::GetDisplayType(const std::string& display_prop) {
    if (display_prop == "none") return DOMElement::DisplayType::NONE;
    if (display_prop == "inline") return DOMElement::DisplayType::INLINE;
    if (display_prop == "inline-block") return DOMElement::DisplayType::INLINE_BLOCK;
    if (display_prop == "flex") return DOMElement::DisplayType::FLEX;
    if (display_prop == "grid") return DOMElement::DisplayType::GRID;
    if (display_prop == "table") return DOMElement::DisplayType::TABLE;

    return DOMElement::DisplayType::BLOCK;
}

float DOMBuilder::ParseDimension(const std::string& value, float reference_size) {
    if (value.empty()) return 0;

    // Remove whitespace
    std::string trimmed = value;
    trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), 
                                  [](char c) { return std::isspace(c); }),
                  trimmed.end());

    // Handle auto
    if (trimmed == "auto") return 0;

    // Handle percentage
    if (trimmed.back() == '%') {
        float percent = std::stof(trimmed.substr(0, trimmed.length() - 1));
        return (percent / 100.0f) * reference_size;
    }

    // Handle pixel values
    if (trimmed.length() >= 2 && trimmed.substr(trimmed.length() - 2) == "px") {
        return std::stof(trimmed.substr(0, trimmed.length() - 2));
    }

    // Handle em values
    if (trimmed.length() >= 2 && trimmed.substr(trimmed.length() - 2) == "em") {
        float em = std::stof(trimmed.substr(0, trimmed.length() - 2));
        return em * 16.0f; // Assume base font size is 16px
    }

    // Handle bare numbers (assume pixels)
    try {
        return std::stof(trimmed);
    } catch (...) {
        return 0;
    }
}

std::string DOMBuilder::ParseColor(const std::string& value) {
    // For now, just return the value as-is
    // In a real implementation, this would convert color formats
    return value;
}

ComputedStyle DOMBuilder::CascadeStyles(
    const std::shared_ptr<DOMElement>& element,
    const StyleSheet& stylesheet) {

    ComputedStyle style;

    // Set default styles
    for (const auto& [name, value] : DEFAULT_STYLES) {
        style.SetProperty(name, value);
    }

    // Get matching rules
    auto classes = element->GetClasses();
    auto id = element->GetId();
    auto matching_rules = stylesheet.GetMatchingRules(
        element->GetTagName(), classes, id);

    // Apply matching rules in order (lowest to highest specificity)
    for (const auto& rule : matching_rules) {
        for (const auto& property : rule.GetProperties()) {
            style.SetProperty(property.GetName(), property.GetValue());
        }
    }

    // Apply inline styles (highest priority)
    std::string inline_style = element->GetAttribute("style");
    if (!inline_style.empty()) {
        size_t start = 0;
        size_t semi_pos = inline_style.find(';');

        while (semi_pos != std::string::npos) {
            std::string prop = inline_style.substr(start, semi_pos - start);
            size_t colon_pos = prop.find(':');

            if (colon_pos != std::string::npos) {
                std::string name = prop.substr(0, colon_pos);
                std::string value = prop.substr(colon_pos + 1);

                // Trim whitespace
                name.erase(0, name.find_first_not_of(" \t\n\r\f\v"));
                name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);
                value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
                value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                style.SetProperty(name, value);
            }

            start = semi_pos + 1;
            semi_pos = inline_style.find(';', start);
        }
    }

    return style;
}

// ============================================================================
// RenderTree Implementation
// ============================================================================

std::shared_ptr<DOMElement> RenderTree::FindElementById(const std::string& id) const {
    if (!root_) return nullptr;

    std::shared_ptr<DOMElement> result;
    FindById(root_, id, result);
    return result;
}

std::vector<std::shared_ptr<DOMElement>> RenderTree::FindElementsByClass(
    const std::string& class_name) const {

    std::vector<std::shared_ptr<DOMElement>> results;
    if (root_) {
        FindByClass(root_, class_name, results);
    }
    return results;
}

std::vector<std::shared_ptr<DOMElement>> RenderTree::FindElementsByTag(
    const std::string& tag_name) const {

    std::vector<std::shared_ptr<DOMElement>> results;
    if (root_) {
        FindByTag(root_, tag_name, results);
    }
    return results;
}

void RenderTree::FindById(
    const std::shared_ptr<DOMElement>& element,
    const std::string& id,
    std::shared_ptr<DOMElement>& result) const {

    if (!element) return;

    if (element->GetId() == id) {
        result = element;
        return;
    }

    for (const auto& child : element->GetChildren()) {
        if (result) return; // Early exit if found
        FindById(child, id, result);
    }
}

void RenderTree::FindByClass(
    const std::shared_ptr<DOMElement>& element,
    const std::string& class_name,
    std::vector<std::shared_ptr<DOMElement>>& results) const {

    if (!element) return;

    if (element->HasClass(class_name)) {
        results.push_back(element);
    }

    for (const auto& child : element->GetChildren()) {
        FindByClass(child, class_name, results);
    }
}

void RenderTree::FindByTag(
    const std::shared_ptr<DOMElement>& element,
    const std::string& tag_name,
    std::vector<std::shared_ptr<DOMElement>>& results) const {

    if (!element) return;

    if (element->GetTagName() == tag_name) {
        results.push_back(element);
    }

    for (const auto& child : element->GetChildren()) {
        FindByTag(child, tag_name, results);
    }
}

} // namespace tarplex::renderer
