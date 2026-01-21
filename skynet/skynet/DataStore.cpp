#include "DataStore.h"
#include "Haversine.h"
#include <fstream>
#include <sstream>  // CRITICAL: Required for flight parsing
//#include <algorithm>
#include <iostream>
#include <filesystem>

DataStore& DataStore::getInstance() {
    static DataStore instance;
    return instance;
}

DataStore::DataStore() {}

bool DataStore::loadAll() {
    try {
        std::filesystem::create_directories("data_files");

        bool success = true;
        success &= loadAirports();
        success &= loadAircraft();
        success &= loadRoutes();
        success &= loadFlights();

        if (success) {
            rebuildGraph();
            std::cout << "✓ All data loaded successfully" << std::endl;
        }

        return success;
    } catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << std::endl;
        return false;
    }
}

bool DataStore::saveAll() {
    try {
        bool success = true;
        success &= saveAirports();
        success &= saveAircraft();
        success &= saveRoutes();
        success &= saveFlights();

        if (success) {
            std::cout << "✓ All data saved successfully" << std::endl;
        }

        return success;
    } catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << std::endl;
        return false;
    }
}

// ==================== AIRPORT CRUD ====================

bool DataStore::addAirport(const Airport& airport) {
    if (airports.find(airport.code) != airports.end()) {
        return false;
    }

    airports[airport.code] = airport;
    pushUndo(Action(ActionType::ADD_AIRPORT, serializeAirport(airport)));
    rebuildGraph();
    return true;
}

bool DataStore::deleteAirport(const std::string& code) {
    auto it = airports.find(code);
    if (it == airports.end()) {
        return false;
    }

    pushUndo(Action(ActionType::DELETE_AIRPORT, serializeAirport(it->second)));

    // Remove all routes involving this airport
    std::vector<std::string> routesToDelete;
    for (const auto& [id, route] : routes) {
        if (route.origin == code || route.destination == code) {
            routesToDelete.push_back(id);
        }
    }
    for (const auto& id : routesToDelete) {
        routes.erase(id);
    }

    airports.erase(it);
    rebuildGraph();
    return true;
}

bool DataStore::updateAirport(const Airport& airport) {
    if (airports.find(airport.code) == airports.end()) {
        return false;
    }

    airports[airport.code] = airport;
    rebuildGraph();
    return true;
}

Airport* DataStore::getAirport(const std::string& code) {
    auto it = airports.find(code);
    return (it != airports.end()) ? &it->second : nullptr;
}

std::vector<Airport> DataStore::getAllAirports() const {
    std::vector<Airport> result;
    for (const auto& pair : airports) {
        result.push_back(pair.second);
    }
    return result;
}

// ==================== AIRCRAFT CRUD ====================

bool DataStore::addAircraft(const Aircraft& ac) {
    if (aircraft.find(ac.id) != aircraft.end()) {
        return false;
    }

    aircraft[ac.id] = ac;
    pushUndo(Action(ActionType::ADD_AIRCRAFT, ac.id));
    return true;
}

bool DataStore::deleteAircraft(const std::string& id) {
    auto it = aircraft.find(id);
    if (it == aircraft.end()) {
        return false;
    }

    pushUndo(Action(ActionType::DELETE_AIRCRAFT, id));
    aircraft.erase(it);
    return true;
}

bool DataStore::updateAircraft(const Aircraft& ac) {
    if (aircraft.find(ac.id) == aircraft.end()) {
        return false;
    }

    aircraft[ac.id] = ac;
    return true;
}

Aircraft* DataStore::getAircraft(const std::string& id) {
    auto it = aircraft.find(id);
    return (it != aircraft.end()) ? &it->second : nullptr;
}

std::vector<Aircraft> DataStore::getAllAircraft() const {
    std::vector<Aircraft> result;
    for (const auto& pair : aircraft) {
        result.push_back(pair.second);
    }
    return result;
}

// ==================== ROUTE CRUD ====================

bool DataStore::addRoute(const Route& route) {
    std::string id = route.getId();
    if (routes.find(id) != routes.end()) {
        return false;
    }

    routes[id] = route;
    pushUndo(Action(ActionType::ADD_ROUTE, id));
    rebuildGraph();
    return true;
}

bool DataStore::deleteRoute(const std::string& routeId) {
    auto it = routes.find(routeId);
    if (it == routes.end()) {
        return false;
    }

    pushUndo(Action(ActionType::DELETE_ROUTE, routeId));
    routes.erase(it);
    rebuildGraph();
    return true;
}

bool DataStore::updateRoute(const Route& route) {
    std::string id = route.getId();
    if (routes.find(id) == routes.end()) {
        return false;
    }

    routes[id] = route;
    rebuildGraph();
    return true;
}

Route* DataStore::getRoute(const std::string& routeId) {
    auto it = routes.find(routeId);
    return (it != routes.end()) ? &it->second : nullptr;
}

