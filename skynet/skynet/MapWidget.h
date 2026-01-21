#ifndef MAPWIDGET_H
#define MAPWIDGET_H
#include <QWidget>
#include <QPainter>
#include <QPoint>
#include <map>
#include <string>
#include "PathResult.h"

/**
 * @brief Visual representation of flight network
 *
 * Displays:
 * - Airports as nodes (circles)
 * - Routes as edges (lines)
 * - Optimal path in red
 * - Interactive zoom and pan
 */
class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

    void setOptimalPath(const PathResult& path);
    void clearOptimalPath();
    void refresh();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawAirports(QPainter& painter);
    void drawRoutes(QPainter& painter);
    void drawOptimalPath(QPainter& painter);

    QPoint latLonToScreen(double lat, double lon) const;
    void calculateBounds();

    PathResult currentPath;
    bool hasPath;

    // Viewport
    double minLat, maxLat, minLon, maxLon;
    double scale;
    QPoint offset;

    // Interaction
    bool dragging;
    QPoint lastMousePos;
};

#endif // MAPWIDGET_H
