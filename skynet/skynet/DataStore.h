#ifndef DATASTORE_H
#define DATASTORE_H

#include "airports.h"
#include "aircraft.h"
#include "Route.h"
#include "Flight.h"
#include "Graph.h"
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <memory>

/**
 * @brief Undo action types for the undo stack
 */
enum class ActionType {
    ADD_AIRPORT,
    DELETE_AIRPORT,
    UPDATE_AIRPORT,
    ADD_AIRCRAFT,
    DELETE_AIRCRAFT,
    UPDATE_AIRCRAFT,
    ADD_ROUTE,
    DELETE_ROUTE,
    ADD_FLIGHT,
    DELETE_FLIGHT
};

/**
 * @brief Represents a reversible action
 * Stores enough data to undo the operation
 */
struct Action {
    ActionType type;
    std::string data;  // Serialized entity data for restoration

    Action(ActionType t, const std::string& d) : type(t), data(d) {}
};

/**
 * @brief Singleton DataStore managing all application data
 *
 * Why Singleton?
 * - Single source of truth for all data
 * - Global access point without global variables
 * - Ensures only one instance manages file I/O and graph
 *
 * Why std::map for storage?
 * - O(log n) lookup by ID/code
 * - Automatic sorting by key
 * - No duplicates
 *
 * Responsibilities:
 * - CRUD operations for all entities
 * - CSV file I/O (manual parsing, no libraries)
 * - Graph lifecycle management (rebuild on data change)
 * - Undo stack (last 5 destructive operations)
 */
class DataStore {
public:
    static DataStore& getInstance();

    // Prevent copying
    DataStore(const DataStore&) = delete;
    DataStore& operator=(const DataStore&) = delete;

    // Initialization
    bool loadAll();
    bool saveAll();

    // Airport CRUD
    bool addAirport(const Airport& airport);
    bool deleteAirport(const std::string& code);
    bool updateAirport(const Airport& airport);
    Airport* getAirport(const std::string& code);
    std::vector<Airport> getAllAirports() const;

    // Aircraft CRUD
    bool addAircraft(const Aircraft& aircraft);
    bool deleteAircraft(const std::string& id);
    bool updateAircraft(const Aircraft& aircraft);
    Aircraft* getAircraft(const std::string& id);
    std::vector<Aircraft> getAllAircraft() const;

    // Route CRUD
    bool addRoute(const Route& route);
    bool deleteRoute(const std::string& routeId);
    bool updateRoute(const Route& route);
    Route* getRoute(const std::string& routeId);
    std::vector<Route> getAllRoutes() const;

    // Flight CRUD
    bool addFlight(const Flight& flight);
    bool deleteFlight(const std::string& flightNum);
    Flight* getFlight(const std::string& flightNum);
    std::vector<Flight> getAllFlights() const;

    // Graph access
    Graph& getGraph();
    void rebuildGraph();

    // Undo system
    bool undo();
    bool canUndo() const;
    void clearUndoStack();

private:
    DataStore();

    // Data containers
    std::map<std::string, Airport> airports;
    std::map<std::string, Aircraft> aircraft;
    std::map<std::string, Route> routes;
    std::map<std::string, Flight> flights;

    // Graph for pathfinding
    Graph graph;

    // Undo stack (max 5 items)
    std::stack<Action> undoStack;
    static const int MAX_UNDO = 5;

    // File paths
    const std::string AIRPORTS_FILE = "data_files/airports.txt";
    const std::string AIRCRAFT_FILE = "data_files/aircraft.txt";
    const std::string ROUTES_FILE = "data_files/routes.txt";
    const std::string FLIGHTS_FILE = "data_files/flights.txt";

    // CSV I/O helpers
    bool loadAirports();
    bool loadAircraft();
    bool loadRoutes();
    bool loadFlights();
    bool saveAirports();
    bool saveAircraft();
    bool saveRoutes();
    bool saveFlights();

    // Undo helpers
    void pushUndo(const Action& action);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);

    // Serialization helpers for undo
    std::string serializeAirport(const Airport& a);
    Airport deserializeAirport(const std::string& data);
};
#endif // DATASTORE_H
