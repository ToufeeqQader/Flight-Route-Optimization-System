#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>

class AirportManager;
class AircraftManager;
class RouteManager;
class FlightManager;
class MapWidget;

/**
 * @brief Main application window with tabbed interface
 *
 * Architecture pattern: Presenter layer
 * - Coordinates between UI components
 * - No business logic (delegated to DataStore)
 * - Manages application lifecycle
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSaveData();
    void onUndo();
    void onAbout();

private:
    void setupUi();
    void setupMenuBar();
    void loadData();

    QTabWidget* tabWidget;
    AirportManager* airportManager;
    AircraftManager* aircraftManager;
    RouteManager* routeManager;
    FlightManager* flightManager;
    MapWidget* mapWidget;
};

#endif // MAINWINDOW_H
