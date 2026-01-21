#include "AircraftManager.h"
#include "DataStore.h"
#include "Aircraft.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

AircraftManager::AircraftManager(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadAircraft();
}

void AircraftManager::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Input section
    QGroupBox* inputGroup = new QGroupBox("Aircraft Details");
    QFormLayout* formLayout = new QFormLayout(inputGroup);

    idEdit = new QLineEdit();
    modelEdit = new QLineEdit();
    capacityEdit = new QLineEdit();
    speedEdit = new QLineEdit();
    fuelEdit = new QLineEdit();
    statusCombo = new QComboBox();

    statusCombo->addItem("AVAILABLE");
    statusCombo->addItem("IN_FLIGHT");
    statusCombo->addItem("MAINTENANCE");
    statusCombo->addItem("RETIRED");

    formLayout->addRow("Aircraft ID:", idEdit);
    formLayout->addRow("Model:", modelEdit);
    formLayout->addRow("Capacity:", capacityEdit);
    formLayout->addRow("Cruise Speed (km/h):", speedEdit);
    formLayout->addRow("Fuel Consumption (L/km):", fuelEdit);
    formLayout->addRow("Status:", statusCombo);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Add Aircraft");
    deleteBtn = new QPushButton("Delete Selected");
    QPushButton* refreshBtn = new QPushButton("Refresh");

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addStretch();

    formLayout->addRow(btnLayout);

    // Table
    table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "Model", "Capacity", "Speed (km/h)", "Fuel (L/km)", "Status"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(table);

    connect(addBtn, &QPushButton::clicked, this, &AircraftManager::onAdd);
    connect(deleteBtn, &QPushButton::clicked, this, &AircraftManager::onDelete);
    connect(refreshBtn, &QPushButton::clicked, this, &AircraftManager::onRefresh);
}

void AircraftManager::loadAircraft() {
    table->setRowCount(0);

    DataStore& store = DataStore::getInstance();
    auto aircraftList = store.getAllAircraft();

    for (const auto& ac : aircraftList) {
        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(ac.id)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(ac.model)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(ac.capacity)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(ac.cruiseSpeed)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(ac.fuelConsumption, 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(
                                   Aircraft::statusToString(ac.status))));
    }
}

void AircraftManager::onAdd() {
    QString id = idEdit->text();
    QString model = modelEdit->text();
    int capacity = capacityEdit->text().toInt();
    double speed = speedEdit->text().toDouble();
    double fuel = fuelEdit->text().toDouble();

    if (id.isEmpty() || model.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "ID and Model are required.");
        return;
    }

    if (capacity <= 0 || speed <= 0 || fuel <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Capacity, Speed, and Fuel must be positive numbers.");
        return;
    }

    Aircraft aircraft(id.toStdString(), model.toStdString(), capacity, speed, fuel);
    aircraft.status = Aircraft::stringToStatus(statusCombo->currentText().toStdString());

    DataStore& store = DataStore::getInstance();
    if (store.addAircraft(aircraft)) {
        QMessageBox::information(this, "Success", "Aircraft added successfully.");
        store.saveAll();
        loadAircraft();

        // Clear form
        idEdit->clear();
        modelEdit->clear();
        capacityEdit->clear();
        speedEdit->clear();
        fuelEdit->clear();
    } else {
        QMessageBox::warning(this, "Error", "Aircraft with this ID already exists.");
    }
}

void AircraftManager::onDelete() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an aircraft to delete.");
        return;
    }

    QString id = table->item(row, 0)->text();

    auto reply = QMessageBox::question(this, "Confirm Delete",
                                       QString("Delete aircraft %1?").arg(id),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DataStore& store = DataStore::getInstance();
        if (store.deleteAircraft(id.toStdString())) {
            QMessageBox::information(this, "Success", "Aircraft deleted.");
            store.saveAll();
            loadAircraft();
        }
    }
}

void AircraftManager::onRefresh() {
    loadAircraft();
}
void AircraftManager::refreshData() {
    loadAircraft();
}
