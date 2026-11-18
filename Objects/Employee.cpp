
#include "Employee.h"
#include <iomanip>
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

// Constructor mac dinh
Employee::Employee()
{
    employee_id = "";
    employee_name = "";
    employee_address = "";
    position = "";
    employee_status = "Active";
}

// Constructor co tham so
Employee::Employee(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address,
    const string& position
)
{
    this->employee_id = employee_id;
    this->employee_name = employee_name;
    this->employee_address = employee_address;
    this->position = position;
    this->employee_status = "Active";

    validate();
}

// Destructor
Employee::~Employee()
{
}

// Getter / Setter

string Employee::get_id() const
{
    return employee_id;
}

void Employee::set_id(const string& employee_id)
{
    if (isBlank(employee_id))
    {
        throw invalid_argument("Employee ID cannot be empty!");
    }

    this->employee_id = employee_id;
}

string Employee::get_name() const
{
    return employee_name;
}

void Employee::set_name(const string& employee_name)
{
    if (isBlank(employee_name))
    {
        throw invalid_argument("Employee name cannot be empty!");
    }

    this->employee_name = employee_name;
}

string Employee::get_address() const
{
    return employee_address;
}

void Employee::set_address(const string& employee_address)
{
    if (isBlank(employee_address))
    {
        throw invalid_argument("Employee address cannot be empty!");
    }

    this->employee_address = employee_address;
}

long long Employee::get_salary() const
{
    return calculate_salary();
}

void Employee::set_salary(long long employee_salary)
{
    if (employee_salary != calculate_salary())
    {
        throw invalid_argument(
            "Base salary is fixed according to employee position!"
        );
    }
}

string Employee::get_position() const
{
    return position;
}

// Trang thai nhan vien

string Employee::get_status() const
{
    return employee_status;
}

void Employee::set_status(const string& employee_status)
{
    if (employee_status != "Active" &&
        employee_status != "Inactive")
    {
        throw invalid_argument("Invalid employee status!");
    }

    this->employee_status = employee_status;
}

bool Employee::isActive() const
{
    return employee_status == "Active";
}

// Kiem tra du lieu nhan vien

void Employee::validate() const
{
    if (isBlank(employee_id))
    {
        throw invalid_argument("Employee ID cannot be empty!");
    }

    if (isBlank(employee_name))
    {
        throw invalid_argument("Employee name cannot be empty!");
    }

    if (isBlank(employee_address))
    {
        throw invalid_argument("Employee address cannot be empty!");
    }

    if (position != "Driver" &&
        position != "Janitor" &&
        position != "Manager")
    {
        throw invalid_argument("Invalid employee position!");
    }

    if (employee_status != "Active" &&
        employee_status != "Inactive")
    {
        throw invalid_argument("Invalid employee status!");
    }
}

// Hien thi thong tin nhan vien

void Employee::Show() const
{
    cout << left
         << setw(12) << employee_id
         << setw(25) << employee_name
         << setw(30) << employee_address
         << setw(15) << calculate_salary()
         << setw(15) << position
         << setw(12) << employee_status
         << endl;
}
