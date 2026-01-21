#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <string>

enum class AircraftStatus {
    AVAILABLE,
    IN_FLIGHT,
    MAINTENANCE,
    RETIRED
};

struct Aircraft {
    std::string id;
    std::string model;
    int capacity;
    double cruiseSpeed;
    double fuelConsumption;
    AircraftStatus status;

    Aircraft() : capacity(0), cruiseSpeed(0.0),
        fuelConsumption(0.0), status(AircraftStatus::AVAILABLE) {}

    Aircraft(const std::string& id, const std::string& model,
             int cap, double speed, double fuel)
        : id(id), model(model), capacity(cap), cruiseSpeed(speed),
        fuelConsumption(fuel), status(AircraftStatus::AVAILABLE) {}

    bool operator<(const Aircraft& other) const {
        return id < other.id;
    }

    bool isAvailable() const {
        return status == AircraftStatus::AVAILABLE;
    }

    static std::string statusToString(AircraftStatus s) {
        switch(s) {
        case AircraftStatus::AVAILABLE: return "AVAILABLE";
        case AircraftStatus::IN_FLIGHT: return "IN_FLIGHT";
        case AircraftStatus::MAINTENANCE: return "MAINTENANCE";
        case AircraftStatus::RETIRED: return "RETIRED";
        default: return "AVAILABLE";
        }
    }

    static AircraftStatus stringToStatus(const std::string& s) {
        if (s == "IN_FLIGHT") return AircraftStatus::IN_FLIGHT;
        if (s == "MAINTENANCE") return AircraftStatus::MAINTENANCE;
        if (s == "RETIRED") return AircraftStatus::RETIRED;
        return AircraftStatus::AVAILABLE;
    }
};
#endif // AIRCRAFT_H
