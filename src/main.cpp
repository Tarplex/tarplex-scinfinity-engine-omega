#include <iostream>
#include <QApplication>
#include "ui/MainWindow.h"
#include "Engine.h"

int main(int argc, char *argv[])
{
    // Initialize Qt Application
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Tarplex Scinfinity Engine Omega");
    app.setApplicationVersion("0.1.0");
    app.setApplicationDisplayName("Tarplex Browser");
    
    // Initialize the browser engine
    if (!tarplex::Engine::Initialize()) {
        std::cerr << "Failed to initialize browser engine" << std::endl;
        return 1;
    }
    
    // Create and show main window
    tarplex::ui::MainWindow window;
    window.show();
    
    // Run the application
    int result = app.exec();
    
    // Shutdown the engine
    tarplex::Engine::Shutdown();
    
    return result;
}
