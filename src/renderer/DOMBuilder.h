#ifndef TARPLEX_DOM_BUILDER_H
#define TARPLEX_DOM_BUILDER_H

#include "HTMLParser.h"
#include "CSSParser.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace tarplex::renderer {

/**
 * @class ComputedStyle
 * @brief Represents computed CSS styles for a DOM element
 */
class ComputedStyle {
public:
    ComputedStyle() = default;
    ~ComputedStyle() = default;

    void SetProperty(const std::string& name, const std::string& value) {
        properties_[name] = value;
    }

    std::string GetProperty(const std::string& name) const {
        auto it = properties_.find(name);
        if (it != properties_.end()) {
            return it->second;
        }
        return GetDefaultValue(name);
    }

    bool HasProperty(const std::string& name) const {
        return properties_.find(name) != properties_.end();
    }

    const std::unordered_map<std::string, std::string>& GetAllProperties() const {
        return properties_;
    }

private:
    std::unordered_map<std::string, std::string> properties_;

    std::string GetDefaultValue(const std::string& name) const;
};

/**
 * @class DOMElement
 * @brief Represents a styled DOM element in the render tree
 */
class DOMElement {
public:
    enum class DisplayType {
        BLOCK,
        INLINE,
        INLINE_BLOCK,
        NONE,
        FLEX,
        GRID,
        TABLE
    };

    DOMElement(const std::string& tag_name);
    ~DOMElement() = default;

    // Getters
    const std::string& GetTagName() const { return tag_name_; }
    const std::string& GetContent() const { return content_; }
    const std::unordered_map<std::string, std::string>& GetAttributes() const {
        return attributes_;
    }
    const ComputedStyle& GetComputedStyle() const { return computed_style_; }
    ComputedStyle& GetComputedStyle() { return computed_style_; }
    const std::vector<std::shared_ptr<DOMElement>>& GetChildren() const {
        return children_;
    }
    DOMElement* GetParent() const { return parent_; }
    DisplayType GetDisplayType() const { return display_type_; }

    // Setters
    void SetContent(const std::string& content) { content_ = content; }
    void AddAttribute(const std::string& key, const std::string& value) {
        attributes_[key] = value;
    }
    void AddChild(std::shared_ptr<DOMElement> child) {
        child->parent_ = this;
        children_.push_back(child);
    }
    void SetComputedStyle(const ComputedStyle& style) {
        computed_style_ = style;
    }
    void SetDisplayType(DisplayType type) { display_type_ = type; }

    // Query methods
    std::string GetAttribute(const std::string& key) const;
    bool HasAttribute(const std::string& key) const;
    bool HasClass(const std::string& class_name) const;
    std::vector<std::string> GetClasses() const;
    std::string GetId() const;

    // Layout properties
    struct BoxModel {
        float x, y;              // Position
        float width, height;     // Dimensions
        float margin_top, margin_right, margin_bottom, margin_left;
        float padding_top, padding_right, padding_bottom, padding_left;
        float border_top, border_right, border_bottom, border_left;
    };

    BoxModel& GetBoxModel() { return box_model_; }
    const BoxModel& GetBoxModel() const { return box_model_; }

private:
    std::string tag_name_;
    std::string content_;
    std::unordered_map<std::string, std::string> attributes_;
    ComputedStyle computed_style_;
    std::vector<std::shared_ptr<DOMElement>> children_;
    DOMElement* parent_;
    DisplayType display_type_;
    BoxModel box_model_;
};

/**
 * @class DOMBuilder
 * @brief Builds styled DOM tree from HTML and CSS
 */
class DOMBuilder {
public:
    DOMBuilder();
    ~DOMBuilder() = default;

    /**
     * @brief Build DOM tree with styles from HTML and CSS
     * @param html_root Root node from HTML parser
     * @param stylesheet CSS stylesheet
     * @return Root DOM element with computed styles
     */
    std::shared_ptr<DOMElement> Build(
        const std::shared_ptr<HTMLNode>& html_root,
        const StyleSheet& stylesheet);

    /**
     * @brief Get building errors
     * @return Vector of error messages
     */
    const std::vector<std::string>& GetErrors() const { return errors_; }

    /**
     * @brief Clear errors
     */
    void ClearErrors() { errors_.clear(); }

private:
    /**
     * @brief Recursively build DOM elements from HTML nodes
     */
    std::shared_ptr<DOMElement> BuildElement(
        const std::shared_ptr<HTMLNode>& html_node,
        const StyleSheet& stylesheet);

    /**
     * @brief Apply CSS styles to a DOM element
     */
    void ApplyStyles(
        std::shared_ptr<DOMElement> element,
        const StyleSheet& stylesheet);

    /**
     * @brief Get display type from CSS
     */
    DOMElement::DisplayType GetDisplayType(const std::string& display_prop);

    /**
     * @brief Parse dimension values (e.g., "10px", "50%")
     */
    float ParseDimension(const std::string& value, float reference_size = 0);

    /**
     * @brief Parse color values
     */
    std::string ParseColor(const std::string& value);

    /**
     * @brief Cascade CSS rules - find the most specific matching rule
     */
    ComputedStyle CascadeStyles(
        const std::shared_ptr<DOMElement>& element,
        const StyleSheet& stylesheet);

    std::vector<std::string> errors_;

    // Default styles
    static const std::unordered_map<std::string, std::string> DEFAULT_STYLES;
};

/**
 * @class RenderTree
 * @brief Complete render tree for a page
 */
class RenderTree {
public:
    RenderTree() = default;
    ~RenderTree() = default;

    void SetRoot(std::shared_ptr<DOMElement> root) { root_ = root; }
    const std::shared_ptr<DOMElement>& GetRoot() const { return root_; }

    /**
     * @brief Find element by ID
     */
    std::shared_ptr<DOMElement> FindElementById(const std::string& id) const;

    /**
     * @brief Find elements by class name
     */
    std::vector<std::shared_ptr<DOMElement>> FindElementsByClass(
        const std::string& class_name) const;

    /**
     * @brief Find elements by tag name
     */
    std::vector<std::shared_ptr<DOMElement>> FindElementsByTag(
        const std::string& tag_name) const;

private:
    std::shared_ptr<DOMElement> root_;

    void FindById(
        const std::shared_ptr<DOMElement>& element,
        const std::string& id,
        std::shared_ptr<DOMElement>& result) const;

    void FindByClass(
        const std::shared_ptr<DOMElement>& element,
        const std::string& class_name,
        std::vector<std::shared_ptr<DOMElement>>& results) const;

    void FindByTag(
        const std::shared_ptr<DOMElement>& element,
        const std::string& tag_name,
        std::vector<std::shared_ptr<DOMElement>>& results) const;
};

} // namespace tarplex::renderer

#endif // TARPLEX_DOM_BUILDER_H
