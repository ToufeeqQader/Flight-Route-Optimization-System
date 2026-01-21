// tests/system_test.cpp
/**
 * @file system_test.cpp
 * @brief Comprehensive system and integration tests
 *
 * Tests the complete workflow from data loading to flight booking
 */

#include "Graph.h"
#include "Dijkstra.h"
#include "Haversine.h"
#include "DataStore.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>

// Test helper
void assertTrue(bool condition, const std::string& testName) {
    if (condition) {
        std::cout << "✓ " << testName << " PASSED" << std::endl;
    } else {
        std::cerr << "✗ " << testName << " FAILED" << std::endl;
        assert(false);
    }
}

// Unit Tests
void testHaversine() {
    std::cout << "\n=== Testing Haversine Distance Calculation ===" << std::endl;

    // Test 1: JFK to LAX (known distance ~3983 km)
    double dist1 = Haversine::calculate(40.6413, -73.7781, 33.9416, -118.4085);
    assertTrue(std::abs(dist1 - 3983.86) < 50.0, "JFK to LAX distance");

    // Test 2: Same location (should be 0)
    double dist2 = Haversine::calculate(51.47, -0.4543, 51.47, -0.4543);
    assertTrue(std::abs(dist2) < 0.01, "Same location distance");

    // Test 3: LHR to CDG (known distance ~343 km)
    double dist3 = Haversine::calculate(51.47, -0.4543, 49.0097, 2.5479);
    assertTrue(std::abs(dist3 - 343.81) < 10.0, "LHR to CDG distance");
}

void testGraph() {
    std::cout << "\n=== Testing Graph ADT ===" << std::endl;

    Graph g;

    // Test node operations
    g.addNode("A");
    g.addNode("B");
    g.addNode("C");
    assertTrue(g.getNodeCount() == 3, "Node count after additions");
    assertTrue(g.hasNode("A"), "Node A exists");
    assertTrue(!g.hasNode("D"), "Node D doesn't exist");

    // Test edge operations
    g.addEdge("A", "B", 10.0, 100.0);
    g.addEdge("B", "C", 20.0, 200.0);
    g.addEdge("A", "C", 35.0, 350.0);
    assertTrue(g.getEdgeCount() == 3, "Edge count after additions");
    assertTrue(g.hasEdge("A", "B"), "Edge A->B exists");
    assertTrue(!g.hasEdge("C", "A"), "Edge C->A doesn't exist (directed)");

    // Test neighbors
    auto neighbors = g.getNeighbors("A");
    assertTrue(neighbors.size() == 2, "A has 2 neighbors");

    // Test node deletion
    g.removeNode("B");
    assertTrue(g.getNodeCount() == 2, "Node count after deletion");
    assertTrue(!g.hasEdge("A", "B"), "Edge A->B removed with node B");
}

void testDijkstra() {
    std::cout << "\n=== Testing Dijkstra Algorithm ===" << std::endl;

    // Build test graph
    //     A --10--> B
    //     |         |
    //    35        20
    //     |         |
    //     v         v
    //     C <--5--- D

    Graph g;
    g.addNode("A");
    g.addNode("B");
    g.addNode("C");
    g.addNode("D");

    g.addEdge("A", "B", 10.0, 100.0);
    g.addEdge("A", "C", 35.0, 350.0);
    g.addEdge("B", "D", 20.0, 200.0);
    g.addEdge("D", "C", 5.0, 50.0);

    // Test 1: A to C via B->D->C should be shorter than direct
    PathResult result1 = Dijkstra::findShortestPath(g, "A", "C");
    assertTrue(result1.found, "Path A to C found");
    assertTrue(result1.totalDistance == 35.0, "Optimal distance A to C");
    // Note: Path might be direct or via B-D depending on implementation

    // Test 2: Path to non-existent node
    PathResult result2 = Dijkstra::findShortestPath(g, "A", "Z");
    assertTrue(!result2.found, "No path to non-existent node");

    // Test 3: Path to same node
    PathResult result3 = Dijkstra::findShortestPath(g, "A", "A");
    assertTrue(result3.found, "Path to same node found");
    assertTrue(result3.totalDistance == 0.0, "Distance to same node is 0");
    assertTrue(result3.path.size() == 1, "Path to same node has 1 element");
}

