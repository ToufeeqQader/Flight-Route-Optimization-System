#include "Graph.h"
#include <algorithm>

Graph::Graph() {}

void Graph::addNode(const std::string& nodeId) {
    if (!hasNode(nodeId)) {
        nodes.insert(nodeId);
        adjacencyList[nodeId] = std::vector<Edge>();
    }
}

void Graph::removeNode(const std::string& nodeId) {
    if (!hasNode(nodeId)) return;

    // Remove all edges pointing to this node
    for (auto& pair : adjacencyList) {
        auto& edges = pair.second;
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                           [&nodeId](const Edge& e) { return e.destination == nodeId; }),
            edges.end()
            );
    }

    // Remove node's own adjacency list
    adjacencyList.erase(nodeId);
    nodes.erase(nodeId);
}

bool Graph::hasNode(const std::string& nodeId) const {
    return nodes.find(nodeId) != nodes.end();
}

std::set<std::string> Graph::getNodes() const {
    return nodes;
}

void Graph::addEdge(const std::string& from, const std::string& to,
                    double weight, double cost) {
    // Ensure both nodes exist
    addNode(from);
    addNode(to);

    // Check if edge already exists, update if so
    auto& edges = adjacencyList[from];
    for (auto& edge : edges) {
        if (edge.destination == to) {
            edge.weight = weight;
            edge.cost = cost;
            return;
        }
    }

    // Add new edge
    edges.emplace_back(to, weight, cost);
}

void Graph::removeEdge(const std::string& from, const std::string& to) {
    if (!hasNode(from)) return;

    auto& edges = adjacencyList[from];
    edges.erase(
        std::remove_if(edges.begin(), edges.end(),
                       [&to](const Edge& e) { return e.destination == to; }),
        edges.end()
        );
}

bool Graph::hasEdge(const std::string& from, const std::string& to) const {
    if (!hasNode(from)) return false;

    const auto& edges = adjacencyList.at(from);
    return std::any_of(edges.begin(), edges.end(),
                       [&to](const Edge& e) { return e.destination == to; });
}

std::vector<Edge> Graph::getNeighbors(const std::string& nodeId) const {
    if (!hasNode(nodeId)) return {};
    return adjacencyList.at(nodeId);
}

int Graph::getNodeCount() const {
    return nodes.size();
}

int Graph::getEdgeCount() const {
    int count = 0;
    for (const auto& pair : adjacencyList) {
        count += pair.second.size();
    }
    return count;
}

void Graph::clear() {
    adjacencyList.clear();
    nodes.clear();
}

bool Graph::isEmpty() const {
    return nodes.empty();
}
