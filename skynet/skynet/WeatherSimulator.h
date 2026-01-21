#ifndef WEATHERSIMULATOR_H
#define WEATHERSIMULATOR_H
#include "Graph.h"
#include <string>

/**
 * @brief Weather simulation affecting route viability
 *
 * Simulates weather conditions that affect:
 * - Route operational status
 * - Flight time (headwinds/tailwinds)
 * - Safety margins
 * - Fuel consumption
 */
class WeatherSimulator {
public:
    enum class Condition {
        CLEAR,
        CLOUDY,
        RAIN,
        STORM,
        SNOW
    };

    struct WeatherImpact {
        double timeMultiplier;     // 1.0 = normal, >1 = slower
        double costMultiplier;     // Fuel consumption change
        bool operational;          // Can route be used?
    };

    /**
     * Apply weather effects to graph
     */
    static void applyWeather(Graph& graph,
                             const std::string& routeId,
                             Condition condition);

    /**
     * Get weather impact for a condition
     */
    static WeatherImpact getImpact(Condition condition);

    /**
     * Simulate random weather for all routes
     */
    static void simulateRandomWeather(Graph& graph);
};

#endif // WEATHERSIMULATOR_H
