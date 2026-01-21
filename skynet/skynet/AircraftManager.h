#ifndef AIRCRAFTMANAGER_H
#define AIRCRAFTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class AircraftManager : public QWidget {
    Q_OBJECT
public:
    explicit AircraftManager(QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onAdd();
    void onDelete();
    void onRefresh();

private:
    void setupUi();
    void loadAircraft();

    QTableWidget* table;
    QLineEdit* idEdit;
    QLineEdit* modelEdit;
    QLineEdit* capacityEdit;
    QLineEdit* speedEdit;
    QLineEdit* fuelEdit;
    QComboBox* statusCombo;
    QPushButton* addBtn;
    QPushButton* deleteBtn;
};

#endif // AIRCRAFTMANAGER_H
