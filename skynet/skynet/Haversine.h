#ifndef HAVERSINE_H
#define HAVERSINE_H

/**
 * @brief Haversine formula for calculating great-circle distance
 *
 * Given two GPS coordinates (lat/lon), calculates shortest distance
 * over Earth's surface. Critical for realistic route distances.
 *
 * Formula: a = sin²(Δlat/2) + cos(lat1)·cos(lat2)·sin²(Δlon/2)
 *          c = 2·atan2(√a, √(1-a))
 *          distance = R · c
 *
 * where R = Earth's radius (6371 km)
 */
class Haversine {
public:
    /**
     * Calculate distance between two points in kilometers
     * @param lat1, lon1 First point coordinates (degrees)
     * @param lat2, lon2 Second point coordinates (degrees)
     * @return Distance in kilometers
     */
    static double calculate(double lat1, double lon1,
                            double lat2, double lon2);

private:
    static constexpr double EARTH_RADIUS_KM = 6371.0;
    static double toRadians(double degrees);
};
#endif // HAVERSINE_H
