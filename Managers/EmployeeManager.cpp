#include "EmployeeManager.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

// Constructor
EmployeeManager::EmployeeManager()
{
    capacity = 5;
    size = 0;
    arr = new Employee*[capacity];

    for (int i = 0; i < capacity; i++)
    {
        arr[i] = nullptr;
    }
}

// Copy constructor
EmployeeManager::EmployeeManager(
    const EmployeeManager& other
)
{
    size = other.size;
    capacity = other.capacity;

    arr = new Employee*[capacity];

    for (int i = 0; i < capacity; i++)
    {
        arr[i] = nullptr;
    }

    try
    {
        for (int i = 0; i < size; i++)
        {
            arr[i] = other.arr[i]->clone();
        }
    }
    catch (...)
    {
        for (int i = 0; i < size; i++)
        {
            delete arr[i];
        }

        delete[] arr;
        throw;
    }
}

// Assignment operator
EmployeeManager& EmployeeManager::operator=(
    const EmployeeManager& other
)
{
    if (this == &other)
    {
        return *this;
    }

    Employee** newArr = new Employee*[other.capacity];

    for (int i = 0; i < other.capacity; i++)
    {
        newArr[i] = nullptr;
    }

    try
    {
        for (int i = 0; i < other.size; i++)
        {
            newArr[i] = other.arr[i]->clone();
        }
    }
    catch (...)
    {
        for (int i = 0; i < other.capacity; i++)
        {
            delete newArr[i];
        }

        delete[] newArr;
        throw;
    }

    for (int i = 0; i < size; i++)
    {
        delete arr[i];
    }

    delete[] arr;

    arr = newArr;
    size = other.size;
    capacity = other.capacity;

    return *this;
}

// Destructor
EmployeeManager::~EmployeeManager()
{
    for (int i = 0; i < size; i++)
    {
        delete arr[i];
    }

    delete[] arr;
}

// Mo rong mang
void EmployeeManager::resize()
{
    int newCapacity = capacity * 2;

    Employee** newArr = new Employee*[newCapacity];

    for (int i = 0; i < newCapacity; i++)
    {
        newArr[i] = nullptr;
    }

    for (int i = 0; i < size; i++)
    {
        newArr[i] = arr[i];
    }

    delete[] arr;

    arr = newArr;
    capacity = newCapacity;
}

// Getter
int EmployeeManager::getSize() const
{
    return size;
}

int EmployeeManager::getCapacity() const
{
    return capacity;
}

// Truy cap nhan vien
Employee* EmployeeManager::operator[](int index)
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Employee index out of range!"
        );
    }

    return arr[index];
}

const Employee* EmployeeManager::operator[](int index) const
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Employee index out of range!"
        );
    }

    return arr[index];
}

// Nap chong toan tu
EmployeeManager& EmployeeManager::operator+=(
    const Employee& employee
)
{
    addEmployee(employee);
    return *this;
}

EmployeeManager& EmployeeManager::operator-=(
    const string& id
)
{
    deleteEmployee(id);
    return *this;
}

// Them nhan vien
void EmployeeManager::addEmployee(
    const Employee& employee
)
{
    employee.validate();

    if (isEmployeeIdExist(employee.get_id()))
    {
        throw invalid_argument(
            "Employee ID already exists!"
        );
    }

    // Tao ban sao truoc khi thay doi mang
    Employee* newEmployee = employee.clone();

    try
    {
        if (size == capacity)
        {
            resize();
        }
    }
    catch (...)
    {
        delete newEmployee;
        throw;
    }

    arr[size] = newEmployee;
    size++;
}

// Chinh sua nhan vien
bool EmployeeManager::editEmployee(
    const string& id,
    const Employee& updatedEmployee
)
{
    Employee* currentEmployee = findEmployeeById(id);

    if (currentEmployee == nullptr)
    {
        return false;
    }

    updatedEmployee.validate();

    if (updatedEmployee.get_id() != id)
    {
        throw invalid_argument(
            "Employee ID cannot be changed during editing!"
        );
    }

    if (updatedEmployee.get_position() !=
        currentEmployee->get_position())
    {
        throw invalid_argument(
            "Employee position cannot be changed directly!"
        );
    }

    Employee* newEmployee = updatedEmployee.clone();

    // Tim vi tri truoc khi thay the con tro, tranh truy cap
    // currentEmployee sau khi da delete (use-after-free).
    int index = -1;

    for (int i = 0; i < size; i++)
    {
        if (arr[i] == currentEmployee)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        delete newEmployee;
        return false;
    }

    delete arr[index];
    arr[index] = newEmployee;

    return true;
}

// Xoa nhan vien
bool EmployeeManager::deleteEmployee(
    const string& id
)
{
    int index = -1;

    for (int i = 0; i < size; i++)
    {
        if (arr[i]->get_id() == id)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        return false;
    }

    delete arr[index];

    for (int i = index; i < size - 1; i++)
    {
        arr[i] = arr[i + 1];
    }

    arr[size - 1] = nullptr;
    size--;

    return true;
}

// Tim nhan vien theo ID
Employee* EmployeeManager::findEmployeeById(
    const string& id
)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i]->get_id() == id)
        {
            return arr[i];
        }
    }

    return nullptr;
}

const Employee* EmployeeManager::findEmployeeById(
    const string& id
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i]->get_id() == id)
        {
            return arr[i];
        }
    }

    return nullptr;
}

// Hien thi tat ca nhan vien
void EmployeeManager::displayAll() const
{
    if (size == 0)
    {
        cout << "No employees available!" << endl;
        return;
    }

    cout << left
         << setw(12) << "ID"
         << setw(25) << "Name"
         << setw(30) << "Address"
         << setw(15) << "Base salary"
         << setw(15) << "Position"
         << setw(12) << "Status"
         << endl;

    cout << string(109, '-') << endl;

    for (int i = 0; i < size; i++)
    {
        arr[i]->Show();
    }
}

// Tinh tong quy luong co ban
long long EmployeeManager::calculateTotalSalaryFund() const
{
    long long total = 0;

    for (int i = 0; i < size; i++)
    {
        if (arr[i]->isActive())
        {
            total += arr[i]->calculate_salary();
        }
    }

    return total;
}

// Hien thi theo chuc vu
void EmployeeManager::displayByPosition(
    const string& position
) const
{
    bool found = false;

    for (int i = 0; i < size; i++)
    {
        if (arr[i]->get_position() == position)
        {
            arr[i]->Show();
            found = true;
        }
    }

    if (!found)
    {
        cout << "No employees found for this position!"
             << endl;
    }
}

// Kiem tra ID da ton tai
bool EmployeeManager::isEmployeeIdExist(
    const string& id
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i]->get_id() == id)
        {
            return true;
        }
    }

    return false;
}
