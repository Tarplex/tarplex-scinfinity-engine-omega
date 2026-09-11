#ifndef TARPLEX_LAYOUT_ENGINE_H
#define TARPLEX_LAYOUT_ENGINE_H

#include "DOMBuilder.h"
#include <vector>
#include <memory>

namespace tarplex::renderer {

/**
 * @class LayoutContext
 * @brief Context information for layout calculations
 */
class LayoutContext {
public:
    LayoutContext(float viewport_width = 800, float viewport_height = 600)
        : viewport_width_(viewport_width), viewport_height_(viewport_height) {}

    float GetViewportWidth() const { return viewport_width_; }
    float GetViewportHeight() const { return viewport_height_; }
    void SetViewportSize(float width, float height) {
        viewport_width_ = width;
        viewport_height_ = height;
    }

private:
    float viewport_width_;
    float viewport_height_;
};

/**
 * @class LayoutEngine
 * @brief Performs layout calculations on the DOM tree
 * 
 * Supports:
 * - Block layout (default)
 * - Inline layout
 * - Flexbox layout
 * - Grid layout (basic)
 */
class LayoutEngine {
public:
    LayoutEngine(const LayoutContext& context = LayoutContext());
    ~LayoutEngine() = default;

    /**
     * @brief Perform layout on the entire render tree
     * @param root Root DOM element
     * @return true if layout succeeded, false otherwise
     */
    bool Layout(const std::shared_ptr<DOMElement>& root);

    /**
     * @brief Get layout errors
     * @return Vector of error messages
     */
    const std::vector<std::string>& GetErrors() const { return errors_; }

    /**
     * @brief Clear errors
     */
    void ClearErrors() { errors_.clear(); }

    /**
     * @brief Set layout context
     */
    void SetContext(const LayoutContext& context) { context_ = context; }

    /**
     * @brief Get layout context
     */
    const LayoutContext& GetContext() const { return context_; }

private:
    /**
     * @brief Layout a single element and its children
     */
    bool LayoutElement(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height,
        float parent_x = 0,
        float parent_y = 0);

    /**
     * @brief Perform block layout for an element
     */
    void LayoutBlock(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height,
        float parent_x,
        float parent_y);

    /**
     * @brief Perform inline layout for an element
     */
    void LayoutInline(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height,
        float parent_x,
        float parent_y);

    /**
     * @brief Perform flexbox layout for an element
     */
    void LayoutFlex(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height,
        float parent_x,
        float parent_y);

    /**
     * @brief Perform grid layout for an element
     */
    void LayoutGrid(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height,
        float parent_x,
        float parent_y);

    /**
     * @brief Calculate element dimensions based on parent constraints
     */
    void CalculateDimensions(
        const std::shared_ptr<DOMElement>& element,
        float parent_width,
        float parent_height);

    /**
     * @brief Calculate element position within parent
     */
    void CalculatePosition(
        const std::shared_ptr<DOMElement>& element,
        float parent_x,
        float parent_y,
        float offset_x,
        float offset_y);

    /**
     * @brief Calculate margin, padding, and border (box model)
     */
    void CalculateBoxModel(
        const std::shared_ptr<DOMElement>& element,
        float parent_width);

    /**
     * @brief Collapse vertical margins between elements
     */
    float CollapseMargin(float margin1, float margin2) const;

    /**
     * @brief Calculate content width for flex items
     */
    float CalculateFlexItemWidth(
        const std::shared_ptr<DOMElement>& item,
        float available_width,
        float total_flex);

    /**
     * @brief Get element's z-index
     */
    int GetZIndex(const std::shared_ptr<DOMElement>& element) const;

    /**
     * @brief Sort elements by z-index
     */
    void SortByZIndex(std::vector<std::shared_ptr<DOMElement>>& elements);

    LayoutContext context_;
    std::vector<std::string> errors_;

    // Layout constants
    static constexpr float MIN_DIMENSION = 0.0f;
    static constexpr float DEFAULT_FONT_SIZE = 16.0f;
    static constexpr float DEFAULT_LINE_HEIGHT = 1.5f;
};

/**
 * @class LayoutResult
 * @brief Results of layout calculation
 */
struct LayoutResult {
    bool success;
    std::vector<std::string> errors;
    float total_width;
    float total_height;
    int element_count;
};

} // namespace tarplex::renderer

#endif // TARPLEX_LAYOUT_ENGINE_H
