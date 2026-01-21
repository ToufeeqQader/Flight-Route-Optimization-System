#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include "Graph.h"
#include "PathResult.h"
#include <string>

/**
 * @brief Dijkstra's shortest path algorithm implementation
 *
 * Why Dijkstra?
 * - Finds shortest path in weighted graph
 * - Non-negative weights (distances are always positive)
 * - Optimal for single-source shortest path
 * - O((V + E) log V) with priority queue
 *
 * Why priority_queue?
 * - Always extract minimum distance node efficiently
 * - O(log V) insertion and extraction
 * - Better than linear search through all nodes: O(V²) → O((V+E) log V)
 *
 * Algorithm:
 * 1. Initialize distances to infinity, source to 0
 * 2. Use min-heap to always process nearest unvisited node
 * 3. For each neighbor, relax edge if shorter path found
 * 4. Reconstruct path by backtracking through parent pointers
 */
class Dijkstra {
public:
    /**
     * Find shortest path between two airports
     * @param graph Flight network graph
     * @param start Origin airport code
     * @param end Destination airport code
     * @return PathResult with complete path information
     */
    static PathResult findShortestPath(const Graph& graph,
                                       const std::string& start,
                                       const std::string& end);

private:
    // Reconstruct path from parent map
    static std::vector<std::string> reconstructPath(
        const std::map<std::string, std::string>& parent,
        const std::string& start,
        const std::string& end);
};

#endif // DIJKSTRA_H
