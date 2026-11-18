
#include "AssignmentManager.h"
#include <iostream>
#include <stdexcept>
#include <cctype>

namespace
{
    bool isBlank(const string& value)
    {
        for (size_t i = 0; i < value.length(); i++)
        {
            if (!isspace(static_cast<unsigned char>(value[i])))
            {
                return false;
            }
        }
        return true;
    }
}

// Constructor
AssignmentManager::AssignmentManager()
{
    capacity = 5;
    size = 0;
    arr = new Assignment[capacity];
}

// Copy constructor
AssignmentManager::AssignmentManager(
    const AssignmentManager& other
)
{
    size = other.size;
    capacity = other.capacity;
    arr = new Assignment[capacity];

    for (int i = 0; i < size; i++)
    {
        arr[i] = other.arr[i];
    }
}

// Assignment operator
AssignmentManager& AssignmentManager::operator=(
    const AssignmentManager& other
)
{
    if (this == &other)
    {
        return *this;
    }

    Assignment* newArr = new Assignment[other.capacity];

    for (int i = 0; i < other.size; i++)
    {
        newArr[i] = other.arr[i];
    }

    delete[] arr;

    arr = newArr;
    size = other.size;
    capacity = other.capacity;

    return *this;
}

// Destructor
AssignmentManager::~AssignmentManager()
{
    delete[] arr;
}

// Mo rong mang
void AssignmentManager::resize()
{
    int newCapacity = capacity * 2;

    Assignment* newArr = new Assignment[newCapacity];

    for (int i = 0; i < size; i++)
    {
        newArr[i] = arr[i];
    }

    delete[] arr;

    arr = newArr;
    capacity = newCapacity;
}

// Getter
int AssignmentManager::getSize() const
{
    return size;
}

int AssignmentManager::getCapacity() const
{
    return capacity;
}

Assignment* AssignmentManager::getArr()
{
    return arr;
}

const Assignment* AssignmentManager::getArr() const
{
    return arr;
}

// Nap chong toan tu
AssignmentManager& AssignmentManager::operator+=(
    const Assignment& assignment
)
{
    addAssignment(assignment);
    return *this;
}

AssignmentManager& AssignmentManager::operator-=(
    const string& assignmentId
)
{
    deleteAssignment(assignmentId);
    return *this;
}

Assignment& AssignmentManager::operator[](int index)
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Assignment index out of range!"
        );
    }

    return arr[index];
}

const Assignment& AssignmentManager::operator[](
    int index
) const
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Assignment index out of range!"
        );
    }

    return arr[index];
}

// Them Assignment
void AssignmentManager::addAssignment(
    const Assignment& assignment
)
{
    if (isBlank(assignment.getAssignment_id()))
    {
        throw invalid_argument(
            "Assignment ID cannot be empty!"
        );
    }

    if (isBlank(assignment.getAssignment_date()))
    {
        throw invalid_argument(
            "Assignment date cannot be empty!"
        );
    }

    if (isBlank(assignment.getShift_id()))
    {
        throw invalid_argument(
            "Shift ID cannot be empty!"
        );
    }

    if (isAssignmentIdExist(assignment.getAssignment_id()))
    {
        throw invalid_argument(
            "Assignment ID already exists!"
        );
    }

    if (checkConflict(assignment))
    {
        throw invalid_argument(
            "Assignment schedule conflict!"
        );
    }

    if (size == capacity)
    {
        resize();
    }

    arr[size] = assignment;
    size++;
}

