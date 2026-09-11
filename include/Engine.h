#ifndef TARPLEX_ENGINE_H
#define TARPLEX_ENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace tarplex {

/**
 * @class Engine
 * @brief Main browser engine class
 * 
 * This is the core of the Tarplex Scinfinity Engine Omega.
 * It manages all browser operations including:
 * - Tab management
 * - Navigation
 * - Rendering
 * - JavaScript execution
 * - Network operations
 */
class Engine {
public:
    /**
     * @brief Initialize the browser engine
     * @return true if initialization successful, false otherwise
     */
    static bool Initialize();
    
    /**
     * @brief Shutdown the browser engine
     */
    static void Shutdown();
    
    /**
     * @brief Create a new browser tab
     * @param url Initial URL to load
     * @return Tab ID
     */
    static int CreateTab(const std::string& url = "about:blank");
    
    /**
     * @brief Close a browser tab
     * @param tab_id Tab identifier
     */
    static void CloseTab(int tab_id);
    
    /**
     * @brief Navigate to a URL in a tab
     * @param tab_id Tab identifier
     * @param url URL to navigate to
     */
    static void Navigate(int tab_id, const std::string& url);
    
    /**
     * @brief Get the version of the engine
     * @return Version string
     */
    static std::string GetVersion();
    
private:
    Engine() = delete;
    ~Engine() = delete;
};

} // namespace tarplex

#endif // TARPLEX_ENGINE_H
