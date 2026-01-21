#include "AirportManager.h"
#include "DataStore.h"
#include "Haversine.h"
#include "airports.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>

AirportManager::AirportManager(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadAirports();
}

void AirportManager::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ========== INPUT FORM ==========
    QGroupBox* inputGroup = new QGroupBox("✈️ Airport Details");
    QFormLayout* formLayout = new QFormLayout(inputGroup);

    codeEdit = new QLineEdit();
    nameEdit = new QLineEdit();
    cityEdit = new QLineEdit();
    countryEdit = new QLineEdit();
    latEdit = new QLineEdit();
    lonEdit = new QLineEdit();

    codeEdit->setPlaceholderText("e.g., JFK");
    nameEdit->setPlaceholderText("e.g., John F Kennedy International");
    cityEdit->setPlaceholderText("e.g., New York");
    countryEdit->setPlaceholderText("e.g., USA");
    latEdit->setPlaceholderText("e.g., 40.6413");
    lonEdit->setPlaceholderText("e.g., -73.7781");

    codeEdit->setMaxLength(3);

    formLayout->addRow("IATA Code *:", codeEdit);
    formLayout->addRow("Airport Name *:", nameEdit);
    formLayout->addRow("City:", cityEdit);
    formLayout->addRow("Country:", countryEdit);
    formLayout->addRow("Latitude:", latEdit);
    formLayout->addRow("Longitude:", lonEdit);

    // ========== BUTTONS ==========
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("➕ Add Airport");
    updateBtn = new QPushButton("✏️ Update Selected");
    deleteBtn = new QPushButton("🗑️ Delete Selected");
    refreshBtn = new QPushButton("🔄 Refresh");

    addBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; font-weight: bold; }");
    updateBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; font-weight: bold; }");
    deleteBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px; font-weight: bold; }");
    refreshBtn->setStyleSheet("QPushButton { padding: 8px; }");

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addStretch();

    formLayout->addRow(btnLayout);

    // Info label
    QLabel* infoLabel = new QLabel("💡 Tip: Select a row from table to edit");
    infoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    formLayout->addRow(infoLabel);

    // ========== TABLE ==========
    table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Code", "Name", "City", "Country", "Latitude", "Longitude"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(true);

    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(new QLabel("<b>📋 Airport Database:</b>"));
    mainLayout->addWidget(table);

    // ========== CONNECTIONS ==========
    connect(addBtn, &QPushButton::clicked, this, &AirportManager::onAdd);
    connect(deleteBtn, &QPushButton::clicked, this, &AirportManager::onDelete);
    connect(updateBtn, &QPushButton::clicked, this, &AirportManager::onUpdate);
    connect(refreshBtn, &QPushButton::clicked, this, &AirportManager::onRefresh);
    connect(table, &QTableWidget::itemSelectionChanged, this, &AirportManager::onTableSelectionChanged);
}

void AirportManager::loadAirports() {
    table->setRowCount(0);

    DataStore& store = DataStore::getInstance();
    auto airports = store.getAllAirports();

    for (const auto& airport : airports) {
        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(airport.code)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(airport.name)));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(airport.city)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(airport.country)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(airport.latitude, 'f', 4)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(airport.longitude, 'f', 4)));
    }

    table->resizeColumnsToContents();
}

void AirportManager::onTableSelectionChanged() {
    int row = table->currentRow();
    if (row < 0) return;

    // Load selected airport data into form
    codeEdit->setText(table->item(row, 0)->text());
    nameEdit->setText(table->item(row, 1)->text());
    cityEdit->setText(table->item(row, 2)->text());
    countryEdit->setText(table->item(row, 3)->text());
    latEdit->setText(table->item(row, 4)->text());
    lonEdit->setText(table->item(row, 5)->text());

    // Disable code editing when updating
    codeEdit->setReadOnly(true);
    codeEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; }");
}

