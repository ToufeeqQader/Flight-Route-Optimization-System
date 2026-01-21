#include "WeatherSimulator.h"
#include <random>

void WeatherSimulator::applyWeather(Graph& graph,
                                    const std::string& routeId,
                                    Condition condition) {
    WeatherImpact impact = getImpact(condition);

    // Modify graph edge weights based on weather
    // Implementation would adjust edge weights in graph
    // For now, this is a placeholder
}

WeatherSimulator::WeatherImpact
WeatherSimulator::getImpact(Condition condition) {
    WeatherImpact impact;

    switch (condition) {
    case Condition::CLEAR:
        impact.timeMultiplier = 1.0;
        impact.costMultiplier = 1.0;
        impact.operational = true;
        break;

    case Condition::CLOUDY:
        impact.timeMultiplier = 1.05;
        impact.costMultiplier = 1.02;
        impact.operational = true;
        break;

    case Condition::RAIN:
        impact.timeMultiplier = 1.15;
        impact.costMultiplier = 1.10;
        impact.operational = true;
        break;

    case Condition::STORM:
        impact.timeMultiplier = 1.5;
        impact.costMultiplier = 1.3;
        impact.operational = false; // Dangerous
        break;

    case Condition::SNOW:
        impact.timeMultiplier = 1.3;
        impact.costMultiplier = 1.2;
        impact.operational = true;
        break;
    }

    return impact;
}

void WeatherSimulator::simulateRandomWeather(Graph& graph) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 4);

    // Apply random weather to random routes
    // This is a simplified simulation
    for (const auto& node : graph.getNodes()) {
        Condition condition = static_cast<Condition>(dis(gen));
        // Apply to some routes from this node
    }
}
