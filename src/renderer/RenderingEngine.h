#ifndef TARPLEX_RENDERING_ENGINE_H
#define TARPLEX_RENDERING_ENGINE_H

#include "LayoutEngine.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace tarplex::renderer {

/**
 * @struct Color
 * @brief RGBA color representation
 */
struct Color {
    uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    static Color FromHex(const std::string& hex);
    static Color FromRGB(uint8_t r, uint8_t g, uint8_t b);
    static Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t ToARGB() const {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8) |
               static_cast<uint32_t>(b);
    }

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

/**
 * @struct Rect
 * @brief Rectangle with integer coordinates
 */
struct Rect {
    int x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}

    bool Contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }

    bool Intersects(const Rect& other) const {
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }
};

/**
 * @class Framebuffer
 * @brief In-memory pixel buffer for rendering
 */
class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer() = default;

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

    // Pixel operations
    void SetPixel(int x, int y, const Color& color);
    Color GetPixel(int x, int y) const;
    void Clear(const Color& color);
    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color, int stroke_width = 1);
    void FillCircle(int cx, int cy, int radius, const Color& color);
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color, int width = 1);

    // Buffer access
    const uint32_t* GetBuffer() const { return buffer_.data(); }
    uint32_t* GetBuffer() { return buffer_.data(); }

    // Utility
    void CopyRect(const Rect& src, Framebuffer& dst, int dst_x, int dst_y) const;

private:
    int width_, height_;
    std::vector<uint32_t> buffer_;

    bool IsInBounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    void BlendPixel(int x, int y, const Color& color);
};

/**
 * @class RenderingEngine
 * @brief Converts layout tree to pixels
 */
class RenderingEngine {
public:
    RenderingEngine(int viewport_width, int viewport_height);
    ~RenderingEngine() = default;

    /**
     * @brief Render DOM tree to framebuffer
     * @param root Root DOM element with layout
     * @param framebuffer Target framebuffer
     * @return true if rendering succeeded
     */
    bool Render(
        const std::shared_ptr<DOMElement>& root,
        Framebuffer& framebuffer);

    /**
     * @brief Get rendering errors
     */
    const std::vector<std::string>& GetErrors() const { return errors_; }

    void ClearErrors() { errors_.clear(); }

    // Viewport management
    void SetViewportSize(int width, int height) {
        viewport_width_ = width;
        viewport_height_ = height;
    }

    int GetViewportWidth() const { return viewport_width_; }
    int GetViewportHeight() const { return viewport_height_; }

private:
    /**
     * @brief Recursively render element and children
     */
    void RenderElement(
        const std::shared_ptr<DOMElement>& element,
        Framebuffer& framebuffer);

    /**
     * @brief Render element's background
     */
    void RenderBackground(
        const std::shared_ptr<DOMElement>& element,
        Framebuffer& framebuffer);

    /**
     * @brief Render element's border
     */
    void RenderBorder(
        const std::shared_ptr<DOMElement>& element,
        Framebuffer& framebuffer);

    /**
     * @brief Render element's text content
     */
    void RenderText(
        const std::shared_ptr<DOMElement>& element,
        Framebuffer& framebuffer);

    /**
     * @brief Render element as image
     */
    void RenderImage(
        const std::shared_ptr<DOMElement>& element,
        Framebuffer& framebuffer);

    /**
     * @brief Parse color from CSS value
     */
    Color ParseColor(const std::string& color_str) const;

    /**
     * @brief Parse border style
     */
    enum class BorderStyle {
        SOLID,
        DASHED,
        DOTTED,
        DOUBLE,
        NONE
    };

    BorderStyle ParseBorderStyle(const std::string& style) const;

    /**
     * @brief Apply opacity/alpha blending
     */
    Color ApplyOpacity(const Color& color, float opacity) const;

    /**
     * @brief Check if element is visible in viewport
     */
    bool IsVisible(const std::shared_ptr<DOMElement>& element) const;

    int viewport_width_, viewport_height_;
    std::vector<std::string> errors_;

    // Rendering constants
    static constexpr int DEFAULT_FONT_SIZE = 16;
    static constexpr float ANTI_ALIAS_FACTOR = 1.0f;
};

/**
 * @class Font
 * @brief Font metrics and rendering information
 */
class Font {
public:
    Font(const std::string& family = "Arial", int size = 16, bool bold = false, bool italic = false)
        : family_(family), size_(size), bold_(bold), italic_(italic) {}

    const std::string& GetFamily() const { return family_; }
    int GetSize() const { return size_; }
    bool IsBold() const { return bold_; }
    bool IsItalic() const { return italic_; }

    int GetLineHeight() const {
        return static_cast<int>(size_ * 1.2f);
    }

    int MeasureText(const std::string& text) const;

private:
    std::string family_;
    int size_;
    bool bold_, italic_;
};

/**
 * @class TextRasterizer
 * @brief Renders text to framebuffer
 */
class TextRasterizer {
public:
    TextRasterizer() = default;

    /**
     * @brief Draw text on framebuffer
     */
    void DrawText(
        Framebuffer& framebuffer,
        const std::string& text,
        int x, int y,
        const Font& font,
        const Color& color);

    /**
     * @brief Measure text dimensions
     */
    void MeasureText(
        const std::string& text,
        const Font& font,
        int& width,
        int& height) const;

private:
    // Simple monospace font rendering
    // In production, would use FreeType or similar
    static constexpr int CHAR_WIDTH = 8;
    static constexpr int CHAR_HEIGHT = 16;

    void DrawChar(
        Framebuffer& framebuffer,
        char ch,
        int x, int y,
        const Color& color);
};

/**
 * @class RenderPass
 * @brief Represents a single rendering pass
 */
struct RenderPass {
    enum class Type {
        BACKGROUND,
        BORDER,
        TEXT,
        IMAGE,
        SHADOW
    };

    Type type;
    std::shared_ptr<DOMElement> element;
    Rect rect;
    Color color;
    int layer;

    RenderPass(Type t, const std::shared_ptr<DOMElement>& elem, const Rect& r, const Color& c, int l)
        : type(t), element(elem), rect(r), color(c), layer(l) {}
};

} // namespace tarplex::renderer

#endif // TARPLEX_RENDERING_ENGINE_H
