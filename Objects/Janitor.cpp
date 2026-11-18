
#include "Janitor.h"
#include <stdexcept>

// Constructor mac dinh
Janitor::Janitor()
    : Employee()
{
    position = "Janitor";
}

// Constructor moi
Janitor::Janitor(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address
)
    : Employee(
        employee_id,
        employee_name,
        employee_address,
        "Janitor"
    )
{
    validate();
}

// Constructor tuong thich voi code cu
Janitor::Janitor(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address,
    long long employee_salary
)
    : Janitor(
        employee_id,
        employee_name,
        employee_address
    )
{
    if (employee_salary != calculate_salary())
    {
        throw invalid_argument(
            "Janitor base salary must be 9000000 VND!"
        );
    }
}

// Clone
Employee* Janitor::clone() const
{
    return new Janitor(*this);
}

// Luong co ban Janitor
long long Janitor::calculate_salary() const
{
    return 9000000LL;
}

// Kiem tra du lieu Janitor
void Janitor::validate() const
{
    Employee::validate();

    if (position != "Janitor")
    {
        throw invalid_argument("Invalid Janitor position!");
    }
}

// Hien thi
void Janitor::Show() const
{
    Employee::Show();
}
