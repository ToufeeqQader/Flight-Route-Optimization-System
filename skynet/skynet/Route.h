#ifndef ROUTE_H
#define ROUTE_H

#include <string>

struct Route {
    std::string origin;
    std::string destination;
    double distance;
    double baseCost;
    bool operational;

    Route() : distance(0.0), baseCost(0.0), operational(true) {}

    Route(const std::string& from, const std::string& to,
          double dist, double cost = 0.0, bool op = true)
        : origin(from), destination(to), distance(dist),
        baseCost(cost), operational(op) {}

    std::string getId() const {
        return origin + "-" + destination;
    }

    Route reverse() const {
        return Route(destination, origin, distance, baseCost, operational);
    }
};

#endif // ROUTE_H