std::vector<Route> DataStore::getAllRoutes() const {
    std::vector<Route> result;
    for (const auto& pair : routes) {
        result.push_back(pair.second);
    }
    return result;
}

// ==================== FLIGHT CRUD ====================

bool DataStore::addFlight(const Flight& flight) {
    if (flights.find(flight.flightNumber) != flights.end()) {
        return false;
    }

    flights[flight.flightNumber] = flight;
    pushUndo(Action(ActionType::ADD_FLIGHT, flight.flightNumber));
    return true;
}

bool DataStore::deleteFlight(const std::string& flightNum) {
    auto it = flights.find(flightNum);
    if (it == flights.end()) {
        return false;
    }

    pushUndo(Action(ActionType::DELETE_FLIGHT, flightNum));
    flights.erase(it);
    return true;
}

Flight* DataStore::getFlight(const std::string& flightNum) {
    auto it = flights.find(flightNum);
    return (it != flights.end()) ? &it->second : nullptr;
}

std::vector<Flight> DataStore::getAllFlights() const {
    std::vector<Flight> result;
    for (const auto& pair : flights) {
        result.push_back(pair.second);
    }
    return result;
}

// ==================== GRAPH MANAGEMENT ====================

Graph& DataStore::getGraph() {
    return graph;
}

void DataStore::rebuildGraph() {
    graph.clear();

    // Register all airport nodes
    for (const auto& [code, airport] : airports) {
        graph.addNode(code);
    }

    // Add all operational routes as bidirectional edges
    for (const auto& [id, route] : routes) {
        if (route.operational) {
            graph.addEdge(route.origin, route.destination,
                          route.distance, route.baseCost);
            graph.addEdge(route.destination, route.origin,
                          route.distance, route.baseCost);
        }
    }

    std::cout << "✓ Graph rebuilt: " << graph.getNodeCount()
              << " nodes, " << graph.getEdgeCount() << " edges" << std::endl;
}

// ==================== UNDO SYSTEM ====================

bool DataStore::undo() {
    if (undoStack.empty()) {
        return false;
    }

    undoStack.pop();
    return true;
}

bool DataStore::canUndo() const {
    return !undoStack.empty();
}

void DataStore::clearUndoStack() {
    while (!undoStack.empty()) {
        undoStack.pop();
    }
}

void DataStore::pushUndo(const Action& action) {
    undoStack.push(action);

    if (undoStack.size() > MAX_UNDO) {
        std::stack<Action> temp;
        while (undoStack.size() > 1) {
            temp.push(undoStack.top());
            undoStack.pop();
        }
        undoStack.pop();
        while (!temp.empty()) {
            undoStack.push(temp.top());
            temp.pop();
        }
    }
}

// ==================== CSV LOADING ====================

bool DataStore::loadAirports() {
    std::ifstream file(AIRPORTS_FILE);
    if (!file.is_open()) {
        std::cout << "⚠ airports.txt not found, creating empty file" << std::endl;
        return true;
    }

    std::string line;
    std::getline(file, line); // Skip header

    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto parts = split(line, ',');
        if (parts.size() >= 6) {
            try {
                Airport airport;
                airport.code = trim(parts[0]);
                airport.name = trim(parts[1]);
                airport.city = trim(parts[2]);
                airport.country = trim(parts[3]);
                airport.latitude = std::stod(trim(parts[4]));
                airport.longitude = std::stod(trim(parts[5]));
                airports[airport.code] = airport;
                count++;
            } catch (...) {
                std::cerr << "Error parsing airport line: " << line << std::endl;
            }
        }
    }

    file.close();
    std::cout << "✓ Loaded " << count << " airports" << std::endl;
    return true;
}

bool DataStore::loadAircraft() {
    std::ifstream file(AIRCRAFT_FILE);
    if (!file.is_open()) {
        std::cout << "⚠ aircraft.txt not found, creating empty file" << std::endl;
        return true;
    }

    std::string line;
    std::getline(file, line); // Skip header

    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto parts = split(line, ',');
        if (parts.size() >= 6) {
            try {
                Aircraft ac;
                ac.id = trim(parts[0]);
                ac.model = trim(parts[1]);
                ac.capacity = std::stoi(trim(parts[2]));
                ac.cruiseSpeed = std::stod(trim(parts[3]));
                ac.fuelConsumption = std::stod(trim(parts[4]));
                ac.status = Aircraft::stringToStatus(trim(parts[5]));
                aircraft[ac.id] = ac;
                count++;
            } catch (...) {
                std::cerr << "Error parsing aircraft line: " << line << std::endl;
            }
        }
    }

    file.close();
    std::cout << "✓ Loaded " << count << " aircraft" << std::endl;
    return true;
}