// Chinh sua Assignment
bool AssignmentManager::editAssignment(
    const string& assignmentId,
    const Assignment& updatedAssignment
)
{
    Assignment* current = findAssignmentById(assignmentId);

    if (current == nullptr)
    {
        return false;
    }

    if (updatedAssignment.getAssignment_id() != assignmentId)
    {
        throw invalid_argument(
            "Assignment ID cannot be changed!"
        );
    }

    // Khong cho phep chinh sua Assignment da ket thuc
    if (current->getAssignment_status() == "Completed" ||
        current->getAssignment_status() == "Cancelled")
    {
        throw invalid_argument(
            "Cannot edit a finished Assignment!"
        );
    }

    // Trang thai chi duoc thay doi qua workflow cua EmployeeWorkSystem.
    if (updatedAssignment.getAssignment_status() !=
        current->getAssignment_status())
    {
        throw invalid_argument(
            "Assignment status cannot be changed through editAssignment!"
        );
    }

    if (isBlank(updatedAssignment.getAssignment_date()) ||
        isBlank(updatedAssignment.getShift_id()))
    {
        throw invalid_argument(
            "Assignment date and shift cannot be empty!"
        );
    }

    if (checkConflict(updatedAssignment))
    {
        throw invalid_argument(
            "Assignment schedule conflict!"
        );
    }

    *current = updatedAssignment;

    return true;
}

// Xoa Assignment
bool AssignmentManager::deleteAssignment(
    const string& assignmentId
)
{
    int index = -1;

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == assignmentId)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        return false;
    }

    // Chi cho phep xoa vat ly Assignment dang Planned
    // Assignment co lich su phai duoc giu lai
    if (arr[index].getAssignment_status() != "Planned")
    {
        return false;
    }

    for (int i = index; i < size - 1; i++)
    {
        arr[i] = arr[i + 1];
    }

    size--;

    return true;
}

// Tim Assignment theo ID
Assignment* AssignmentManager::findAssignmentById(
    const string& assignmentId
)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == assignmentId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

const Assignment* AssignmentManager::findAssignmentById(
    const string& assignmentId
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == assignmentId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

// Hien thi tat ca Assignment
void AssignmentManager::displayAll() const
{
    if (size == 0)
    {
        cout << "No Assignment available!" << endl;
        return;
    }

    for (int i = 0; i < size; i++)
    {
        cout << "\nAssignment " << i + 1 << endl;
        arr[i].Show();
    }
}

// Kiem tra trung lich phuong tien
bool AssignmentManager::checkVehicleConflict(
    const string& vehicleId,
    const string& date,
    const string& shiftId,
    const string& excludeAssignmentId
) const
{
    if (vehicleId.empty())
    {
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == excludeAssignmentId)
        {
            continue;
        }

        if (arr[i].getAssignment_status() == "Cancelled")
        {
            continue;
        }

        if (arr[i].getVehicle_id() == vehicleId &&
            arr[i].getAssignment_date() == date &&
            arr[i].getShift_id() == shiftId)
        {
            return true;
        }
    }

    return false;
}

// Kiem tra trung lich nhan vien
bool AssignmentManager::checkEmployeeConflict(
    const string& employeeId,
    const string& date,
    const string& shiftId,
    const string& excludeAssignmentId
) const
{
    if (employeeId.empty())
    {
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == excludeAssignmentId)
        {
            continue;
        }

        if (arr[i].getAssignment_status() == "Cancelled")
        {
            continue;
        }

        if (arr[i].getAssignment_date() == date &&
            arr[i].getShift_id() == shiftId &&
            arr[i].hasEmployee(employeeId))
        {
            return true;
        }
    }

    return false;
}

// Kiem tra trung lich tong hop
bool AssignmentManager::checkConflict(
    const Assignment& assignment
) const
{
    if (assignment.getAssignment_status() == "Cancelled")
    {
        return false;
    }

    string assignmentId = assignment.getAssignment_id();
    string date = assignment.getAssignment_date();
    string shiftId = assignment.getShift_id();
    string vehicleId = assignment.getVehicle_id();

    // Kiem tra trung lich phuong tien
    if (checkVehicleConflict(
            vehicleId,
            date,
            shiftId,
            assignmentId))
    {
        return true;
    }

    // Kiem tra trung lich tung nhan vien
    const vector<string>& employeeIds =
        assignment.getEmployee_ids();

    for (size_t i = 0; i < employeeIds.size(); i++)
    {
        if (checkEmployeeConflict(
                employeeIds[i],
                date,
                shiftId,
                assignmentId))
        {
            return true;
        }
    }

    return false;
}

// Kiem tra ma Assignment
bool AssignmentManager::isAssignmentIdExist(
    const string& assignmentId
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == assignmentId)
        {
            return true;
        }
    }

    return false;
}
