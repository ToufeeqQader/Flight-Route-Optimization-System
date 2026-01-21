#include "MainWindow.h"
#include "DataStore.h"
#include <QApplication>
#include <QMessageBox>
#include <iostream>

/**
 * @brief Application entry point
 *
 * Initializes:
 * 1. Qt application
 * 2. DataStore singleton
 * 3. Main window
 *
 * Flow:
 * - Load data from CSV files
 * - Build flight network graph
 * - Show main window
 * - Enter Qt event loop
 */
int main(int argc, char *argv[]) {
    // Create Qt application
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("SkyNet");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Flight Systems Inc.");

    try {
        // Initialize data store
        DataStore& dataStore = DataStore::getInstance();

        std::cout << "SkyNet Flight Management System v1.0" << std::endl;
        std::cout << "Loading data..." << std::endl;

        // Load all data and build graph
        if (!dataStore.loadAll()) {
            std::cerr << "Warning: Some data files could not be loaded." << std::endl;
            std::cerr << "The application will start with empty data." << std::endl;
        }

        // Print statistics
        std::cout << "Airports loaded: " << dataStore.getAllAirports().size() << std::endl;
        std::cout << "Aircraft loaded: " << dataStore.getAllAircraft().size() << std::endl;
        std::cout << "Routes loaded: " << dataStore.getAllRoutes().size() << std::endl;
        std::cout << "Flights loaded: " << dataStore.getAllFlights().size() << std::endl;
        std::cout << "Graph nodes: " << dataStore.getGraph().getNodeCount() << std::endl;
        std::cout << "Graph edges: " << dataStore.getGraph().getEdgeCount() << std::endl;

        // Create and show main window
        MainWindow mainWindow;
        mainWindow.show();

        std::cout << "Application started successfully." << std::endl;
        std::cout << "Ready for operations." << std::endl;

        // Enter Qt event loop
        return app.exec();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;

        QMessageBox::critical(nullptr, "Fatal Error",
                              QString("Application failed to start:\n%1").arg(e.what()));

        return 1;
    }
}
