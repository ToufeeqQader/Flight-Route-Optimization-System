#include "FlightManager.h"
#include "DataStore.h"
#include "Dijkstra.h"
#include "aircraft.h"
#include "MapWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QProgressDialog>
#include <QDateTime>

FlightManager::FlightManager(QWidget *parent)
    : QWidget(parent), mapWidget(nullptr), hasPlannedRoute(false) {
    setupUi();
}

void FlightManager::setMapWidget(MapWidget* map) {
    mapWidget = map;
}

void FlightManager::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ==================== PLANNING SECTION ====================
    QGroupBox* planGroup = new QGroupBox("✈️ Flight Route Planning");
    QVBoxLayout* planMainLayout = new QVBoxLayout(planGroup);

    QFormLayout* planForm = new QFormLayout();

    originCombo = new QComboBox();
    destCombo = new QComboBox();
    aircraftCombo = new QComboBox();

    originCombo->setMinimumWidth(300);
    destCombo->setMinimumWidth(300);
    aircraftCombo->setMinimumWidth(300);

    // Load data
    DataStore& store = DataStore::getInstance();

    originCombo->addItem("-- Select Origin Airport --", "");
    destCombo->addItem("-- Select Destination Airport --", "");
    aircraftCombo->addItem("-- Select Aircraft --", "");

    for (const auto& airport : store.getAllAirports()) {
        QString display = QString("[%1] %2, %3")
        .arg(QString::fromStdString(airport.code))
            .arg(QString::fromStdString(airport.name))
            .arg(QString::fromStdString(airport.city));
        QString code = QString::fromStdString(airport.code);

        originCombo->addItem(display, code);
        destCombo->addItem(display, code);
    }

    for (const auto& ac : store.getAllAircraft()) {
        if (ac.isAvailable()) {
            QString display = QString("[%1] %2 - %3 pax, %4 km/h")
            .arg(QString::fromStdString(ac.id))
                .arg(QString::fromStdString(ac.model))
                .arg(ac.capacity)
                .arg(ac.cruiseSpeed, 0, 'f', 0);

            aircraftCombo->addItem(display, QString::fromStdString(ac.id));
        }
    }

    planForm->addRow("📍 Origin:", originCombo);
    planForm->addRow("📍 Destination:", destCombo);
    planForm->addRow("✈️ Aircraft:", aircraftCombo);

    planMainLayout->addLayout(planForm);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();

    QPushButton* previewBtn = new QPushButton("🔍 Preview Route");
    QPushButton* planBtn = new QPushButton("📊 Calculate Optimal Path");
    QPushButton* bookBtn = new QPushButton("✅ Book Flight");
    QPushButton* clearBtn = new QPushButton("🗑️ Clear");

    previewBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    planBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    bookBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    clearBtn->setStyleSheet("QPushButton { padding: 8px; }");

    btnLayout->addWidget(previewBtn);
    btnLayout->addWidget(planBtn);
    btnLayout->addWidget(bookBtn);
    btnLayout->addWidget(clearBtn);

    planMainLayout->addLayout(btnLayout);

    // Result display
    resultText = new QTextEdit();
    resultText->setReadOnly(true);
    resultText->setMaximumHeight(200);
    resultText->setStyleSheet(
        "QTextEdit {"
        " color: black;"
        " background-color: #f5f5f5;"
        " font-family: 'Courier New';"
        "}"
        );
    resultText->setPlaceholderText(
        "Select airports and aircraft, then click 'Preview Route' to see options..."
        );

    //resultText = new QTextEdit();
   // resultText->setReadOnly(true);
    //resultText->setMaximumHeight(200);
   // resultText->setStyleSheet("QTextEdit { font-family: 'Courier New'; background-color: #f5f5f5;color: black; }");
   // resultText->setPlaceholderText("Select airports and aircraft, then click 'Preview Route' to see options...");

    planMainLayout->addWidget(new QLabel("<b>Route Details:</b>"));
    planMainLayout->addWidget(resultText);

    // ==================== BOOKED FLIGHTS SECTION ====================
    QGroupBox* flightsGroup = new QGroupBox("🎫 Booked Flights");
    QVBoxLayout* flightsLayout = new QVBoxLayout(flightsGroup);

    flightTable = new QTableWidget();
    flightTable->setColumnCount(8);
    flightTable->setHorizontalHeaderLabels({
        "Flight #", "Aircraft", "Route", "Distance", "Cost", "Duration", "Departure", "Status"
    });
    flightTable->horizontalHeader()->setStretchLastSection(true);
    flightTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flightTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    flightTable->setAlternatingRowColors(true);

    QHBoxLayout* tableButtons = new QHBoxLayout();
    QPushButton* refreshBtn = new QPushButton("🔄 Refresh");
    QPushButton* deleteBtn = new QPushButton("❌ Delete Selected");

    tableButtons->addWidget(refreshBtn);
    tableButtons->addWidget(deleteBtn);
    tableButtons->addStretch();

    flightsLayout->addWidget(flightTable);
    flightsLayout->addLayout(tableButtons);

    // ==================== ADD TO MAIN LAYOUT ====================
    mainLayout->addWidget(planGroup, 1);
    mainLayout->addWidget(flightsGroup, 1);

    // ==================== CONNECTIONS ====================
    connect(previewBtn, &QPushButton::clicked, this, &FlightManager::onPreviewRoute);
    connect(planBtn, &QPushButton::clicked, this, &FlightManager::onPlanFlight);
    connect(bookBtn, &QPushButton::clicked, this, &FlightManager::onBookFlight);
    connect(clearBtn, &QPushButton::clicked, this, &FlightManager::onClearSelection);
    connect(refreshBtn, &QPushButton::clicked, this, &FlightManager::onRefreshFlights);
    connect(deleteBtn, &QPushButton::clicked, this, &FlightManager::onDeleteFlight);

    // Initial load
    onRefreshFlights();
}

