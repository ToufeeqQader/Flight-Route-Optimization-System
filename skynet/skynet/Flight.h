#ifndef FLIGHT_H
#define FLIGHT_H

#include <string>
#include <vector>

/**
 * @brief Represents a booked Flight with full route details
 *
 * A Flight is the result of path planning + booking.
 * It contains the complete journey from origin to destination,
 * including all intermediate stops, assigned aircraft, and costs.
 */
struct Flight {
    std::string flightNumber;        // Unique flight ID (e.g., "FL001")
    std::string aircraftId;          // Assigned aircraft
    std::vector<std::string> route;  // Ordered list of airport codes
    double totalDistance;            // Total km
    double totalCost;                // Fuel + operational costs
    double estimatedTime;            // Hours
    std::string departureTime;       // ISO format or simple string
    std::string status;              // SCHEDULED, COMPLETED, CANCELLED

    Flight() : totalDistance(0.0), totalCost(0.0), estimatedTime(0.0),
        status("SCHEDULED") {}

    Flight(const std::string& num, const std::string& aircraft,
           const std::vector<std::string>& r)
        : flightNumber(num), aircraftId(aircraft), route(r),
        totalDistance(0.0), totalCost(0.0), estimatedTime(0.0),
        status("SCHEDULED") {}

    std::string getOrigin() const {
        return route.empty() ? "" : route.front();
    }

    std::string getDestination() const {
        return route.empty() ? "" : route.back();
    }

    int getStops() const {
        return route.size() > 2 ? route.size() - 2 : 0;
    }
};

#endif // FLIGHT_H
