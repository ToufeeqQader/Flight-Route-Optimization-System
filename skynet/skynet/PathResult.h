#ifndef PATHRESULT_H
#define PATHRESULT_H

#include <string>
#include <vector>

/**
 * @brief Result structure from Dijkstra's algorithm
 *
 * Encapsulates the complete path information after pathfinding.
 * UI layer consumes this without knowing algorithm internals.
 * Separation of concerns: algorithm produces PathResult,
 * UI renders it, business logic processes it.
 */
struct PathResult {
    bool found;                      // Was path found?
    std::vector<std::string> path;   // Ordered airport codes
    double totalDistance;            // Total km
    double totalCost;                // Estimated cost
    double estimatedTime;            // Hours
    std::string errorMessage;        // If not found, why?

    PathResult() : found(false), totalDistance(0.0),
        totalCost(0.0), estimatedTime(0.0) {}

    PathResult(bool f, const std::vector<std::string>& p,
               double dist, double cost, double time)
        : found(f), path(p), totalDistance(dist),
        totalCost(cost), estimatedTime(time) {}

    // Quick check if path exists and is valid
    bool isValid() const {
        return found && path.size() >= 2;
    }

    std::string getOrigin() const {
        return path.empty() ? "" : path.front();
    }

    std::string getDestination() const {
        return path.empty() ? "" : path.back();
    }
};
#endif // PATHRESULT_H
