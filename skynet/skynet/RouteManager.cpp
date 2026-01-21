#include "RouteManager.h"
#include "DataStore.h"
#include "Haversine.h"
#include "Route.h"
#include "airports.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

RouteManager::RouteManager(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadRoutes();
}

void RouteManager::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Input section
    QGroupBox* inputGroup = new QGroupBox("Add New Route");
    QFormLayout* formLayout = new QFormLayout(inputGroup);

    originCombo = new QComboBox();
    destCombo = new QComboBox();
    costEdit = new QLineEdit("1000.00");

    loadAirports();

    formLayout->addRow("Origin Airport:", originCombo);
    formLayout->addRow("Destination Airport:", destCombo);
    formLayout->addRow("Base Cost ($):", costEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Add Route (Bidirectional)");
    deleteBtn = new QPushButton("Delete Selected");
    QPushButton* refreshBtn = new QPushButton("Refresh");

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addStretch();

    formLayout->addRow(btnLayout);

    // Table
    table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Origin", "Destination", "Distance (km)", "Base Cost ($)", "Status"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(table);

    connect(addBtn, &QPushButton::clicked, this, &RouteManager::onAdd);
    connect(deleteBtn, &QPushButton::clicked, this, &RouteManager::onDelete);
    connect(refreshBtn, &QPushButton::clicked, this, &RouteManager::onRefresh);
}

void RouteManager::loadAirports() {
    originCombo->clear();
    destCombo->clear();

    DataStore& store = DataStore::getInstance();
    auto airports = store.getAllAirports();

    for (const auto& airport : airports) {
        QString display = QString::fromStdString(airport.code + " - " + airport.name);
        originCombo->addItem(display, QString::fromStdString(airport.code));
        destCombo->addItem(display, QString::fromStdString(airport.code));
    }
}

void RouteManager::loadRoutes() {
    table->setRowCount(0);

    DataStore& store = DataStore::getInstance();
    auto routes = store.getAllRoutes();

    for (const auto& route : routes) {
        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(route.origin)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(route.destination)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(route.distance, 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(route.baseCost, 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(route.operational ? "Active" : "Inactive"));
    }
}

void RouteManager::onAdd() {
    QString origin = originCombo->currentData().toString();
    QString dest = destCombo->currentData().toString();

    if (origin == dest) {
        QMessageBox::warning(this, "Invalid Route", "Origin and destination must be different.");
        return;
    }

    DataStore& store = DataStore::getInstance();

    // Get airport coordinates
    Airport* originAirport = store.getAirport(origin.toStdString());
    Airport* destAirport = store.getAirport(dest.toStdString());

    if (!originAirport || !destAirport) {
        QMessageBox::warning(this, "Error", "Could not find airport data.");
        return;
    }

    // Calculate distance using Haversine
    double distance = Haversine::calculate(
        originAirport->latitude, originAirport->longitude,
        destAirport->latitude, destAirport->longitude
        );

    double cost = costEdit->text().toDouble();

    // Create route
    Route route(origin.toStdString(), dest.toStdString(), distance, cost, true);

    if (store.addRoute(route)) {
        QMessageBox::information(this, "Success",
                                 QString("Route added: %1 → %2 (%3 km)\nDistance calculated automatically using Haversine formula.")
                                     .arg(origin).arg(dest).arg(distance, 0, 'f', 2));

        store.saveAll();
        loadRoutes();
    } else {
        QMessageBox::warning(this, "Error", "Route already exists.");
    }
}

void RouteManager::onDelete() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a route to delete.");
        return;
    }

    QString origin = table->item(row, 0)->text();
    QString dest = table->item(row, 1)->text();
    QString routeId = origin + "-" + dest;

    auto reply = QMessageBox::question(this, "Confirm Delete",
                                       QString("Delete route: %1 → %2?").arg(origin).arg(dest),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DataStore& store = DataStore::getInstance();
        if (store.deleteRoute(routeId.toStdString())) {
            QMessageBox::information(this, "Success", "Route deleted.");
            store.saveAll();
            loadRoutes();
        }
    }
}

void RouteManager::onRefresh() {
    loadAirports();
    loadRoutes();
}
void RouteManager::refreshData() {
    loadAirports();
    loadRoutes();
}
