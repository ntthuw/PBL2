
#include "Assignment.h"
#include <stdexcept>
#include <algorithm>
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

// Constructor mac dinh
Assignment::Assignment()
{
    assignment_id = "";
    vehicle_id = "";
    shift_id = "";
    assignment_date = "";
    assignment_status = "Planned";
}

// Constructor co tham so
Assignment::Assignment(
    const string& assignment_id,
    const vector<string>& employee_ids,
    const string& vehicle_id,
    const string& shift_id,
    const string& assignment_date,
    const string& assignment_status
)
{
    if (!isValidStatus(assignment_status))
    {
        throw invalid_argument(
            "Invalid assignment status!"
        );
    }

    this->assignment_id = assignment_id;
    this->vehicle_id = vehicle_id;
    this->shift_id = shift_id;
    this->assignment_date = assignment_date;
    this->assignment_status = assignment_status;
    for (size_t i = 0; i < employee_ids.size(); i++)
    {
        if (isBlank(employee_ids[i]))
        {
            throw invalid_argument(
                "Employee ID cannot be empty!"
            );
        }

        if (hasEmployee(employee_ids[i]))
        {
            throw invalid_argument(
                "Duplicate employee ID in assignment!"
            );
        }

        this->employee_ids.push_back(employee_ids[i]);
    }
}

// Getter / Setter
string Assignment::getAssignment_id() const
{
    return assignment_id;
}

void Assignment::setAssignment_id(
    const string& assignment_id
)
{
    if (isBlank(assignment_id))
    {
        throw invalid_argument(
            "Assignment ID cannot be empty!"
        );
    }

    this->assignment_id = assignment_id;
}

// Quan ly danh sach nhan vien
const vector<string>& Assignment::getEmployee_ids() const
{
    return employee_ids;
}

int Assignment::getEmployeeCount() const
{
    return static_cast<int>(employee_ids.size());
}

string Assignment::getEmployee_id(int index) const
{
    if (index < 0 ||
        index >= static_cast<int>(employee_ids.size()))
    {
        throw out_of_range(
            "Employee index out of range!"
        );
    }

    return employee_ids[index];
}

bool Assignment::hasEmployee(
    const string& employee_id
) const
{
    return find(
        employee_ids.begin(),
        employee_ids.end(),
        employee_id
    ) != employee_ids.end();
}

bool Assignment::addEmployee(
    const string& employee_id
)
{
    if (isBlank(employee_id) || hasEmployee(employee_id))
    {
        return false;
    }

    employee_ids.push_back(employee_id);
    return true;
}

bool Assignment::removeEmployee(
    const string& employee_id
)
{
    vector<string>::iterator it = find(
        employee_ids.begin(),
        employee_ids.end(),
        employee_id
    );

    if (it == employee_ids.end())
    {
        return false;
    }

    employee_ids.erase(it);
    return true;
}

// Quan ly phuong tien
string Assignment::getVehicle_id() const
{
    return vehicle_id;
}

void Assignment::setVehicle_id(
    const string& vehicle_id
)
{
    // Cho phep chuoi rong de go phuong tien
    // khi Assignment chua duoc xac nhan.
    this->vehicle_id = vehicle_id;
}

// Quan ly ca lam
string Assignment::getShift_id() const
{
    return shift_id;
}

void Assignment::setShift_id(
    const string& shift_id
)
{
    if (isBlank(shift_id))
    {
        throw invalid_argument(
            "Shift ID cannot be empty!"
        );
    }

    this->shift_id = shift_id;
}

// Quan ly ngay lam
string Assignment::getAssignment_date() const
{
    return assignment_date;
}

void Assignment::setAssignment_date(
    const string& assignment_date
)
{
    if (isBlank(assignment_date))
    {
        throw invalid_argument(
            "Assignment date cannot be empty!"
        );
    }

    this->assignment_date = assignment_date;
}

// Quan ly trang thai
string Assignment::getAssignment_status() const
{
    return assignment_status;
}

void Assignment::setAssignment_status(
    const string& assignment_status
)
{
    if (!isValidStatus(assignment_status))
    {
        throw invalid_argument(
            "Invalid assignment status!"
        );
    }

    this->assignment_status = assignment_status;
}

bool Assignment::isValidStatus(
    const string& status
)
{
    return status == "Planned" ||
           status == "Assigned" ||
           status == "Cancelled" ||
           status == "Completed";
}
// Hien thi

void Assignment::Show() const
{
    cout << "Assignment ID: "
         << assignment_id << endl;

    cout << "Vehicle ID: "
         << (vehicle_id.empty() ? "Not assigned" : vehicle_id)
         << endl;

    cout << "Shift ID: "
         << shift_id << endl;

    cout << "Date: "
         << assignment_date << endl;

    cout << "Status: "
         << assignment_status << endl;

    cout << "Employee IDs: ";

    if (employee_ids.empty())
    {
        cout << "No employees assigned";
    }
    else
    {
        for (size_t i = 0; i < employee_ids.size(); i++)
        {
            cout << employee_ids[i];

            if (i < employee_ids.size() - 1)
            {
                cout << ", ";
            }
        }
    }

    cout << endl;
}
