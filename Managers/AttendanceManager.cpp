
#include "AttendanceManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// Constructor
AttendanceManager::AttendanceManager()
{
    capacity = 5;
    size = 0;
    arr = new Attendance[capacity];
}

// Copy constructor
AttendanceManager::AttendanceManager(
    const AttendanceManager& other
)
{
    size = other.size;
    capacity = other.capacity;
    arr = new Attendance[capacity];

    for (int i = 0; i < size; i++)
    {
        arr[i] = other.arr[i];
    }
}

// Assignment operator
AttendanceManager& AttendanceManager::operator=(
    const AttendanceManager& other
)
{
    if (this == &other)
    {
        return *this;
    }

    Attendance* newArr = new Attendance[other.capacity];

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
AttendanceManager::~AttendanceManager()
{
    delete[] arr;
}

// Mo rong mang
void AttendanceManager::resize()
{
    int newCapacity = capacity * 2;

    Attendance* newArr = new Attendance[newCapacity];

    for (int i = 0; i < size; i++)
    {
        newArr[i] = arr[i];
    }

    delete[] arr;

    arr = newArr;
    capacity = newCapacity;
}

// Tao ma Attendance
string AttendanceManager::generateAttendanceId() const
{
    int number = 1;
    string attendanceId;

    do
    {
        ostringstream oss;

        oss << "AT"
            << setw(4) << setfill('0')
            << number;

        attendanceId = oss.str();
        number++;

    } while (findAttendanceById(attendanceId) != nullptr);

    return attendanceId;
}

// Getter
int AttendanceManager::getSize() const
{
    return size;
}

int AttendanceManager::getCapacity() const
{
    return capacity;
}

Attendance* AttendanceManager::getArr()
{
    return arr;
}

const Attendance* AttendanceManager::getArr() const
{
    return arr;
}

// Nap chong toan tu
AttendanceManager& AttendanceManager::operator+=(
    const Attendance& attendance
)
{
    addAttendance(attendance);
    return *this;
}

AttendanceManager& AttendanceManager::operator-=(
    const string& attendanceId
)
{
    deleteAttendance(attendanceId);
    return *this;
}

Attendance& AttendanceManager::operator[](int index)
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Attendance index out of range!"
        );
    }

    return arr[index];
}

const Attendance& AttendanceManager::operator[](
    int index
) const
{
    if (index < 0 || index >= size)
    {
        throw out_of_range(
            "Attendance index out of range!"
        );
    }

    return arr[index];
}

// Tim Attendance theo ID
Attendance* AttendanceManager::findAttendanceById(
    const string& attendanceId
)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAttendance_id() == attendanceId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

const Attendance* AttendanceManager::findAttendanceById(
    const string& attendanceId
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAttendance_id() == attendanceId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

// Tim Attendance theo nhan vien va Assignment
Attendance* AttendanceManager::findAttendance(
    const string& employeeId,
    const string& assignmentId
)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getEmployee_id() == employeeId &&
            arr[i].getAssignment_id() == assignmentId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

const Attendance* AttendanceManager::findAttendance(
    const string& employeeId,
    const string& assignmentId
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getEmployee_id() == employeeId &&
            arr[i].getAssignment_id() == assignmentId)
        {
            return &arr[i];
        }
    }

    return nullptr;
}

// Them Attendance
// EmployeeWorkSystem goi ham nay khi them nhan vien vao Assignment
void AttendanceManager::addAttendance(
    const Attendance& attendance
)
{
    if (attendance.getAttendance_id().empty() ||
        attendance.getEmployee_id().empty() ||
        attendance.getAssignment_id().empty() ||
        attendance.getAttendance_date().empty())
    {
        throw invalid_argument(
            "Invalid attendance information!"
        );
    }

    if (!Attendance::isValidStatus(
            attendance.getAttendance_status()))
    {
        throw invalid_argument(
            "Invalid attendance status!"
        );
    }

    if (findAttendanceById(
            attendance.getAttendance_id()) != nullptr)
    {
        throw invalid_argument(
            "Attendance ID already exists!"
        );
    }

    if (findAttendance(
            attendance.getEmployee_id(),
            attendance.getAssignment_id()) != nullptr)
    {
        throw invalid_argument(
            "Attendance record already exists!"
        );
    }

    if (size == capacity)
    {
        resize();
    }

    arr[size] = attendance;
    size++;
}

