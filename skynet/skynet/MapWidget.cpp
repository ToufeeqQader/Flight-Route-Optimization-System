#include "MapWidget.h"
#include "DataStore.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , hasPath(false)
    , scale(1.0)
    , offset(0, 0)
    , dragging(false)
{
    setMinimumSize(800, 600);
    setMouseTracking(true);
    calculateBounds();
}

void MapWidget::setOptimalPath(const PathResult& path) {
    currentPath = path;
    hasPath = path.found;
    update();
}

void MapWidget::clearOptimalPath() {
    hasPath = false;
    update();
}

void MapWidget::refresh() {
    calculateBounds();
    update();
}

void MapWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(240, 248, 255)); // Alice blue

    // Draw grid
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int x = 0; x < width(); x += 50) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += 50) {
        painter.drawLine(0, y, width(), y);
    }

    // Draw network
    drawRoutes(painter);
    drawOptimalPath(painter);
    drawAirports(painter);

    // Legend
    painter.setPen(Qt::black);
    painter.drawText(10, 20, "Map View - Flight Network");
    painter.drawText(10, 40, QString("Airports: %1").arg(
                                 DataStore::getInstance().getAllAirports().size()));
    painter.drawText(10, 60, QString("Routes: %1").arg(
                                 DataStore::getInstance().getAllRoutes().size()));
}

void MapWidget::drawAirports(QPainter& painter) {
    DataStore& store = DataStore::getInstance();
    auto airports = store.getAllAirports();

    for (const auto& airport : airports) {
        QPoint pos = latLonToScreen(airport.latitude, airport.longitude);

        // Check if in optimal path
        bool inPath = false;
        if (hasPath) {
            for (const auto& code : currentPath.path) {
                if (code == airport.code) {
                    inPath = true;
                    break;
                }
            }
        }

        // Draw node
        if (inPath) {
            painter.setBrush(QColor(255, 0, 0)); // Red for path
            painter.setPen(QPen(Qt::black, 2));
            painter.drawEllipse(pos, 8, 8);
        } else {
            painter.setBrush(QColor(70, 130, 180)); // Steel blue
            painter.setPen(QPen(Qt::black, 1));
            painter.drawEllipse(pos, 6, 6);
        }

        // Draw label
        painter.setPen(Qt::black);
        painter.drawText(pos.x() + 10, pos.y() + 5,
                         QString::fromStdString(airport.code));
    }
}

void MapWidget::drawRoutes(QPainter& painter) {
    DataStore& store = DataStore::getInstance();
    auto routes = store.getAllRoutes();
    auto airports = store.getAllAirports();

    // Create airport map for quick lookup
    std::map<std::string, Airport> airportMap;
    for (const auto& airport : airports) {
        airportMap[airport.code] = airport;
    }

    painter.setPen(QPen(QColor(100, 100, 100, 100), 1)); // Semi-transparent

    for (const auto& route : routes) {
        if (!route.operational) continue;

        auto it1 = airportMap.find(route.origin);
        auto it2 = airportMap.find(route.destination);

        if (it1 != airportMap.end() && it2 != airportMap.end()) {
            QPoint p1 = latLonToScreen(it1->second.latitude, it1->second.longitude);
            QPoint p2 = latLonToScreen(it2->second.latitude, it2->second.longitude);

            painter.drawLine(p1, p2);
        }
    }
}

void MapWidget::drawOptimalPath(QPainter& painter) {
    if (!hasPath || currentPath.path.size() < 2) return;

    DataStore& store = DataStore::getInstance();

    painter.setPen(QPen(Qt::red, 3));

    for (size_t i = 0; i < currentPath.path.size() - 1; ++i) {
        Airport* a1 = store.getAirport(currentPath.path[i]);
        Airport* a2 = store.getAirport(currentPath.path[i + 1]);

        if (a1 && a2) {
            QPoint p1 = latLonToScreen(a1->latitude, a1->longitude);
            QPoint p2 = latLonToScreen(a2->latitude, a2->longitude);

            painter.drawLine(p1, p2);

            // Draw arrow
            double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
            QPoint mid((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2);

            painter.setBrush(Qt::red);
            QPolygon arrow;
            arrow << mid
                  << mid + QPoint(-5 * std::cos(angle - M_PI/6), -5 * std::sin(angle - M_PI/6))
                  << mid + QPoint(-5 * std::cos(angle + M_PI/6), -5 * std::sin(angle + M_PI/6));
            painter.drawPolygon(arrow);
        }
    }
}

QPoint MapWidget::latLonToScreen(double lat, double lon) const {
    // Mercator projection (simplified)
    double x = (lon - minLon) / (maxLon - minLon) * width() * scale + offset.x();
    double y = (maxLat - lat) / (maxLat - minLat) * height() * scale + offset.y();

    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

void MapWidget::calculateBounds() {
    DataStore& store = DataStore::getInstance();
    auto airports = store.getAllAirports();

    if (airports.empty()) {
        minLat = -90; maxLat = 90;
        minLon = -180; maxLon = 180;
        return;
    }

    minLat = maxLat = airports[0].latitude;
    minLon = maxLon = airports[0].longitude;

    for (const auto& airport : airports) {
        minLat = std::min(minLat, airport.latitude);
        maxLat = std::max(maxLat, airport.latitude);
        minLon = std::min(minLon, airport.longitude);
        maxLon = std::max(maxLon, airport.longitude);
    }

    // Add padding
    double latPadding = (maxLat - minLat) * 0.1;
    double lonPadding = (maxLon - minLon) * 0.1;

    minLat -= latPadding;
    maxLat += latPadding;
    minLon -= lonPadding;
    maxLon += lonPadding;
}

void MapWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        lastMousePos = event->pos();
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (dragging) {
        QPoint delta = event->pos() - lastMousePos;
        offset += delta;
        lastMousePos = event->pos();
        update();
    }
}

void MapWidget::wheelEvent(QWheelEvent *event) {
    double delta = event->angleDelta().y() / 120.0;
    scale *= (1.0 + delta * 0.1);
    scale = std::max(0.1, std::min(scale, 5.0));
    update();
}