void AirportManager::onAdd() {
    Airport airport;
    airport.code = codeEdit->text().trimmed().toUpper().toStdString();
    airport.name = nameEdit->text().trimmed().toStdString();
    airport.city = cityEdit->text().trimmed().toStdString();
    airport.country = countryEdit->text().trimmed().toStdString();

    // Validate required fields
    if (airport.code.empty() || airport.name.empty()) {
        QMessageBox::warning(this, "❌ Invalid Input",
                             "Airport Code and Name are required fields.\n\nPlease fill them before adding.");
        return;
    }

    if (airport.code.length() != 3) {
        QMessageBox::warning(this, "❌ Invalid Code",
                             "Airport code must be exactly 3 characters (IATA standard).\n\nExample: JFK, LAX, LHR");
        return;
    }

    // Parse coordinates
    bool latOk, lonOk;
    airport.latitude = latEdit->text().toDouble(&latOk);
    airport.longitude = lonEdit->text().toDouble(&lonOk);

    if (!latOk || !lonOk) {
        QMessageBox::warning(this, "❌ Invalid Coordinates",
                             "Latitude and Longitude must be valid numbers.\n\n"
                             "Examples:\n"
                             "Latitude: 40.6413 (range: -90 to 90)\n"
                             "Longitude: -73.7781 (range: -180 to 180)");
        return;
    }

    // Validate coordinate ranges
    if (airport.latitude < -90 || airport.latitude > 90) {
        QMessageBox::warning(this, "❌ Invalid Latitude",
                             "Latitude must be between -90 and 90 degrees.");
        return;
    }

    if (airport.longitude < -180 || airport.longitude > 180) {
        QMessageBox::warning(this, "❌ Invalid Longitude",
                             "Longitude must be between -180 and 180 degrees.");
        return;
    }

    DataStore& store = DataStore::getInstance();
    if (store.addAirport(airport)) {
        store.saveAll();
        QMessageBox::information(this, "✅ Success",
                                 QString("Airport '%1' added successfully!\n\nYou can now create routes using this airport.")
                                     .arg(QString::fromStdString(airport.code)));
        loadAirports();
        clearForm();
    } else {
        QMessageBox::warning(this, "❌ Error",
                             QString("Airport with code '%1' already exists!\n\nPlease use a different code.")
                                 .arg(QString::fromStdString(airport.code)));
    }
}

void AirportManager::onUpdate() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "⚠️ No Selection",
                             "Please select an airport from the table to update.");
        return;
    }

    QString code = table->item(row, 0)->text();

    Airport airport;
    airport.code = code.toStdString();
    airport.name = nameEdit->text().trimmed().toStdString();
    airport.city = cityEdit->text().trimmed().toStdString();
    airport.country = countryEdit->text().trimmed().toStdString();

    bool latOk, lonOk;
    airport.latitude = latEdit->text().toDouble(&latOk);
    airport.longitude = lonEdit->text().toDouble(&lonOk);

    if (!latOk || !lonOk) {
        QMessageBox::warning(this, "❌ Invalid Input", "Invalid coordinates.");
        return;
    }

    DataStore& store = DataStore::getInstance();
    if (store.updateAirport(airport)) {
        store.saveAll();
        QMessageBox::information(this, "✅ Success",
                                 QString("Airport '%1' updated successfully!").arg(code));
        loadAirports();
        clearForm();
    } else {
        QMessageBox::critical(this, "❌ Error", "Failed to update airport.");
    }
}

void AirportManager::onDelete() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "⚠️ No Selection",
                             "Please select an airport from the table to delete.");
        return;
    }

    QString code = table->item(row, 0)->text();
    QString name = table->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "⚠️ Confirm Delete",
                                       QString("Are you sure you want to delete airport:\n\n"
                                               "Code: %1\n"
                                               "Name: %2\n\n"
                                               "⚠️ This will also remove all routes connected to this airport!\n\n"
                                               "This action cannot be undone.")
                                           .arg(code).arg(name),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DataStore& store = DataStore::getInstance();
        if (store.deleteAirport(code.toStdString())) {
            store.saveAll();
            QMessageBox::information(this, "✅ Deleted",
                                     QString("Airport '%1' has been deleted.").arg(code));
            loadAirports();
            clearForm();
        }
    }
}

void AirportManager::onRefresh() {
    loadAirports();
    clearForm();
    QMessageBox::information(this, "🔄 Refreshed",
                             "Airport data has been reloaded from database.");
}

void AirportManager::refreshData() {
    loadAirports();
}

void AirportManager::clearForm() {
    codeEdit->clear();
    nameEdit->clear();
    cityEdit->clear();
    countryEdit->clear();
    latEdit->clear();
    lonEdit->clear();

    codeEdit->setReadOnly(false);
    codeEdit->setStyleSheet("");

    table->clearSelection();
}
