#include "RenderingEngine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <cctype>

namespace tarplex::renderer {

// ============================================================================
// Color Implementation
// ============================================================================

Color Color::FromHex(const std::string& hex) {
    std::string trimmed = hex;
    if (trimmed[0] == '#') {
        trimmed = trimmed.substr(1);
    }

    if (trimmed.length() == 6) {
        uint32_t color = std::stoul(trimmed, nullptr, 16);
        return Color(
            (color >> 16) & 0xFF,
            (color >> 8) & 0xFF,
            color & 0xFF,
            255
        );
    } else if (trimmed.length() == 8) {
        uint32_t color = std::stoul(trimmed, nullptr, 16);
        return Color(
            (color >> 16) & 0xFF,
            (color >> 8) & 0xFF,
            color & 0xFF,
            (color >> 24) & 0xFF
        );
    }

    return Color(0, 0, 0, 255);
}

Color Color::FromRGB(uint8_t r, uint8_t g, uint8_t b) {
    return Color(r, g, b, 255);
}

Color Color::FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return Color(r, g, b, a);
}

// ============================================================================
// Framebuffer Implementation
// ============================================================================

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height), buffer_(width * height, 0) {}

void Framebuffer::SetPixel(int x, int y, const Color& color) {
    if (!IsInBounds(x, y)) return;

    int idx = y * width_ + x;
    buffer_[idx] = color.ToARGB();
}

Color Framebuffer::GetPixel(int x, int y) const {
    if (!IsInBounds(x, y)) return Color(0, 0, 0, 0);

    int idx = y * width_ + x;
    uint32_t argb = buffer_[idx];

    return Color(
        (argb >> 16) & 0xFF,
        (argb >> 8) & 0xFF,
        argb & 0xFF,
        (argb >> 24) & 0xFF
    );
}

void Framebuffer::Clear(const Color& color) {
    uint32_t packed = color.ToARGB();
    std::fill(buffer_.begin(), buffer_.end(), packed);
}

void Framebuffer::FillRect(const Rect& rect, const Color& color) {
    for (int y = rect.y; y < rect.y + rect.height; ++y) {
        for (int x = rect.x; x < rect.x + rect.width; ++x) {
            SetPixel(x, y, color);
        }
    }
}

void Framebuffer::DrawRect(const Rect& rect, const Color& color, int stroke_width) {
    // Top and bottom edges
    for (int x = rect.x; x < rect.x + rect.width; ++x) {
        for (int w = 0; w < stroke_width; ++w) {
            SetPixel(x, rect.y + w, color);
            SetPixel(x, rect.y + rect.height - 1 - w, color);
        }
    }

    // Left and right edges
    for (int y = rect.y; y < rect.y + rect.height; ++y) {
        for (int w = 0; w < stroke_width; ++w) {
            SetPixel(rect.x + w, y, color);
            SetPixel(rect.x + rect.width - 1 - w, y, color);
        }
    }
}

void Framebuffer::FillCircle(int cx, int cy, int radius, const Color& color) {
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                SetPixel(cx + x, cy + y, color);
            }
        }
    }
}