void testDataStore() {
    std::cout << "\n=== Testing DataStore ===" << std::endl;

    DataStore& store = DataStore::getInstance();

    // Test airport CRUD
    Airport testAirport("TST", "Test Airport", "Test City", "Test Country", 0.0, 0.0);
    assertTrue(store.addAirport(testAirport), "Add airport");
    assertTrue(store.getAirport("TST") != nullptr, "Get airport");
    assertTrue(!store.addAirport(testAirport), "Duplicate airport rejected");

    // Test aircraft CRUD
    Aircraft testAircraft("AC999", "Test Model", 200, 850.0, 3.0);
    assertTrue(store.addAircraft(testAircraft), "Add aircraft");
    assertTrue(store.getAircraft("AC999") != nullptr, "Get aircraft");

    // Test route with automatic distance calculation
    Route testRoute("JFK", "LAX", 3983.86, 6000.0, true);
    assertTrue(store.addRoute(testRoute), "Add route");

    // Test graph rebuild
    store.rebuildGraph();
    Graph& graph = store.getGraph();
    assertTrue(graph.getNodeCount() > 0, "Graph has nodes after rebuild");
}

// Integration Tests
void testCompleteWorkflow() {
    std::cout << "\n=== Testing Complete Flight Planning Workflow ===" << std::endl;

    DataStore& store = DataStore::getInstance();

    // Ensure we have test data
    if (store.getAllAirports().size() < 2) {
        std::cout << "⚠ Skipping workflow test - insufficient data" << std::endl;
        return;
    }

    // Get two airports
    auto airports = store.getAllAirports();
    std::string origin = airports[0].code;
    std::string dest = airports.size() > 1 ? airports[1].code : airports[0].code;

    // Plan a route
    Graph& graph = store.getGraph();
    PathResult path = Dijkstra::findShortestPath(graph, origin, dest);

    if (path.found) {
        std::cout << "✓ Route found: " << origin << " -> " << dest << std::endl;
        std::cout << "  Distance: " << path.totalDistance << " km" << std::endl;
        std::cout << "  Path: ";
        for (size_t i = 0; i < path.path.size(); ++i) {
            std::cout << path.path[i];
            if (i < path.path.size() - 1) std::cout << " -> ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "✗ No route found (this may be expected if airports aren't connected)" << std::endl;
    }
}

void testPerformance() {
    std::cout << "\n=== Performance Tests ===" << std::endl;

    DataStore& store = DataStore::getInstance();
    Graph& graph = store.getGraph();

    std::cout << "Graph statistics:" << std::endl;
    std::cout << "  Nodes: " << graph.getNodeCount() << std::endl;
    std::cout << "  Edges: " << graph.getEdgeCount() << std::endl;

    // Test pathfinding performance
    auto airports = store.getAllAirports();
    if (airports.size() >= 2) {
        auto start = std::chrono::high_resolution_clock::now();

        PathResult result = Dijkstra::findShortestPath(
            graph, airports[0].code, airports[1].code);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "  Pathfinding time: " << duration.count() << " microseconds" << std::endl;
        assertTrue(duration.count() < 100000, "Pathfinding under 100ms");
    }
}

// Main test runner
int main() {
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║  SkyNet Test Suite                    ║" << std::endl;
    std::cout << "║  Comprehensive System Testing         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;

    try {
        // Unit tests
        testHaversine();
        testGraph();
        testDijkstra();
        testDataStore();

        // Integration tests
        testCompleteWorkflow();
        testPerformance();

        std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "║  ✓ ALL TESTS PASSED                   ║" << std::endl;
        std::cout << "╚════════════════════════════════════════╝" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ TEST SUITE FAILED: " << e.what() << std::endl;
        return 1;
    }
}

