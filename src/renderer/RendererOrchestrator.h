#ifndef TARPLEX_RENDERER_ORCHESTRATOR_H
#define TARPLEX_RENDERER_ORCHESTRATOR_H

#include "DOMBuilder.h"
#include "StyleEngine.h"
#include "LayoutEngine.h"
#include "RenderingEngine.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>

namespace tarplex::renderer {

/**
 * @struct RenderStats
 * @brief Performance metrics for rendering
 */
struct RenderStats {
    std::chrono::milliseconds parse_time;
    std::chrono::milliseconds style_time;
    std::chrono::milliseconds layout_time;
    std::chrono::milliseconds render_time;
    std::chrono::milliseconds total_time;

    int element_count;
    int style_rule_count;
    int paint_operation_count;

    float GetTotalMs() const {
        return total_time.count();
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "=== Render Statistics ===\n"
           << "Parse:  " << parse_time.count() << "ms\n"
           << "Style:  " << style_time.count() << "ms\n"
           << "Layout: " << layout_time.count() << "ms\n"
           << "Render: " << render_time.count() << "ms\n"
           << "Total:  " << total_time.count() << "ms\n"
           << "Elements: " << element_count << "\n"
           << "Style Rules: " << style_rule_count << "\n"
           << "Paint Ops: " << paint_operation_count << "\n";
        return ss.str();
    }
};

/**
 * @class RendererOrchestrator
 * @brief Coordinates the entire rendering pipeline
 * 
 * Pipeline stages:
 * 1. Parse HTML/Template -> DOM Tree
 * 2. Apply CSS Styles -> Styled DOM
 * 3. Calculate Layout -> Layout Tree
 * 4. Paint to Framebuffer -> Final Output
 */
class RendererOrchestrator {
public:
    RendererOrchestrator(int viewport_width = 800, int viewport_height = 600);
    ~RendererOrchestrator() = default;

    /**
     * @brief Render HTML content to framebuffer
     * @param html_content HTML/template string
     * @param css_content CSS stylesheet content
     * @param framebuffer Target framebuffer for rendering
     * @return true if rendering succeeded
     */
    bool RenderHTML(
        const std::string& html_content,
        const std::string& css_content,
        Framebuffer& framebuffer);

    /**
     * @brief Render pre-built DOM tree
     * @param root Root DOM element
     * @param framebuffer Target framebuffer
     * @return true if rendering succeeded
     */
    bool RenderDOM(
        const std::shared_ptr<DOMElement>& root,
        Framebuffer& framebuffer);

    /**
     * @brief Get rendering statistics
     */
    const RenderStats& GetStats() const { return stats_; }

    /**
     * @brief Get all errors accumulated during rendering
     */
    std::vector<std::string> GetAllErrors() const;

    /**
     * @brief Clear all errors
     */
    void ClearErrors();

    /**
     * @brief Set viewport dimensions
     */
    void SetViewportSize(int width, int height);

    /**
     * @brief Get current viewport dimensions
     */
    void GetViewportSize(int& width, int& height) const {
        width = viewport_width_;
        height = viewport_height_;
    }

    /**
     * @brief Enable/disable rendering statistics
     */
    void SetCollectStats(bool collect) { collect_stats_ = collect; }

    /**
     * @brief Get DOM builder
     */
    std::shared_ptr<DOMBuilder> GetDOMBuilder() { return dom_builder_; }

    /**
     * @brief Get style engine
     */
    std::shared_ptr<StyleEngine> GetStyleEngine() { return style_engine_; }

    /**
     * @brief Get layout engine
     */
    std::shared_ptr<LayoutEngine> GetLayoutEngine() { return layout_engine_; }

    /**
     * @brief Get rendering engine
     */
    std::shared_ptr<RenderingEngine> GetRenderingEngine() { return rendering_engine_; }

    /**
     * @brief Check if rendering is in progress
     */
    bool IsRendering() const { return is_rendering_; }

    /**
     * @brief Get last render error
     */
    std::string GetLastError() const;

private:
    /**
     * @brief Internal rendering pipeline
     */
    bool ExecutePipeline(
        const std::shared_ptr<DOMElement>& root,
        const std::string& css_content,
        Framebuffer& framebuffer);

    /**
     * @brief Stage 1: Parse HTML to DOM
     */
    std::shared_ptr<DOMElement> ParseHTML(const std::string& html_content);

    /**
     * @brief Stage 2: Apply styles
     */
    bool ApplyStyles(
        const std::shared_ptr<DOMElement>& root,
        const std::string& css_content);

    /**
     * @brief Stage 3: Calculate layout
     */
    bool CalculateLayout(const std::shared_ptr<DOMElement>& root);

    /**
     * @brief Stage 4: Render to framebuffer
     */
    bool RenderToFramebuffer(
        const std::shared_ptr<DOMElement>& root,
        Framebuffer& framebuffer);

    /**
     * @brief Count elements in tree
     */
    int CountElements(const std::shared_ptr<DOMElement>& root) const;

    /**
     * @brief Validate rendering pipeline
     */
    bool ValidatePipeline() const;

    int viewport_width_, viewport_height_;
    bool is_rendering_;
    bool collect_stats_;

    // Pipeline components
    std::shared_ptr<DOMBuilder> dom_builder_;
    std::shared_ptr<StyleEngine> style_engine_;
    std::shared_ptr<LayoutEngine> layout_engine_;
    std::shared_ptr<RenderingEngine> rendering_engine_;

    // Statistics
    RenderStats stats_;

    // Error tracking
    std::vector<std::string> all_errors_;
    static constexpr size_t MAX_ERRORS = 100;
};

/**
 * @class RenderContext
 * @brief Context information for rendering operations
 */
struct RenderContext {
    int viewport_width;
    int viewport_height;
    float device_pixel_ratio;
    bool enable_caching;
    bool enable_antialiasing;

    RenderContext(int w = 800, int h = 600, float dpr = 1.0f)
        : viewport_width(w), viewport_height(h), device_pixel_ratio(dpr),
          enable_caching(true), enable_antialiasing(true) {}
};

/**
 * @class RenderCache
 * @brief Caches rendered output for reuse
 */
class RenderCache {
public:
    RenderCache(size_t max_size = 10);

    /**
     * @brief Store rendered framebuffer
     */
    void Store(const std::string& key, const Framebuffer& framebuffer);

    /**
     * @brief Retrieve cached framebuffer
     */
    bool Retrieve(const std::string& key, Framebuffer& framebuffer) const;

    /**
     * @brief Check if key exists in cache
     */
    bool Contains(const std::string& key) const;

    /**
     * @brief Clear cache
     */
    void Clear();

    /**
     * @brief Get cache size
     */
    size_t GetSize() const { return cache_.size(); }

private:
    struct CacheEntry {
        std::string key;
        std::unique_ptr<Framebuffer> framebuffer;
        std::chrono::system_clock::time_point timestamp;
    };

    std::vector<CacheEntry> cache_;
    size_t max_size_;
};

/**
 * @class RenderPipeline
 * @brief Manages rendering pipeline execution
 */
class RenderPipeline {
public:
    enum class Stage {
        IDLE,
        PARSING,
        STYLING,
        LAYOUT,
        RENDERING,
        COMPLETE,
        ERROR
    };

    RenderPipeline();

    Stage GetCurrentStage() const { return current_stage_; }
    std::string GetStageName() const;

    float GetProgress() const {
        return static_cast<float>(current_stage_) / static_cast<float>(Stage::COMPLETE);
    }

private:
    Stage current_stage_;
};

} // namespace tarplex::renderer

#endif // TARPLEX_RENDERER_ORCHESTRATOR_H