// Check-in
void AttendanceManager::checkIn(
    const string& employeeId,
    const string& assignmentId,
    const string& date,
    const string& time,
    const string& status
)
{
    if (employeeId.empty() ||
        assignmentId.empty() ||
        date.empty() ||
        !Attendance::isValidTime(time))
    {
        throw invalid_argument(
            "Invalid Check-in information! Time must use HH:MM."
        );
    }

    if (status != "OnTime" && status != "Late")
    {
        throw invalid_argument(
            "Invalid check-in status!"
        );
    }

    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        throw invalid_argument(
            "Attendance record not found!"
        );
    }

    if (attendance->getAttendance_date() != date)
    {
        throw invalid_argument(
            "Attendance date does not match!"
        );
    }

    if (attendance->getAttendance_status() != "Pending" ||
        !attendance->getCheck_in().empty())
    {
        throw invalid_argument(
            "Employee has already checked in or cannot check in!"
        );
    }

    attendance->setCheck_in(time);
    attendance->setAttendance_status(status);
}

// Check-out
void AttendanceManager::checkOut(
    const string& employeeId,
    const string& assignmentId,
    const string& date,
    const string& time
)
{
    if (!Attendance::isValidTime(time))
    {
        throw invalid_argument(
            "Invalid Check-out time! Expected HH:MM."
        );
    }

    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        throw invalid_argument(
            "Attendance record not found!"
        );
    }

    if (attendance->getAttendance_date() != date)
    {
        throw invalid_argument(
            "Attendance date does not match!"
        );
    }

    if (attendance->getAttendance_status() != "OnTime" &&
        attendance->getAttendance_status() != "Late")
    {
        throw invalid_argument(
            "Attendance status does not allow check-out!"
        );
    }

    if (attendance->getCheck_in().empty())
    {
        throw invalid_argument(
            "Employee has not checked in!"
        );
    }

    if (!attendance->getCheck_out().empty())
    {
        throw invalid_argument(
            "Employee has already checked out!"
        );
    }

    if (time <= attendance->getCheck_in())
    {
        throw invalid_argument(
            "Check-out must be later than check-in!"
        );
    }

    attendance->setCheck_out(time);
}

// Manager ghi nhan hoac hieu chinh Check-in.
// Khong gioi han ngay thao tac; date la ngay Assignment.
void AttendanceManager::setCheckInByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& date,
    const string& time,
    const string& status
)
{
    if (employeeId.empty() ||
        assignmentId.empty() ||
        date.empty() ||
        !Attendance::isValidTime(time))
    {
        throw invalid_argument(
            "Invalid Manager Check-in information!"
        );
    }

    if (status != "OnTime" && status != "Late")
    {
        throw invalid_argument(
            "Manager Check-in status must be OnTime or Late!"
        );
    }

    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        throw invalid_argument(
            "Attendance record not found!"
        );
    }

    if (attendance->getAttendance_date() != date)
    {
        throw invalid_argument(
            "Attendance date does not match the Assignment date!"
        );
    }

    if (attendance->getAttendance_status() == "Cancelled")
    {
        throw invalid_argument(
            "Cannot record attendance for a cancelled Assignment!"
        );
    }

    // Neu da co Check-out, gio Check-in moi van phai som hon.
    if (!attendance->getCheck_out().empty() &&
        time >= attendance->getCheck_out())
    {
        throw invalid_argument(
            "Check-in must be earlier than the recorded Check-out!"
        );
    }

    attendance->setCheck_in(time);
    attendance->setAttendance_status(status);
}

// Manager ghi nhan hoac hieu chinh Check-out.
// Cho phep cap nhat ban ghi MissingCheckOut sau khi Manager bo sung gio.
void AttendanceManager::setCheckOutByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& date,
    const string& time,
    const string& status
)
{
    if (employeeId.empty() ||
        assignmentId.empty() ||
        date.empty() ||
        !Attendance::isValidTime(time))
    {
        throw invalid_argument(
            "Invalid Manager Check-out information!"
        );
    }

    if (status != "OnTime" && status != "Late")
    {
        throw invalid_argument(
            "Manager Check-out status must be OnTime or Late!"
        );
    }

    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        throw invalid_argument(
            "Attendance record not found!"
        );
    }

    if (attendance->getAttendance_date() != date)
    {
        throw invalid_argument(
            "Attendance date does not match the Assignment date!"
        );
    }

    if (attendance->getAttendance_status() == "Cancelled")
    {
        throw invalid_argument(
            "Cannot record attendance for a cancelled Assignment!"
        );
    }

    if (attendance->getCheck_in().empty())
    {
        throw invalid_argument(
            "Cannot record Check-out before Check-in!"
        );
    }

    if (time <= attendance->getCheck_in())
    {
        throw invalid_argument(
            "Check-out must be later than Check-in!"
        );
    }

    attendance->setCheck_out(time);
    attendance->setAttendance_status(status);
}

