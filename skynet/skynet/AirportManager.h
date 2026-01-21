#ifndef AIRPORTMANAGER_H
#define AIRPORTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

class AirportManager : public QWidget {
    Q_OBJECT
public:
    explicit AirportManager(QWidget *parent = nullptr);
    void refreshData();  // PUBLIC METHOD FOR REFRESH

private slots:
    void onAdd();
    void onDelete();
    void onUpdate();
    void onRefresh();
    void onTableSelectionChanged();

private:
    void setupUi();
    void loadAirports();
    void clearForm();

    QTableWidget* table;
    QLineEdit* codeEdit;
    QLineEdit* nameEdit;
    QLineEdit* cityEdit;
    QLineEdit* countryEdit;
    QLineEdit* latEdit;
    QLineEdit* lonEdit;
    QPushButton* addBtn;
    QPushButton* deleteBtn;
    QPushButton* updateBtn;
    QPushButton* refreshBtn;
};

#endif // AIRPORTMANAGER_H
