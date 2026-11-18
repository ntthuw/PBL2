#pragma once

#include "Employee.h"
#include "Manager.h"
#include "Driver.h"
#include "Janitor.h"
#include <string>

using namespace std;

class EmployeeManager
{
private:
    Employee** arr;
    int size;
    int capacity;

    void resize();

public:
    // Constructor / Destructor
    EmployeeManager();
    EmployeeManager(const EmployeeManager& other);
    EmployeeManager& operator=(const EmployeeManager& other);
    ~EmployeeManager();

    // Getter
    int getSize() const;
    int getCapacity() const;

    // Truy cap nhan vien
    Employee* operator[](int index);
    const Employee* operator[](int index) const;

    // Nap chong toan tu
    EmployeeManager& operator+=(const Employee& employee);
    EmployeeManager& operator-=(const string& id);

    // Quan ly du lieu nhan vien
    void addEmployee(const Employee& employee);

    bool editEmployee(
        const string& id,
        const Employee& updatedEmployee
    );

    bool deleteEmployee(const string& id);

    // Tim kiem va hien thi
    Employee* findEmployeeById(const string& id);
    const Employee* findEmployeeById(const string& id) const;

    void displayAll() const;

    // Tinh toan va thong ke
    long long calculateTotalSalaryFund() const;

    void displayByPosition(const string& position) const;

    bool isEmployeeIdExist(const string& id) const;
};