// Danh dau vang mat
bool AttendanceManager::markAbsent(
    const string& employeeId,
    const string& assignmentId
)
{
    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        return false;
    }

    if (!attendance->getCheck_in().empty())
    {
        return false;
    }

    if (attendance->getAttendance_status() != "Pending")
    {
        return false;
    }

    attendance->setAttendance_status("Absent");

    return true;
}

// Danh dau thieu check-out
bool AttendanceManager::markMissingCheckOut(
    const string& employeeId,
    const string& assignmentId
)
{
    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        return false;
    }

    if (attendance->getCheck_in().empty() ||
        !attendance->getCheck_out().empty())
    {
        return false;
    }

    if (attendance->getAttendance_status() != "OnTime" &&
        attendance->getAttendance_status() != "Late")
    {
        return false;
    }

    attendance->setAttendance_status("MissingCheckOut");

    return true;
}

// Huy Attendance khi Assignment bi huy
void AttendanceManager::cancelAttendanceForAssignment(
    const string& assignmentId
)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAssignment_id() == assignmentId)
        {
            arr[i].setAttendance_status("Cancelled");
        }
    }
}

// Xoa Attendance
bool AttendanceManager::deleteAttendance(
    const string& attendanceId
)
{
    int index = -1;

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAttendance_id() == attendanceId)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        return false;
    }

    // Chi xoa Attendance Pending chua check-in/check-out
    if (arr[index].getAttendance_status() != "Pending" ||
        !arr[index].getCheck_in().empty() ||
        !arr[index].getCheck_out().empty())
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

// Go Attendance khi xoa nhan vien khoi Assignment
bool AttendanceManager::removeAttendanceForEmployee(
    const string& employeeId,
    const string& assignmentId
)
{
    Attendance* attendance = findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr)
    {
        return false;
    }

    return deleteAttendance(
        attendance->getAttendance_id()
    );
}

// Tim theo nhan vien
void AttendanceManager::findByEmployeeId(
    const string& employeeId
) const
{
    bool found = false;

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getEmployee_id() == employeeId)
        {
            arr[i].Show();
            cout << "--------------------" << endl;
            found = true;
        }
    }

    if (!found)
    {
        cout << "No attendance records found!" << endl;
    }
}

// Hien thi tat ca
void AttendanceManager::displayAll() const
{
    if (size == 0)
    {
        cout << "No attendance records available!" << endl;
        return;
    }

    for (int i = 0; i < size; i++)
    {
        arr[i].Show();
        cout << "--------------------" << endl;
    }
}

// Hien thi dang bang
void AttendanceManager::displayAttendanceTable() const
{
    cout << left
         << setw(12) << "ID"
         << setw(12) << "Employee"
         << setw(12) << "Assignment"
         << setw(15) << "Date"
         << setw(12) << "Check-in"
         << setw(12) << "Check-out"
         << setw(20) << "Status"
         << endl;

    cout << string(95, '-') << endl;

    for (int i = 0; i < size; i++)
    {
        cout << left
             << setw(12) << arr[i].getAttendance_id()
             << setw(12) << arr[i].getEmployee_id()
             << setw(12) << arr[i].getAssignment_id()
             << setw(15) << arr[i].getAttendance_date()
             << setw(12) << arr[i].getCheck_in()
             << setw(12) << arr[i].getCheck_out()
             << setw(20) << arr[i].getAttendance_status()
             << endl;
    }
}

// Hien thi theo ngay
void AttendanceManager::displayByDate(
    const string& date
) const
{
    bool found = false;

    for (int i = 0; i < size; i++)
    {
        if (arr[i].getAttendance_date() == date)
        {
            arr[i].Show();
            cout << "--------------------" << endl;
            found = true;
        }
    }

    if (!found)
    {
        cout << "No attendance records found!" << endl;
    }
}

// Kiem tra Attendance theo Assignment
bool AttendanceManager::hasAttendanceForAssignment(
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

// Kiem tra lich su Attendance theo Employee
bool AttendanceManager::hasAttendanceForEmployee(
    const string& employeeId
) const
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].getEmployee_id() == employeeId)
        {
            return true;
        }
    }

    return false;
}
