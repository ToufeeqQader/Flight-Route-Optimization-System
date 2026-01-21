#include "MultiCriteriaOptimizer.h"
#include "Dijkstra.h"

PathResult MultiCriteriaOptimizer::optimize(
    const Graph& graph,
    const std::string& start,
    const std::string& end,
    const Criteria& criteria) {

    // For now, use standard Dijkstra
    // Future: Implement weighted multi-criteria search
    PathResult result = Dijkstra::findShortestPath(graph, start, end);

    // Apply criteria filters
    if (result.found && result.path.size() - 2 > criteria.maxStops) {
        result.found = false;
        result.errorMessage = "Exceeds maximum stops constraint";
    }

    return result;
}

std::vector<PathResult> MultiCriteriaOptimizer::getParetoFrontier(
    const Graph& graph,
    const std::string& start,
    const std::string& end) {

    std::vector<PathResult> frontier;

    // Generate multiple paths with different criteria weights
    Criteria c1{1.0, 0.0, 0.0, 5}; // Distance priority
    Criteria c2{0.0, 1.0, 0.0, 5}; // Cost priority
    Criteria c3{0.0, 0.0, 1.0, 5}; // Time priority

    frontier.push_back(optimize(graph, start, end, c1));
    frontier.push_back(optimize(graph, start, end, c2));
    frontier.push_back(optimize(graph, start, end, c3));

    return frontier;
}
