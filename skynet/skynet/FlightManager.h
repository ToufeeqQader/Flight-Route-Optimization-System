#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include "PathResult.h"

class MapWidget;

class FlightManager : public QWidget {
    Q_OBJECT
public:
    explicit FlightManager(QWidget *parent = nullptr);
    void setMapWidget(MapWidget* map);
    void refreshData();

private slots:
    void onPlanFlight();
    void onBookFlight();
    void onRefreshFlights();
    void onPreviewRoute();
    void onClearSelection();
    void onDeleteFlight();

private:
    void setupUi();
    void showRoutePreview(const PathResult& result);
    bool validateInputs();

    QComboBox* originCombo;
    QComboBox* destCombo;
    QComboBox* aircraftCombo;
    QTextEdit* resultText;
    QTableWidget* flightTable;
    MapWidget* mapWidget;

    PathResult currentPath;
    bool hasPlannedRoute;
};

#endif // FLIGHTMANAGER_H
