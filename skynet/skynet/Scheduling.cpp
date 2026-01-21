#include "Scheduling.h"

bool Scheduling::canSchedule(const Flight& flight,
                             const std::vector<Flight>& existingFlights) {
    // Check if aircraft is available
    for (const auto& existing : existingFlights) {
        if (existing.aircraftId == flight.aircraftId &&
            existing.status == "SCHEDULED") {
            // Simplified: assume conflict if same aircraft
            return false;
        }
    }
    return true;
}

std::vector<std::pair<std::string, std::string>>
Scheduling::detectConflicts(const std::vector<Flight>& flights) {
    std::vector<std::pair<std::string, std::string>> conflicts;

    for (size_t i = 0; i < flights.size(); ++i) {
        for (size_t j = i + 1; j < flights.size(); ++j) {
            if (flights[i].aircraftId == flights[j].aircraftId &&
                flights[i].status == "SCHEDULED" &&
                flights[j].status == "SCHEDULED") {
                conflicts.push_back({flights[i].flightNumber,
                                     flights[j].flightNumber});
            }
        }
    }

    return conflicts;
}

std::vector<Scheduling::TimeSlot>
Scheduling::generateGanttData(const std::vector<Flight>& flights) {
    std::vector<TimeSlot> slots;

    for (const auto& flight : flights) {
        TimeSlot slot;
        slot.aircraftId = flight.aircraftId;
        slot.startTime = flight.departureTime;
        slot.location = flight.getOrigin() + " → " + flight.getDestination();
        // Calculate end time based on estimated time
        slot.endTime = flight.departureTime; // Simplified

        slots.push_back(slot);
    }

    return slots;
}
