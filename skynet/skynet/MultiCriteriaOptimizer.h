#ifndef MULTICRITERIAOPTIMIZER_H
#define MULTICRITERIAOPTIMIZER_H
#include "Graph.h"
#include "PathResult.h"
#include <vector>

/**
 * @brief Multi-objective optimization for flight routes
 *
 * Finds Pareto-optimal solutions balancing:
 * - Distance (minimize)
 * - Cost (minimize)
 * - Time (minimize)
 * - Comfort (maximize - based on stops)
 *
 * Uses weighted sum approach or Pareto frontier generation
 */
class MultiCriteriaOptimizer {
public:
    struct Criteria {
        double distanceWeight = 0.4;
        double costWeight = 0.3;
        double timeWeight = 0.3;
        double maxStops = 3;
    };

    /**
     * Find optimal path considering multiple criteria
     * @param graph Flight network
     * @param start Origin airport
     * @param end Destination airport
     * @param criteria Optimization weights
     * @return Best path according to weighted criteria
     */
    static PathResult optimize(const Graph& graph,
                               const std::string& start,
                               const std::string& end,
                               const Criteria& criteria);

    /**
     * Generate Pareto frontier of non-dominated solutions
     * @return Vector of paths representing tradeoff options
     */
    static std::vector<PathResult> getParetoFrontier(
        const Graph& graph,
        const std::string& start,
        const std::string& end);
};
#endif // MULTICRITERIAOPTIMIZER_H