void Framebuffer::DrawLine(int x1, int y1, int x2, int y2, const Color& color, int width) {
    // Bresenham's line algorithm
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1, y = y1;

    while (true) {
        // Draw thick line
        for (int wy = -width / 2; wy <= width / 2; ++wy) {
            for (int wx = -width / 2; wx <= width / 2; ++wx) {
                SetPixel(x + wx, y + wy, color);
            }
        }

        if (x == x2 && y == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void Framebuffer::BlendPixel(int x, int y, const Color& color) {
    if (!IsInBounds(x, y)) return;

    Color existing = GetPixel(x, y);

    // Alpha blending
    float alpha = color.a / 255.0f;
    float inv_alpha = 1.0f - alpha;

    Color blended(
        static_cast<uint8_t>(color.r * alpha + existing.r * inv_alpha),
        static_cast<uint8_t>(color.g * alpha + existing.g * inv_alpha),
        static_cast<uint8_t>(color.b * alpha + existing.b * inv_alpha),
        255
    );

    SetPixel(x, y, blended);
}

void Framebuffer::CopyRect(const Rect& src, Framebuffer& dst, int dst_x, int dst_y) const {
    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {
            Color pixel = GetPixel(src.x + x, src.y + y);
            dst.SetPixel(dst_x + x, dst_y + y, pixel);
        }
    }
}

// ============================================================================
// RenderingEngine Implementation
// ============================================================================

RenderingEngine::RenderingEngine(int viewport_width, int viewport_height)
    : viewport_width_(viewport_width), viewport_height_(viewport_height) {}

bool RenderingEngine::Render(
    const std::shared_ptr<DOMElement>& root,
    Framebuffer& framebuffer) {

    errors_.clear();

    if (!root) {
        errors_.push_back("Root element is null");
        return false;
    }

    try {
        // Clear framebuffer with white background
        framebuffer.Clear(Color(255, 255, 255, 255));

        // Recursively render all elements
        RenderElement(root, framebuffer);

        return true;
    } catch (const std::exception& e) {
        errors_.push_back(std::string("Rendering exception: ") + e.what());
        return false;
    }
}

void RenderingEngine::RenderElement(
    const std::shared_ptr<DOMElement>& element,
    Framebuffer& framebuffer) {

    if (!element) return;

    // Skip invisible elements
    if (element->GetDisplayType() == DOMElement::DisplayType::NONE) {
        return;
    }

    // Check if element is visible in viewport
    if (!IsVisible(element)) {
        return;
    }

    const auto& box = element->GetBoxModel();

    // Skip text nodes (handled by RenderText)
    if (element->GetTagName() == "text") {
        return;
    }

    // Render background
    RenderBackground(element, framebuffer);

    // Render border
    RenderBorder(element, framebuffer);

    // Render content based on tag
    if (element->GetTagName() == "img") {
        RenderImage(element, framebuffer);
    } else if (element->GetTagName() == "p" || 
               element->GetTagName() == "div" ||
               element->GetTagName() == "span" ||
               element->GetTagName() == "h1" ||
               element->GetTagName() == "h2" ||
               element->GetTagName() == "h3") {
        RenderText(element, framebuffer);
    }

    // Render children
    for (const auto& child : element->GetChildren()) {
        RenderElement(child, framebuffer);
    }
}

void RenderingEngine::RenderBackground(
    const std::shared_ptr<DOMElement>& element,
    Framebuffer& framebuffer) {

    const auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    std::string bg_color_str = style.GetProperty("background-color");
    if (bg_color_str.empty() || bg_color_str == "transparent") {
        return;
    }

    Color bg_color = ParseColor(bg_color_str);

    // Apply opacity
    std::string opacity_str = style.GetProperty("opacity");
    float opacity = 1.0f;
    try {
        if (!opacity_str.empty()) {
            opacity = std::stof(opacity_str);
        }
    } catch (...) {
        opacity = 1.0f;
    }

    bg_color = ApplyOpacity(bg_color, opacity);

    Rect bg_rect(
        static_cast<int>(box.x),
        static_cast<int>(box.y),
        static_cast<int>(box.width),
        static_cast<int>(box.height)
    );

    framebuffer.FillRect(bg_rect, bg_color);
}

void RenderingEngine::RenderBorder(
    const std::shared_ptr<DOMElement>& element,
    Framebuffer& framebuffer) {

    const auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    std::string border_str = style.GetProperty("border");
    if (border_str.empty() || border_str == "none") {
        return;
    }

    // Parse border color (simple parsing)
    std::string border_color_str = style.GetProperty("border-color");
    if (border_color_str.empty()) {
        border_color_str = "black";
    }

    Color border_color = ParseColor(border_color_str);
    int border_width = static_cast<int>(box.border_left);

    if (border_width > 0) {
        Rect border_rect(
            static_cast<int>(box.x),
            static_cast<int>(box.y),
            static_cast<int>(box.width),
            static_cast<int>(box.height)
        );

        framebuffer.DrawRect(border_rect, border_color, border_width);
    }
}

void RenderingEngine::RenderText(
    const std::shared_ptr<DOMElement>& element,
    Framebuffer& framebuffer) {

    const auto& box = element->GetBoxModel();
    const auto& style = element->GetComputedStyle();

    // Collect text from children
    std::string text;
    for (const auto& child : element->GetChildren()) {
        if (child->GetTagName() == "text") {
            text += child->GetContent();
        }
    }

    if (text.empty()) return;

    // Parse font properties
    std::string font_family = style.GetProperty("font-family");
    std::string font_size_str = style.GetProperty("font-size");
    std::string color_str = style.GetProperty("color");

    int font_size = DEFAULT_FONT_SIZE;
    try {
        if (!font_size_str.empty()) {
            font_size = static_cast<int>(std::stof(font_size_str));
        }
    } catch (...) {
        font_size = DEFAULT_FONT_SIZE;
    }

    Color text_color = ParseColor(color_str.empty() ? "black" : color_str);

    Font font(font_family, font_size);
    TextRasterizer rasterizer;

    rasterizer.DrawText(
        framebuffer,
        text,
        static_cast<int>(box.x + box.padding_left),
        static_cast<int>(box.y + box.padding_top),
        font,
        text_color
    );
}

void RenderingEngine::RenderImage(
    const std::shared_ptr<DOMElement>& element,
    Framebuffer& framebuffer) {

    // Placeholder: would load and render actual image
    const auto& box = element->GetBoxModel();
    std::string src = element->GetAttribute("src");

    if (src.empty()) return;

    // Draw placeholder rectangle
    Rect img_rect(
        static_cast<int>(box.x),
        static_cast<int>(box.y),
        static_cast<int>(box.width),
        static_cast<int>(box.height)
    );

    framebuffer.DrawRect(img_rect, Color(200, 200, 200), 2);
}

Color RenderingEngine::ParseColor(const std::string& color_str) const {
    if (color_str.empty()) return Color(0, 0, 0, 255);

    // Handle hex colors
    if (color_str[0] == '#') {
        return Color::FromHex(color_str);
    }

    // Handle named colors
    if (color_str == "black") return Color(0, 0, 0, 255);
    if (color_str == "white") return Color(255, 255, 255, 255);
    if (color_str == "red") return Color(255, 0, 0, 255);
    if (color_str == "green") return Color(0, 128, 0, 255);
    if (color_str == "blue") return Color(0, 0, 255, 255);
    if (color_str == "gray") return Color(128, 128, 128, 255);
    if (color_str == "transparent") return Color(0, 0, 0, 0);

    // Handle rgb() format
    if (color_str.find("rgb") != std::string::npos) {
        // Simple rgb parsing: "rgb(r, g, b)"
        size_t start = color_str.find('(');
        size_t end = color_str.find(')');
        if (start != std::string::npos && end != std::string::npos) {
            std::string values = color_str.substr(start + 1, end - start - 1);
            std::istringstream iss(values);
            int r, g, b;
            char comma;
            if (iss >> r >> comma >> g >> comma >> b) {
                return Color(r, g, b, 255);
            }
        }
    }

    return Color(0, 0, 0, 255);
}

RenderingEngine::BorderStyle RenderingEngine::ParseBorderStyle(const std::string& style) const {
    if (style == "dashed") return BorderStyle::DASHED;
    if (style == "dotted") return BorderStyle::DOTTED;
    if (style == "double") return BorderStyle::DOUBLE;
    if (style == "none") return BorderStyle::NONE;
    return BorderStyle::SOLID;
}

Color RenderingEngine::ApplyOpacity(const Color& color, float opacity) const {
    opacity = std::max(0.0f, std::min(1.0f, opacity));
    return Color(color.r, color.g, color.b, static_cast<uint8_t>(color.a * opacity));
}

bool RenderingEngine::IsVisible(const std::shared_ptr<DOMElement>& element) const {
    if (!element) return false;

    const auto& box = element->GetBoxModel();

    // Check if element is within viewport
    return !(box.x + box.width < 0 ||
             box.x > viewport_width_ ||
             box.y + box.height < 0 ||
             box.y > viewport_height_);
}

// ============================================================================
// Font Implementation
// ============================================================================

int Font::MeasureText(const std::string& text) const {
    return static_cast<int>(text.length() * size_ * 0.6f);
}

// ============================================================================
// TextRasterizer Implementation
// ============================================================================

void TextRasterizer::DrawText(
    Framebuffer& framebuffer,
    const std::string& text,
    int x, int y,
    const Font& font,
    const Color& color) {

    int current_x = x;
    int current_y = y;

    for (char ch : text) {
        if (ch == '\n') {
            current_x = x;
            current_y += font.GetLineHeight();
        } else if (ch == '\t') {
            current_x += CHAR_WIDTH * 4;
        } else {
            DrawChar(framebuffer, ch, current_x, current_y, color);
            current_x += CHAR_WIDTH;
        }
    }
}

void TextRasterizer::MeasureText(
    const std::string& text,
    const Font& font,
    int& width,
    int& height) const {

    width = 0;
    height = font.GetLineHeight();
    int current_width = 0;
    int line_count = 1;

    for (char ch : text) {
        if (ch == '\n') {
            width = std::max(width, current_width);
            current_width = 0;
            line_count++;
        } else if (ch == '\t') {
            current_width += CHAR_WIDTH * 4;
        } else {
            current_width += CHAR_WIDTH;
        }
    }

    width = std::max(width, current_width);
    height = line_count * font.GetLineHeight();
}

void TextRasterizer::DrawChar(
    Framebuffer& framebuffer,
    char ch,
    int x, int y,
    const Color& color) {

    // Simple character rendering - fills a rectangle
    // In production, would use proper font rendering
    if (std::isprint(ch)) {
        Rect char_rect(x, y, CHAR_WIDTH - 2, CHAR_HEIGHT - 2);
        framebuffer.FillRect(char_rect, color);
    }
}

} // namespace tarplex::renderer