bool DataStore::loadRoutes() {
    std::ifstream file(ROUTES_FILE);
    if (!file.is_open()) {
        std::cout << "⚠ routes.txt not found, creating empty file" << std::endl;
        return true;
    }

    std::string line;
    std::getline(file, line); // Skip header

    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto parts = split(line, ',');
        if (parts.size() >= 5) {
            try {
                Route route;
                route.origin = trim(parts[0]);
                route.destination = trim(parts[1]);
                route.distance = std::stod(trim(parts[2]));
                route.baseCost = std::stod(trim(parts[3]));
                route.operational = (trim(parts[4]) == "1" || trim(parts[4]) == "true");
                routes[route.getId()] = route;
                count++;
            } catch (...) {
                std::cerr << "Error parsing route line: " << line << std::endl;
            }
        }
    }

    file.close();
    std::cout << "✓ Loaded " << count << " routes" << std::endl;
    return true;
}

bool DataStore::loadFlights() {
    std::ifstream file(FLIGHTS_FILE);
    if (!file.is_open()) {
        std::cout << "⚠ flights.txt not found, creating empty file" << std::endl;
        return true;
    }

    std::string line;
    std::getline(file, line); // Skip header

    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto parts = split(line, ',');
        if (parts.size() >= 8) {
            try {
                Flight flight;
                flight.flightNumber = trim(parts[0]);
                flight.aircraftId = trim(parts[1]);

                // Parse route (format: JFK-LHR-DXB)
                std::string routeStr = trim(parts[2]);
                std::stringstream ss(routeStr);
                std::string airport;
                while (std::getline(ss, airport, '-')) {
                    flight.route.push_back(trim(airport));
                }

                flight.totalDistance = std::stod(trim(parts[3]));
                flight.totalCost = std::stod(trim(parts[4]));
                flight.estimatedTime = std::stod(trim(parts[5]));
                flight.departureTime = trim(parts[6]);
                flight.status = trim(parts[7]);

                flights[flight.flightNumber] = flight;
                count++;
            } catch (...) {
                std::cerr << "Error parsing flight line: " << line << std::endl;
            }
        }
    }

    file.close();
    std::cout << "✓ Loaded " << count << " flights" << std::endl;
    return true;
}

// ==================== CSV SAVING ====================

bool DataStore::saveAirports() {
    std::ofstream file(AIRPORTS_FILE);
    if (!file.is_open()) {
        return false;
    }

    file << "Code,Name,City,Country,Latitude,Longitude\n";
    for (const auto& [code, airport] : airports) {
        file << airport.code << ","
             << airport.name << ","
             << airport.city << ","
             << airport.country << ","
             << airport.latitude << ","
             << airport.longitude << "\n";
    }

    file.close();
    return true;
}

bool DataStore::saveAircraft() {
    std::ofstream file(AIRCRAFT_FILE);
    if (!file.is_open()) {
        return false;
    }

    file << "ID,Model,Capacity,CruiseSpeed,FuelConsumption,Status\n";
    for (const auto& [id, ac] : aircraft) {
        file << ac.id << ","
             << ac.model << ","
             << ac.capacity << ","
             << ac.cruiseSpeed << ","
             << ac.fuelConsumption << ","
             << Aircraft::statusToString(ac.status) << "\n";
    }

    file.close();
    return true;
}

bool DataStore::saveRoutes() {
    std::ofstream file(ROUTES_FILE);
    if (!file.is_open()) {
        return false;
    }

    file << "Origin,Destination,Distance,BaseCost,Operational\n";
    for (const auto& [id, route] : routes) {
        file << route.origin << ","
             << route.destination << ","
             << route.distance << ","
             << route.baseCost << ","
             << (route.operational ? "1" : "0") << "\n";
    }

    file.close();
    return true;
}

bool DataStore::saveFlights() {
    std::ofstream file(FLIGHTS_FILE);
    if (!file.is_open()) {
        return false;
    }

    file << "FlightNumber,AircraftID,Route,TotalDistance,TotalCost,EstimatedTime,DepartureTime,Status\n";

    for (const auto& [flightNum, flight] : flights) {
        file << flight.flightNumber << ",";
        file << flight.aircraftId << ",";

        // Write route as: JFK-LHR-DXB
        for (size_t i = 0; i < flight.route.size(); ++i) {
            file << flight.route[i];
            if (i < flight.route.size() - 1) {
                file << "-";
            }
        }
        file << ",";

        file << flight.totalDistance << ",";
        file << flight.totalCost << ",";
        file << flight.estimatedTime << ",";
        file << flight.departureTime << ",";
        file << flight.status << "\n";
    }

    file.close();
    return true;
}

// ==================== UTILITY FUNCTIONS ====================

std::vector<std::string> DataStore::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string DataStore::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string DataStore::serializeAirport(const Airport& a) {
    return a.code + "," + a.name + "," + a.city + "," + a.country;
}

Airport DataStore::deserializeAirport(const std::string& data) {
    auto parts = split(data, ',');
    Airport a;
    if (parts.size() >= 4) {
        a.code = parts[0];
        a.name = parts[1];
        a.city = parts[2];
        a.country = parts[3];
    }
    return a;
}
