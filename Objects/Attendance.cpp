#include "Attendance.h"
#include <stdexcept>

// Kiem tra dinh dang gio HH:MM.
bool Attendance::isValidTime(const string& time)
{
    if (time.length() != 5 || time[2] != ':')
    {
        return false;
    }

    for (int i = 0; i < 5; i++)
    {
        if (i == 2)
        {
            continue;
        }

        if (time[i] < '0' || time[i] > '9')
        {
            return false;
        }
    }

    int hour = (time[0] - '0') * 10 + (time[1] - '0');
    int minute = (time[3] - '0') * 10 + (time[4] - '0');

    return hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59;
}

// Kiem tra du lieu gio truoc khi gan vao doi tuong.
namespace
{
    void validateAttendanceTimes(
        const string& checkIn,
        const string& checkOut
    )
    {
        if (!checkIn.empty() && !Attendance::isValidTime(checkIn))
        {
            throw invalid_argument(
                "Invalid check-in time! Expected HH:MM."
            );
        }

        if (!checkOut.empty() && !Attendance::isValidTime(checkOut))
        {
            throw invalid_argument(
                "Invalid check-out time! Expected HH:MM."
            );
        }

        if (!checkIn.empty() &&
            !checkOut.empty() &&
            checkOut <= checkIn)
        {
            throw invalid_argument(
                "Check-out must be later than check-in!"
            );
        }
    }
}

// Constructor mac dinh
Attendance::Attendance()
{
    attendance_id = "";
    employee_id = "";
    assignment_id = "";
    attendance_date = "";
    check_in = "";
    check_out = "";
    attendance_status = "Pending";
}

// Constructor giu tuong thich voi code cu
Attendance::Attendance(
    const string& attendance_id,
    const string& employee_id,
    const string& assignment_id,
    const string& attendance_date,
    const string& check_in,
    const string& check_out
)
{
    validateAttendanceTimes(check_in, check_out);

    this->attendance_id = attendance_id;
    this->employee_id = employee_id;
    this->assignment_id = assignment_id;
    this->attendance_date = attendance_date;
    this->check_in = check_in;
    this->check_out = check_out;

    attendance_status = "Pending";
}

// Constructor co trang thai
Attendance::Attendance(
    const string& attendance_id,
    const string& employee_id,
    const string& assignment_id,
    const string& attendance_date,
    const string& check_in,
    const string& check_out,
    const string& attendance_status
)
{
    if (!isValidStatus(attendance_status))
    {
        throw invalid_argument(
            "Invalid attendance status!"
        );
    }

    validateAttendanceTimes(check_in, check_out);

    this->attendance_id = attendance_id;
    this->employee_id = employee_id;
    this->assignment_id = assignment_id;
    this->attendance_date = attendance_date;
    this->check_in = check_in;
    this->check_out = check_out;
    this->attendance_status = attendance_status;
}

// Getter / Setter
string Attendance::getAttendance_id() const
{
    return attendance_id;
}

void Attendance::setAttendance_id(
    const string& attendance_id
)
{
    if (attendance_id.empty())
    {
        throw invalid_argument(
            "Attendance ID cannot be empty!"
        );
    }

    this->attendance_id = attendance_id;
}

string Attendance::getEmployee_id() const
{
    return employee_id;
}

void Attendance::setEmployee_id(
    const string& employee_id
)
{
    if (employee_id.empty())
    {
        throw invalid_argument(
            "Employee ID cannot be empty!"
        );
    }

    this->employee_id = employee_id;
}

string Attendance::getAssignment_id() const
{
    return assignment_id;
}

void Attendance::setAssignment_id(
    const string& assignment_id
)
{
    if (assignment_id.empty())
    {
        throw invalid_argument(
            "Assignment ID cannot be empty!"
        );
    }

    this->assignment_id = assignment_id;
}

string Attendance::getAttendance_date() const
{
    return attendance_date;
}

void Attendance::setAttendance_date(
    const string& attendance_date
)
{
    if (attendance_date.empty())
    {
        throw invalid_argument(
            "Attendance date cannot be empty!"
        );
    }

    this->attendance_date = attendance_date;
}

string Attendance::getCheck_in() const
{
    return check_in;
}

void Attendance::setCheck_in(
    const string& check_in
)
{
    // Chuoi rong co nghia la chua ghi nhan / xoa gia tri cu.
    if (!check_in.empty() && !isValidTime(check_in))
    {
        throw invalid_argument(
            "Invalid check-in time! Expected HH:MM."
        );
    }

    if (!check_in.empty() &&
        !check_out.empty() &&
        check_out <= check_in)
    {
        throw invalid_argument(
            "Check-in must be earlier than the existing check-out!"
        );
    }

    this->check_in = check_in;
}

string Attendance::getCheck_out() const
{
    return check_out;
}

void Attendance::setCheck_out(
    const string& check_out
)
{
    // Chuoi rong co nghia la chua ghi nhan / xoa gia tri cu.
    if (!check_out.empty() && !isValidTime(check_out))
    {
        throw invalid_argument(
            "Invalid check-out time! Expected HH:MM."
        );
    }

    if (!check_out.empty() &&
        !check_in.empty() &&
        check_out <= check_in)
    {
        throw invalid_argument(
            "Check-out must be later than check-in!"
        );
    }

    this->check_out = check_out;
}

string Attendance::getAttendance_status() const
{
    return attendance_status;
}

void Attendance::setAttendance_status(
    const string& attendance_status
)
{
    if (!isValidStatus(attendance_status))
    {
        throw invalid_argument(
            "Invalid attendance status!"
        );
    }

    this->attendance_status = attendance_status;
}

// Kiem tra trang thai hop le
bool Attendance::isValidStatus(
    const string& status
)
{
    return status == "Pending" ||
           status == "OnTime" ||
           status == "Late" ||
           status == "MissingCheckOut" ||
           status == "Absent" ||
           status == "Cancelled";
}

// Hien thi
void Attendance::Show() const
{
    cout << "Attendance ID: "
         << attendance_id << endl;

    cout << "Employee ID: "
         << employee_id << endl;

    cout << "Assignment ID: "
         << assignment_id << endl;

    cout << "Date: "
         << attendance_date << endl;

    cout << "Check-in: ";

    if (check_in.empty())
    {
        cout << "Not checked in" << endl;
    }
    else
    {
        cout << check_in << endl;
    }

    cout << "Check-out: ";

    if (check_out.empty())
    {
        cout << "Not checked out" << endl;
    }
    else
    {
        cout << check_out << endl;
    }

    cout << "Attendance status: "
         << attendance_status << endl;
}
