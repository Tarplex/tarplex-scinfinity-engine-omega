#include "LayoutEngine.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <sstream>

namespace tarplex::renderer {

// ============================================================================
// LayoutEngine Implementation
// ============================================================================

LayoutEngine::LayoutEngine(const LayoutContext& context)
    : context_(context) {}

bool LayoutEngine::Layout(const std::shared_ptr<DOMElement>& root) {
    errors_.clear();

    if (!root) {
        errors_.push_back("Root element is null");
        return false;
    }

    try {
        bool success = LayoutElement(
            root,
            context_.GetViewportWidth(),
            context_.GetViewportHeight(),
            0, 0);

        if (!success) {
            errors_.push_back("Layout failed for root element");
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        errors_.push_back(std::string("Layout exception: ") + e.what());
        return false;
    }
}

bool LayoutEngine::LayoutElement(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height,
    float parent_x,
    float parent_y) {

    if (!element) return false;

    // Skip hidden elements
    if (element->GetDisplayType() == DOMElement::DisplayType::NONE) {
        return true;
    }

    // Calculate box model first
    CalculateBoxModel(element, parent_width);

    // Calculate dimensions
    CalculateDimensions(element, parent_width, parent_height);

    // Perform layout based on display type
    switch (element->GetDisplayType()) {
        case DOMElement::DisplayType::BLOCK:
            LayoutBlock(element, parent_width, parent_height, parent_x, parent_y);
            break;
        case DOMElement::DisplayType::INLINE:
            LayoutInline(element, parent_width, parent_height, parent_x, parent_y);
            break;
        case DOMElement::DisplayType::INLINE_BLOCK:
            LayoutBlock(element, parent_width, parent_height, parent_x, parent_y);
            break;
        case DOMElement::DisplayType::FLEX:
            LayoutFlex(element, parent_width, parent_height, parent_x, parent_y);
            break;
        case DOMElement::DisplayType::GRID:
            LayoutGrid(element, parent_width, parent_height, parent_x, parent_y);
            break;
        default:
            LayoutBlock(element, parent_width, parent_height, parent_x, parent_y);
    }

    return true;
}

void LayoutEngine::LayoutBlock(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height,
    float parent_x,
    float parent_y) {

    auto& box = element->GetBoxModel();
    float content_x = parent_x + box.margin_left;
    float content_y = parent_y + box.margin_top;
    float content_width = element->GetBoxModel().width;

    float current_y = content_y;

    // Layout children
    for (const auto& child : element->GetChildren()) {
        if (child->GetDisplayType() == DOMElement::DisplayType::NONE) {
            continue;
        }

        // Skip text nodes for positioning
        if (child->GetTagName() == "text") {
            continue;
        }

        LayoutElement(child, content_width, parent_height, content_x, current_y);

        // Update Y position for next block element
        if (child->GetDisplayType() == DOMElement::DisplayType::BLOCK ||
            child->GetDisplayType() == DOMElement::DisplayType::FLEX ||
            child->GetDisplayType() == DOMElement::DisplayType::GRID) {
            
            auto& child_box = child->GetBoxModel();
            current_y += child_box.height + child_box.margin_top + child_box.margin_bottom;
        }
    }

    // Update element height based on content
    box.height = current_y - content_y + box.padding_bottom;
    if (box.height < 0) box.height = 0;

    // Set element position
    CalculatePosition(element, parent_x, parent_y, 0, 0);
}

void LayoutEngine::LayoutInline(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height,
    float parent_x,
    float parent_y) {

    auto& box = element->GetBoxModel();
    float line_height = DEFAULT_LINE_HEIGHT * DEFAULT_FONT_SIZE;
    
    float current_x = parent_x;
    float current_y = parent_y;
    float line_width = 0;
    float line_height_actual = line_height;

    // Layout inline children
    for (const auto& child : element->GetChildren()) {
        if (child->GetDisplayType() == DOMElement::DisplayType::NONE) {
            continue;
        }

        CalculateBoxModel(child, parent_width);
        CalculateDimensions(child, parent_width, parent_height);

        auto& child_box = child->GetBoxModel();
        float child_width = child_box.width + child_box.margin_left + child_box.margin_right;

        // Check if we need to wrap to next line
        if (current_x + child_width > parent_x + parent_width && line_width > 0) {
            current_x = parent_x;
            current_y += line_height_actual;
            line_width = 0;
            line_height_actual = line_height;
        }

        child_box.x = current_x + child_box.margin_left;
        child_box.y = current_y + child_box.margin_top;

        current_x += child_width;
        line_width += child_width;
    }

    box.height = current_y - parent_y + line_height_actual + box.padding_bottom;
    CalculatePosition(element, parent_x, parent_y, 0, 0);
}

void LayoutEngine::LayoutFlex(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height,
    float parent_x,
    float parent_y) {

    auto& box = element->GetBoxModel();
    float content_width = box.width - box.padding_left - box.padding_right;
    float content_height = box.height - box.padding_top - box.padding_bottom;

    // Get flex direction
    std::string flex_direction = element->GetComputedStyle().GetProperty("flex-direction");
    bool is_column = (flex_direction == "column");

    float current_pos = is_column ? parent_y + box.padding_top : parent_x + box.padding_left;
    float cross_pos = is_column ? parent_x + box.padding_left : parent_y + box.padding_top;

    // Calculate total flex and collect items
    std::vector<std::shared_ptr<DOMElement>> flex_items;
    float total_flex = 0;
    float used_space = 0;

    for (const auto& child : element->GetChildren()) {
        if (child->GetDisplayType() == DOMElement::DisplayType::NONE ||
            child->GetTagName() == "text") {
            continue;
        }

        flex_items.push_back(child);

        // Get flex value
        std::string flex_str = child->GetComputedStyle().GetProperty("flex");
        float flex_value = 1;
        try {
            if (!flex_str.empty() && flex_str != "none") {
                flex_value = std::stof(flex_str);
            }
        } catch (...) {
            flex_value = 1;
        }

        total_flex += flex_value;
        CalculateBoxModel(child, content_width);
        CalculateDimensions(child, content_width, content_height);
    }

    // Layout flex items
    for (const auto& item : flex_items) {
        CalculateDimensions(item, content_width, content_height);
        auto& item_box = item->GetBoxModel();

        if (is_column) {
            item_box.x = cross_pos + item_box.margin_left;
            item_box.y = current_pos + item_box.margin_top;
            current_pos += item_box.height + item_box.margin_top + item_box.margin_bottom;
        } else {
            item_box.x = current_pos + item_box.margin_left;
            item_box.y = cross_pos + item_box.margin_top;
            current_pos += item_box.width + item_box.margin_left + item_box.margin_right;
        }
    }

    // Update element dimensions
    if (is_column) {
        box.height = current_pos - parent_y - box.padding_bottom;
    } else {
        box.width = current_pos - parent_x - box.padding_right;
    }

    CalculatePosition(element, parent_x, parent_y, 0, 0);
}

void LayoutEngine::LayoutGrid(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height,
    float parent_x,
    float parent_y) {

    auto& box = element->GetBoxModel();
    float content_width = box.width - box.padding_left - box.padding_right;
    float content_height = box.height - box.padding_top - box.padding_bottom;

    // Get grid template columns
    std::string grid_cols = element->GetComputedStyle().GetProperty("grid-template-columns");
    int cols = 1;
    try {
        // Parse simple grid: "1fr 1fr" or "repeat(2, 1fr)" or "auto auto"
        if (grid_cols.find("repeat") != std::string::npos) {
            size_t start = grid_cols.find('(');
            size_t comma = grid_cols.find(',', start);
            if (start != std::string::npos && comma != std::string::npos) {
                cols = std::stoi(grid_cols.substr(start + 1, comma - start - 1));
            }
        } else {
            cols = std::count(grid_cols.begin(), grid_cols.end(), ' ') + 1;
            if (grid_cols.empty()) cols = 1;
        }
    } catch (...) {
        cols = 1;
    }

    float col_width = content_width / cols;
    int current_col = 0;
    float current_x = parent_x + box.padding_left;
    float current_y = parent_y + box.padding_top;
    float row_height = 0;

    for (const auto& child : element->GetChildren()) {
        if (child->GetDisplayType() == DOMElement::DisplayType::NONE ||
            child->GetTagName() == "text") {
            continue;
        }

        CalculateBoxModel(child, col_width);
        CalculateDimensions(child, col_width, content_height);

        auto& child_box = child->GetBoxModel();
        child_box.x = current_x + child_box.margin_left;
        child_box.y = current_y + child_box.margin_top;

        row_height = std::max(row_height, child_box.height + child_box.margin_top + child_box.margin_bottom);

        current_col++;
        if (current_col >= cols) {
            current_col = 0;
            current_x = parent_x + box.padding_left;
            current_y += row_height;
            row_height = 0;
        } else {
            current_x += col_width;
        }
    }

    box.height = current_y - parent_y + row_height + box.padding_bottom;
    CalculatePosition(element, parent_x, parent_y, 0, 0);
}

void LayoutEngine::CalculateDimensions(
    const std::shared_ptr<DOMElement>& element,
    float parent_width,
    float parent_height) {

    auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    // Get width
    std::string width_str = style.GetProperty("width");
    if (width_str != "auto" && !width_str.empty()) {
        box.width = ParseDimension(width_str, parent_width);
    } else {
        // Default width based on display type
        if (element->GetDisplayType() == DOMElement::DisplayType::BLOCK ||
            element->GetDisplayType() == DOMElement::DisplayType::FLEX ||
            element->GetDisplayType() == DOMElement::DisplayType::GRID) {
            box.width = parent_width - box.margin_left - box.margin_right;
        } else {
            box.width = 0; // Inline elements size to content
        }
    }

    // Get height
    std::string height_str = style.GetProperty("height");
    if (height_str != "auto" && !height_str.empty()) {
        box.height = ParseDimension(height_str, parent_height);
    } else {
        box.height = 0; // Will be calculated from content
    }

    // Ensure minimum dimensions
    if (box.width < MIN_DIMENSION) box.width = MIN_DIMENSION;
    if (box.height < MIN_DIMENSION) box.height = MIN_DIMENSION;
}

void LayoutEngine::CalculatePosition(
    const std::shared_ptr<DOMElement>& element,
    float parent_x,
    float parent_y,
    float offset_x,
    float offset_y) {

    auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    std::string position = style.GetProperty("position");

    if (position == "absolute" || position == "fixed") {
        std::string top = style.GetProperty("top");
        std::string left = style.GetProperty("left");
        std::string right = style.GetProperty("right");
        std::string bottom = style.GetProperty("bottom");

        if (!top.empty() && top != "auto") {
            box.y = parent_y + ParseDimension(top);
        }
        if (!left.empty() && left != "auto") {
            box.x = parent_x + ParseDimension(left);
        }
    } else {
        // Already positioned in layout methods
    }
}

void LayoutEngine::CalculateBoxModel(
    const std::shared_ptr<DOMElement>& element,
    float parent_width) {

    auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    // Parse margin
    std::string margin = style.GetProperty("margin");
    if (!margin.empty()) {
        float m = ParseDimension(margin, parent_width);
        box.margin_top = box.margin_right = box.margin_bottom = box.margin_left = m;
    }

    // Parse padding
    std::string padding = style.GetProperty("padding");
    if (!padding.empty()) {
        float p = ParseDimension(padding, parent_width);
        box.padding_top = box.padding_right = box.padding_bottom = box.padding_left = p;
    }

    // Parse border
    std::string border = style.GetProperty("border");
    if (!border.empty()) {
        float b = ParseDimension(border);
        box.border_top = box.border_right = box.border_bottom = box.border_left = b;
    }
}

float LayoutEngine::CollapseMargin(float margin1, float margin2) const {
    return std::max(margin1, margin2);
}

float LayoutEngine::CalculateFlexItemWidth(
    const std::shared_ptr<DOMElement>& item,
    float available_width,
    float total_flex) {

    if (total_flex == 0) return available_width;

    std::string flex_str = item->GetComputedStyle().GetProperty("flex");
    float flex_value = 1;
    try {
        if (!flex_str.empty() && flex_str != "none") {
            flex_value = std::stof(flex_str);
        }
    } catch (...) {
        flex_value = 1;
    }

    return available_width * (flex_value / total_flex);
}

int LayoutEngine::GetZIndex(const std::shared_ptr<DOMElement>& element) const {
    std::string z_str = element->GetComputedStyle().GetProperty("z-index");
    try {
        if (!z_str.empty() && z_str != "auto") {
            return std::stoi(z_str);
        }
    } catch (...) {
        return 0;
    }
    return 0;
}

void LayoutEngine::SortByZIndex(std::vector<std::shared_ptr<DOMElement>>& elements) {
    std::sort(elements.begin(), elements.end(),
              [this](const std::shared_ptr<DOMElement>& a, const std::shared_ptr<DOMElement>& b) {
                  return GetZIndex(a) < GetZIndex(b);
              });
}

} // namespace tarplex::renderer
