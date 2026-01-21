#ifndef ROUTEMANAGER_H
#define ROUTEMANAGER_H
#include <QWidget>
#include <QTableWidget>

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

class RouteManager : public QWidget {
    Q_OBJECT
public:
    explicit RouteManager(QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onAdd();
    void onDelete();
    void onRefresh();

private:
    void setupUi();
    void loadRoutes();
    void loadAirports();

    QTableWidget* table;
    QComboBox* originCombo;
    QComboBox* destCombo;
    QLineEdit* costEdit;
    QPushButton* addBtn;
    QPushButton* deleteBtn;
};

#endif // ROUTEMANAGER_H
