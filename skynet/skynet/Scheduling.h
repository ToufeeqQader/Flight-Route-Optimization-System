#ifndef SCHEDULING_H
#define SCHEDULING_H
#include "Flight.h"
#include <vector>
#include <string>

/**
 * @brief Flight scheduling and conflict detection
 *
 * Manages:
 * - Time slot allocation
 * - Aircraft scheduling
 * - Gate assignments
 * - Conflict detection and resolution
 */
class Scheduling {
public:
    struct TimeSlot {
        std::string aircraftId;
        std::string startTime;
        std::string endTime;
        std::string location;
    };

    /**
     * Check if a flight can be scheduled without conflicts
     */
    static bool canSchedule(const Flight& flight,
                            const std::vector<Flight>& existingFlights);

    /**
     * Detect scheduling conflicts
     * @return Vector of conflicting flight pairs
     */
    static std::vector<std::pair<std::string, std::string>>
    detectConflicts(const std::vector<Flight>& flights);

    /**
     * Generate Gantt chart data for visualization
     */
    static std::vector<TimeSlot> generateGanttData(
        const std::vector<Flight>& flights);
};
#endif // SCHEDULING_H
