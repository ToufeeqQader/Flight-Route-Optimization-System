#include "mainwindow.h"

#include "AirportManager.h"
#include "AircraftManager.h"
#include "RouteManager.h"
#include "FlightManager.h"
#include "MapWidget.h"
#include "DataStore.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tabWidget(nullptr)
    , airportManager(nullptr)
    , aircraftManager(nullptr)
    , routeManager(nullptr)
    , flightManager(nullptr)
    , mapWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    loadData();

    setWindowTitle("SkyNet - Flight Route Management System");
    resize(1200, 800);
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    // Qt handles cleanup via parent-child relationship
}

void MainWindow::setupUi() {
    // Create central widget with tab interface
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // Create manager widgets
    airportManager = new AirportManager(this);
    aircraftManager = new AircraftManager(this);
    routeManager = new RouteManager(this);
    flightManager = new FlightManager(this);
    mapWidget = new MapWidget(this);

    flightManager->setMapWidget(mapWidget);
    // Add tabs
    tabWidget->addTab(mapWidget, "Map View");
    tabWidget->addTab(airportManager, "Airports");
    tabWidget->addTab(aircraftManager, "Aircraft");
    tabWidget->addTab(routeManager, "Routes");
    tabWidget->addTab(flightManager, "Flights");
}

void MainWindow::setupMenuBar() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");

    QAction* saveAction = fileMenu->addAction("&Save All");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveData);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");

    QAction* undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &MainWindow::onUndo);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::loadData() {
    DataStore& store = DataStore::getInstance();

    // Show loading message
    statusBar()->showMessage("Loading data from files...");

    if (store.loadAll()) {
        // Get loaded data counts
        int airportCount = store.getAllAirports().size();
        int aircraftCount = store.getAllAircraft().size();
        int routeCount = store.getAllRoutes().size();
        int flightCount = store.getAllFlights().size();

        // Show success message with counts
        QString message = QString("✓ Data loaded: %1 airports, %2 aircraft, %3 routes, %4 flights")
                              .arg(airportCount)
                              .arg(aircraftCount)
                              .arg(routeCount)
                              .arg(flightCount);

        statusBar()->showMessage(message, 5000);

        // CRITICAL: Force refresh all manager widgets
        if (airportManager) {
            // Trigger refresh by calling a public method
            // Since we don't have a public refresh, we'll use QMetaObject
            QMetaObject::invokeMethod(airportManager, "onRefresh", Qt::QueuedConnection);
        }

        if (aircraftManager) {
            QMetaObject::invokeMethod(aircraftManager, "onRefresh", Qt::QueuedConnection);
        }

        if (routeManager) {
            QMetaObject::invokeMethod(routeManager, "onRefresh", Qt::QueuedConnection);
        }

        if (flightManager) {
            QMetaObject::invokeMethod(flightManager, "onRefreshFlights", Qt::QueuedConnection);
        }

        if (mapWidget) {
            mapWidget->refresh();
        }

        // Show info dialog if no data
        if (airportCount == 0 && aircraftCount == 0) {
            QMessageBox::information(this, "Welcome to SkyNet",
                                     "No existing data found.\n\n"
                                     "To get started:\n"
                                     "1. Go to 'Airports' tab to add airports\n"
                                     "2. Go to 'Aircraft' tab to add aircraft\n"
                                     "3. Go to 'Routes' tab to create connections\n"
                                     "4. Go to 'Flights' tab to plan and book flights\n\n"
                                     "Sample data files are in the data_files/ folder.");
        }

    } else {
        statusBar()->showMessage("Error loading data", 5000);
        QMessageBox::warning(this, "Load Warning",
                             "Some data files could not be loaded.\n\n"
                             "Please ensure data_files/ folder exists with:\n"
                             "- airports.txt\n"
                             "- aircraft.txt\n"
                             "- routes.txt\n"
                             "- flights.txt");
    }
}

void MainWindow::onSaveData() {
    DataStore& store = DataStore::getInstance();

    if (store.saveAll()) {
        statusBar()->showMessage("Data saved successfully", 3000);
        QMessageBox::information(this, "Save Complete",
                                 "All data has been saved to disk.");
    } else {
        QMessageBox::critical(this, "Save Error",
                              "Failed to save data. Check file permissions.");
    }
}

void MainWindow::onUndo() {
    DataStore& store = DataStore::getInstance();

    if (store.canUndo()) {
        if (store.undo()) {
            statusBar()->showMessage("Action undone", 3000);
            // Refresh all views
            // Signal managers to reload
        } else {
            QMessageBox::warning(this, "Undo Failed",
                                 "Could not undo the last action.");
        }
    } else {
        QMessageBox::information(this, "No Actions",
                                 "No actions available to undo.");
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About SkyNet",
                       "<h2>SkyNet Flight Management System</h2>"
                       "<p>Version 1.0</p>"
                       "<p>An academic-grade flight route optimization system</p>"
                       "<p>Built with C++17 and Qt 6</p>"
                       "<p><b>Features:</b></p>"
                       "<ul>"
                       "<li>Dijkstra's shortest path algorithm</li>"
                       "<li>Haversine distance calculation</li>"
                       "<li>Complete CRUD operations</li>"
                       "<li>Interactive map visualization</li>"
                       "<li>Flight booking workflow</li>"
                       "<li>Undo system</li>"
                       "</ul>");
}
