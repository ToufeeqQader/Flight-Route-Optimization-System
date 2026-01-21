#ifndef AIRPORTS_H
#define AIRPORTS_H

#include <string>

struct Airport {
    std::string code;        // IATA code (e.g., "JFK")
    std::string name;        // Full airport name
    std::string city;        // City location
    std::string country;     // Country
    double latitude;         // GPS coordinate
    double longitude;        // GPS coordinate

    Airport() : latitude(0.0), longitude(0.0) {}

    Airport(const std::string& code, const std::string& name,
            const std::string& city, const std::string& country,
            double lat, double lon)
        : code(code), name(name), city(city), country(country),
        latitude(lat), longitude(lon) {}

    bool operator<(const Airport& other) const {
        return code < other.code;
    }

    bool operator==(const Airport& other) const {
        return code == other.code;
    }
};

#endif // AIRPORTS_H
