#ifndef GRAPH_H
#define GRAPH_H
#include <string>
#include <vector>
#include <map>
#include <set>

/**
 * @brief Weighted directed graph using adjacency list
 *
 * Why adjacency list over matrix?
 * - Sparse graph (not all airports connect to all others)
 * - Memory efficient: O(V + E) vs O(V²)
 * - Fast neighbor iteration for Dijkstra
 *
 * Structure: map<string, vector<Edge>>
 * - Key: airport code (node)
 * - Value: list of outgoing edges with weights
 */

struct Edge {
    std::string destination;
    double weight;           // Distance in km
    double cost;            // Operational cost

    Edge(const std::string& dest, double w, double c = 0.0)
        : destination(dest), weight(w), cost(c) {}
};

class Graph {
public:
    Graph();

    // Node operations
    void addNode(const std::string& nodeId);
    void removeNode(const std::string& nodeId);
    bool hasNode(const std::string& nodeId) const;
    std::set<std::string> getNodes() const;

    // Edge operations (directed)
    void addEdge(const std::string& from, const std::string& to,
                 double weight, double cost = 0.0);
    void removeEdge(const std::string& from, const std::string& to);
    bool hasEdge(const std::string& from, const std::string& to) const;

    // Query operations
    std::vector<Edge> getNeighbors(const std::string& nodeId) const;
    int getNodeCount() const;
    int getEdgeCount() const;

    // Utility
    void clear();
    bool isEmpty() const;

private:
    // Adjacency list: airport code -> list of edges
    std::map<std::string, std::vector<Edge>> adjacencyList;

    // Quick lookup for node existence (could use adjacencyList.count() but this is clearer)
    std::set<std::string> nodes;
};

#endif // GRAPH_H