bool FlightManager::validateInputs() {
    QString origin = originCombo->currentData().toString();
    QString dest = destCombo->currentData().toString();
    QString aircraftId = aircraftCombo->currentData().toString();

    if (origin.isEmpty()) {
        QMessageBox::warning(this, "❌ Missing Information",
                             "Please select an origin airport.");
        originCombo->setFocus();
        return false;
    }

    if (dest.isEmpty()) {
        QMessageBox::warning(this, "❌ Missing Information",
                             "Please select a destination airport.");
        destCombo->setFocus();
        return false;
    }

    if (origin == dest) {
        QMessageBox::warning(this, "❌ Invalid Route",
                             "Origin and destination must be different airports.\n\n"
                             "Please select a different destination.");
        destCombo->setFocus();
        return false;
    }

    if (aircraftId.isEmpty()) {
        QMessageBox::warning(this, "❌ Missing Information",
                             "Please select an aircraft for this flight.");
        aircraftCombo->setFocus();
        return false;
    }

    return true;
}

void FlightManager::onPreviewRoute() {
    if (!validateInputs()) return;

    QString origin = originCombo->currentData().toString();
    QString dest = destCombo->currentData().toString();

    QProgressDialog progress("Calculating optimal route...", nullptr, 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(30);

    DataStore& store = DataStore::getInstance();
    Graph& graph = store.getGraph();

    progress.setValue(60);

    PathResult result = Dijkstra::findShortestPath(graph,
                                                   origin.toStdString(),
                                                   dest.toStdString());

    progress.setValue(100);

    if (result.found) {
        showRoutePreview(result);
        currentPath = result;
        hasPlannedRoute = true;

        // Show on map
        if (mapWidget) {
            mapWidget->setOptimalPath(result);
        }
    } else {
        resultText->setPlainText(
            "❌ NO ROUTE AVAILABLE\n\n"
            "Reason: " + QString::fromStdString(result.errorMessage) + "\n\n"
                                                            "This could mean:\n"
                                                            "• No connecting routes exist\n"
                                                            "• All routes are marked as non-operational\n"
                                                            "• Airports are not connected in the network\n\n"
                                                            "💡 Suggestion: Add routes between these airports in the Routes tab."
            );

        hasPlannedRoute = false;

        if (mapWidget) {
            mapWidget->clearOptimalPath();
        }
    }
}

void FlightManager::showRoutePreview(const PathResult& result) {
    QString output;

    output += "✈️ OPTIMAL ROUTE FOUND\n";
    output += "═══════════════════════════════════════\n\n";

    // Route visualization
    output += "📍 ROUTE PATH:\n";
    for (size_t i = 0; i < result.path.size(); ++i) {
        output += "   " + QString::fromStdString(result.path[i]);
        if (i < result.path.size() - 1) {
            output += " ➜ ";
            if ((i + 1) % 4 == 0 && i < result.path.size() - 1) {
                output += "\n   ";
            }
        }
    }
    output += "\n\n";

    // Statistics
    output += "📊 ROUTE STATISTICS:\n";
    output += QString("   • Total Distance: %1 km\n").arg(result.totalDistance, 0, 'f', 2);
    output += QString("   • Number of Stops: %1\n").arg(result.path.size() - 2);

    // Calculate with selected aircraft
    QString aircraftId = aircraftCombo->currentData().toString();
    if (!aircraftId.isEmpty()) {
        DataStore& store = DataStore::getInstance();
        Aircraft* aircraft = store.getAircraft(aircraftId.toStdString());

        if (aircraft) {
            double estimatedCost = result.totalDistance * aircraft->fuelConsumption * 0.8;
            double estimatedTime = result.totalDistance / aircraft->cruiseSpeed;

            output += QString("   • Estimated Cost: $%1\n").arg(estimatedCost, 0, 'f', 2);
            output += QString("   • Estimated Duration: %1 hrs %2 min\n")
                          .arg((int)estimatedTime)
                          .arg((int)((estimatedTime - (int)estimatedTime) * 60));

            output += "\n";
            output += QString("✈️ SELECTED AIRCRAFT: %1\n").arg(QString::fromStdString(aircraft->model));
            output += QString("   • Capacity: %1 passengers\n").arg(aircraft->capacity);
            output += QString("   • Cruise Speed: %1 km/h\n").arg(aircraft->cruiseSpeed, 0, 'f', 0);
            output += QString("   • Fuel Consumption: %1 L/km\n").arg(aircraft->fuelConsumption, 0, 'f', 2);
        }
    }

    output += "\n";
    output += "✅ Route is valid and ready for booking!\n";
    output += "Click 'Book Flight' to confirm reservation.";

    resultText->setPlainText(output);
}

void FlightManager::onPlanFlight() {
    onPreviewRoute();
}

void FlightManager::onBookFlight() {
    if (!validateInputs()) return;

    if (!hasPlannedRoute) {
        QMessageBox::warning(this, "⚠️ No Route Planned",
                             "Please preview and calculate the route first before booking.");
        return;
    }

    // Confirm booking
    auto reply = QMessageBox::question(this, "✅ Confirm Flight Booking",
                                       "Are you sure you want to book this flight?\n\n"
                                       "This will:\n"
                                       "• Create a flight reservation\n"
                                       "• Update aircraft status\n"
                                       "• Save to flights database\n\n"
                                       "Proceed with booking?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QString aircraftId = aircraftCombo->currentData().toString();

    DataStore& store = DataStore::getInstance();
    Aircraft* aircraft = store.getAircraft(aircraftId.toStdString());

    if (!aircraft || !aircraft->isAvailable()) {
        QMessageBox::critical(this, "❌ Aircraft Unavailable",
                              "The selected aircraft is no longer available.\n"
                              "It may have been booked by another flight.\n\n"
                              "Please select a different aircraft.");
        return;
    }

    // Generate flight number
    static int flightCounter = 1000;
    std::string flightNum = "FL" + std::to_string(flightCounter++);

    // Create flight
    Flight flight;
    flight.flightNumber = flightNum;
    flight.aircraftId = aircraftId.toStdString();
    flight.route = currentPath.path;
    flight.totalDistance = currentPath.totalDistance;
    flight.totalCost = currentPath.totalDistance * aircraft->fuelConsumption * 0.8;
    flight.estimatedTime = currentPath.totalDistance / aircraft->cruiseSpeed;

    // Set departure time (2 hours from now)
    QDateTime departure = QDateTime::currentDateTime().addSecs(2 * 3600);
    flight.departureTime = departure.toString("yyyy-MM-ddTHH:mm:ss").toStdString();
    flight.status = "SCHEDULED";

    // Save flight
    if (store.addFlight(flight)) {
        // Update aircraft status
        aircraft->status = AircraftStatus::IN_FLIGHT;
        store.updateAircraft(*aircraft);

        // Save all data
        store.saveAll();

        // Success message
        QString message = QString(
                              "✅ FLIGHT BOOKED SUCCESSFULLY!\n\n"
                              "Flight Number: %1\n"
                              "Aircraft: %2\n"
                              "Departure: %3\n\n"
                              "Route: %4\n\n"
                              "Distance: %5 km\n"
                              "Cost: $%6\n"
                              "Duration: %7 hours\n\n"
                              "✈️ Flight has been saved to database.\n"
                              "Check the 'Booked Flights' table below for details."
                              ).arg(QString::fromStdString(flightNum))
                              .arg(QString::fromStdString(aircraft->model))
                              .arg(departure.toString("MMM dd, yyyy HH:mm"))
                              .arg(QString::fromStdString(currentPath.path[0]) + " → " +
                                   QString::fromStdString(currentPath.path[currentPath.path.size()-1]))
                              .arg(flight.totalDistance, 0, 'f', 2)
                              .arg(flight.totalCost, 0, 'f', 2)
                              .arg(flight.estimatedTime, 0, 'f', 2);

        QMessageBox::information(this, "✈️ Booking Confirmed", message);

        // Refresh UI
        onRefreshFlights();
        onClearSelection();

    } else {
        QMessageBox::critical(this, "❌ Booking Failed",
                              "Failed to book flight.\nPlease try again.");
    }
}

void FlightManager::onRefreshFlights() {
    flightTable->setRowCount(0);

    DataStore& store = DataStore::getInstance();
    auto flights = store.getAllFlights();

    for (const auto& flight : flights) {
        int row = flightTable->rowCount();
        flightTable->insertRow(row);

        flightTable->setItem(row, 0,
                             new QTableWidgetItem(QString::fromStdString(flight.flightNumber)));

        flightTable->setItem(row, 1,
                             new QTableWidgetItem(QString::fromStdString(flight.aircraftId)));

        // Build route string
        QString routeStr = QString::fromStdString(flight.route[0]);
        if (flight.route.size() > 2) {
            routeStr += QString(" (+%1 stops) ").arg(flight.route.size() - 2);
        } else {
            routeStr += " → ";
        }
        routeStr += QString::fromStdString(flight.route[flight.route.size()-1]);

        flightTable->setItem(row, 2, new QTableWidgetItem(routeStr));

        flightTable->setItem(row, 3,
                             new QTableWidgetItem(QString::number(flight.totalDistance, 'f', 0) + " km"));

        flightTable->setItem(row, 4,
                             new QTableWidgetItem("$" + QString::number(flight.totalCost, 'f', 2)));

        double hours = (int)flight.estimatedTime;
        double mins = (flight.estimatedTime - hours) * 60;
        flightTable->setItem(row, 5,
                             new QTableWidgetItem(QString("%1h %2m").arg((int)hours).arg((int)mins)));

        flightTable->setItem(row, 6,
                             new QTableWidgetItem(QString::fromStdString(flight.departureTime)));

        // Color-code status
        QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(flight.status));
        if (flight.status == "SCHEDULED") {
            statusItem->setBackground(QColor(76, 175, 80, 50)); // Green
        } else if (flight.status == "COMPLETED") {
            statusItem->setBackground(QColor(33, 150, 243, 50)); // Blue
        }
        flightTable->setItem(row, 7, statusItem);
    }

    flightTable->resizeColumnsToContents();
}

void FlightManager::onClearSelection() {
    originCombo->setCurrentIndex(0);
    destCombo->setCurrentIndex(0);
    aircraftCombo->setCurrentIndex(0);
    resultText->clear();
    resultText->setPlaceholderText("Select airports and aircraft, then click 'Preview Route' to see options...");
    hasPlannedRoute = false;

    if (mapWidget) {
        mapWidget->clearOptimalPath();
    }
}

void FlightManager::onDeleteFlight() {
    int row = flightTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection",
                             "Please select a flight to delete.");
        return;
    }

    QString flightNum = flightTable->item(row, 0)->text();

    auto reply = QMessageBox::question(this, "Confirm Delete",
                                       QString("Delete flight %1?").arg(flightNum),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DataStore& store = DataStore::getInstance();
        if (store.deleteFlight(flightNum.toStdString())) {
            store.saveAll();
            QMessageBox::information(this, "Success", "Flight deleted.");
            onRefreshFlights();
        }
    }
}
void FlightManager::refreshData() {
    onRefreshFlights();
}
