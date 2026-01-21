#include "Dijkstra.h"
#include <queue>
#include <map>
#include <limits>
#include <algorithm>

PathResult Dijkstra::findShortestPath(const Graph& graph,
                                      const std::string& start,
                                      const std::string& end) {
    // Validate inputs
    if (!graph.hasNode(start)) {
        PathResult result;
        result.found = false;
        result.errorMessage = "Origin airport not found";
        return result;
    }

    if (!graph.hasNode(end)) {
        PathResult result;
        result.found = false;
        result.errorMessage = "Destination airport not found";
        return result;
    }

    if (start == end) {
        PathResult result;
        result.found = true;
        result.path = {start};
        result.totalDistance = 0.0;
        result.totalCost = 0.0;
        return result;
    }

    // Initialize data structures
    const double INF = std::numeric_limits<double>::infinity();

    // Priority queue: pair<distance, node>
    // Using greater to make it a min-heap (smallest distance on top)
    std::priority_queue<std::pair<double, std::string>,
                        std::vector<std::pair<double, std::string>>,
                        std::greater<std::pair<double, std::string>>> pq;

    std::map<std::string, double> distances;      // Best known distance to each node
    std::map<std::string, double> costs;          // Accumulated costs
    std::map<std::string, std::string> parent;    // Parent node in optimal path
    std::set<std::string> visited;                // Processed nodes

    // Initialize all distances to infinity
    for (const auto& node : graph.getNodes()) {
        distances[node] = INF;
        costs[node] = 0.0;
    }

    // Start node has distance 0
    distances[start] = 0.0;
    pq.push({0.0, start});

    // Main algorithm loop
    while (!pq.empty()) {
        // Extract node with minimum distance
        auto [currentDist, current] = pq.top();
        pq.pop();

        // If we reached destination, we're done (Dijkstra guarantees optimal)
        if (current == end) {
            break;
        }

        // Skip if already visited (can have duplicates in priority queue)
        if (visited.count(current)) {
            continue;
        }
        visited.insert(current);

        // Explore all neighbors
        for (const auto& edge : graph.getNeighbors(current)) {
            const std::string& neighbor = edge.destination;
            double newDist = distances[current] + edge.weight;

            // Relaxation: found shorter path to neighbor
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                costs[neighbor] = costs[current] + edge.cost;
                parent[neighbor] = current;
                pq.push({newDist, neighbor});
            }
        }
    }

    // Check if destination was reached
    if (distances[end] == INF) {
        PathResult result;
        result.found = false;
        result.errorMessage = "No route available between airports";
        return result;
    }

    // Reconstruct path
    std::vector<std::string> path = reconstructPath(parent, start, end);

    // Build result
    PathResult result;
    result.found = true;
    result.path = path;
    result.totalDistance = distances[end];
    result.totalCost = costs[end];

    return result;
}

std::vector<std::string> Dijkstra::reconstructPath(
    const std::map<std::string, std::string>& parent,
    const std::string& start,
    const std::string& end) {

    std::vector<std::string> path;
    std::string current = end;

    // Backtrack from destination to source
    while (current != start) {
        path.push_back(current);

        // If no parent exists, path is broken (shouldn't happen if properly validated)
        if (parent.find(current) == parent.end()) {
            return {};
        }

        current = parent.at(current);
    }

    path.push_back(start);

    // Reverse to get path from start to end
    std::reverse(path.begin(), path.end());

    return path;
}
