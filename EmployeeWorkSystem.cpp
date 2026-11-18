
#include "EmployeeWorkSystem.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <sstream>
#include <iomanip>

using namespace std;

// ============================================================
// CAC HAM HO TRO NOI BO
// ============================================================

namespace
{
    // Tao ma Attendance tiep theo: AT0001, AT0002, ...
    string generateAttendanceId(
        const AttendanceManager& attendanceManager
    )
    {
        int number = 1;
        string attendanceId;

        do
        {
            ostringstream output;
            output << "AT"
                   << setw(4) << setfill('0')
                   << number;

            attendanceId = output.str();
            number++;
        }
        while (
            attendanceManager.findAttendanceById(attendanceId)
            != nullptr
        );

        return attendanceId;
    }

    // Lay ngay hien tai theo dinh dang YYYY-MM-DD
    string getCurrentDate()
    {
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);

        if (localTime == nullptr)
        {
            throw runtime_error(
                "Cannot get current local date!"
            );
        }

        ostringstream output;
        output << setfill('0')
               << setw(4) << localTime->tm_year + 1900
               << "-"
               << setw(2) << localTime->tm_mon + 1
               << "-"
               << setw(2) << localTime->tm_mday;

        return output.str();
    }

    // Lay gio hien tai theo dinh dang HH:MM
    string getCurrentTime()
    {
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);

        if (localTime == nullptr)
        {
            throw runtime_error(
                "Cannot get current local time!"
            );
        }

        ostringstream output;
        output << setfill('0')
               << setw(2) << localTime->tm_hour
               << ":"
               << setw(2) << localTime->tm_min;

        return output.str();
    }

    // Chuyen HH:MM thanh so phut trong ngay
    int convertTimeToMinutes(const string& time)
    {
        if (time.length() != 5 || time[2] != ':')
        {
            throw invalid_argument(
                "Invalid time format! Expected HH:MM."
            );
        }

        int hour = -1;
        int minute = -1;

        if (sscanf(
            time.c_str(),
            "%2d:%2d",
            &hour,
            &minute
        ) != 2)
        {
            throw invalid_argument("Invalid time!");
        }

        if (hour < 0 || hour > 23 ||
            minute < 0 || minute > 59)
        {
            throw invalid_argument("Invalid time!");
        }

        return hour * 60 + minute;
    }

    // Kiem tra va chuyen YYYY-MM-DD thanh tm
    bool parseDate(const string& date, tm& result)
    {
        int year = 0;
        int month = 0;
        int day = 0;

        if (date.length() != 10 ||
            sscanf(
                date.c_str(),
                "%4d-%2d-%2d",
                &year,
                &month,
                &day
            ) != 3)
        {
            return false;
        }

        if (date[4] != '-' || date[7] != '-')
        {
            return false;
        }

        tm parsed = {};
        parsed.tm_year = year - 1900;
        parsed.tm_mon = month - 1;
        parsed.tm_mday = day;
        parsed.tm_hour = 12;
        parsed.tm_isdst = -1;

        time_t converted = mktime(&parsed);

        if (converted == static_cast<time_t>(-1))
        {
            return false;
        }

        // mktime tu dieu chinh ngay khong hop le,
        // nen can so sanh lai cac thanh phan.
        if (parsed.tm_year != year - 1900 ||
            parsed.tm_mon != month - 1 ||
            parsed.tm_mday != day)
        {
            return false;
        }

        result = parsed;
        return true;
    }

    // So sanh hai ngay dang YYYY-MM-DD
    int compareDates(const string& first, const string& second)
    {
        if (first < second) return -1;
        if (first > second) return 1;
        return 0;
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

EmployeeWorkSystem::EmployeeWorkSystem(
    EmployeeManager& employeeManager,
    AssignmentManager& assignmentManager,
    AttendanceManager& attendanceManager,
    VehicleManager& vehicleManager
)
    : employeeManager(employeeManager),
      assignmentManager(assignmentManager),
      attendanceManager(attendanceManager),
      vehicleManager(vehicleManager)
{
    syncVehicleStatuses();
}

// ============================================================
// KIEM TRA THOI GIAN CA LAM
// ============================================================

bool EmployeeWorkSystem::getShiftTime(
    const string& shiftId,
    int& startHour,
    int& startMinute,
    int& endHour,
    int& endMinute
) const
{
    if (shiftId == "CA01")
    {
        startHour = 5;
        startMinute = 30;
        endHour = 12;
        endMinute = 0;
        return true;
    }

    if (shiftId == "CA02")
    {
        startHour = 12;
        startMinute = 0;
        endHour = 15;
        endMinute = 0;
        return true;
    }

    if (shiftId == "CA03")
    {
        startHour = 17;
        startMinute = 0;
        endHour = 21;
        endMinute = 0;
        return true;
    }

    return false;
}

bool EmployeeWorkSystem::hasShiftStarted(
    const string& date,
    const string& shiftId
) const
{
    tm assignmentDate = {};

    if (!parseDate(date, assignmentDate))
    {
        throw invalid_argument(
            "Invalid assignment date! Use YYYY-MM-DD."
        );
    }

    int startHour, startMinute, endHour, endMinute;

    if (!getShiftTime(
        shiftId,
        startHour,
        startMinute,
        endHour,
        endMinute
    ))
    {
        throw invalid_argument("Invalid shift ID!");
    }

    string today = getCurrentDate();

    int comparison = compareDates(date, today);

    if (comparison < 0)
    {
        return true;
    }

    if (comparison > 0)
    {
        return false;
    }

    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    if (localTime == nullptr)
    {
        throw runtime_error("Cannot get current local time!");
    }

    int currentMinutes =
        localTime->tm_hour * 60 + localTime->tm_min;

    int shiftStartMinutes =
        startHour * 60 + startMinute;

    return currentMinutes >= shiftStartMinutes;
}

bool EmployeeWorkSystem::hasShiftEnded(
    const string& date,
    const string& shiftId
) const
{
    tm assignmentDate = {};

    if (!parseDate(date, assignmentDate))
    {
        throw invalid_argument(
            "Invalid assignment date! Use YYYY-MM-DD."
        );
    }

    int startHour, startMinute, endHour, endMinute;

    if (!getShiftTime(
        shiftId,
        startHour,
        startMinute,
        endHour,
        endMinute
    ))
    {
        throw invalid_argument("Invalid shift ID!");
    }

    string today = getCurrentDate();

    int comparison = compareDates(date, today);

    if (comparison < 0)
    {
        return true;
    }

    if (comparison > 0)
    {
        return false;
    }

    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    if (localTime == nullptr)
    {
        throw runtime_error("Cannot get current local time!");
    }

    int currentMinutes =
        localTime->tm_hour * 60 + localTime->tm_min;

    int shiftEndMinutes =
        endHour * 60 + endMinute;

    return currentMinutes >= shiftEndMinutes;
}

// ============================================================
// KIEM TRA ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::canModifyAssignment(
    const string& assignmentId
) const
{
    const Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    string status = assignment->getAssignment_status();

    if (status != "Planned" && status != "Assigned")
    {
        return false;
    }

    return !hasShiftStarted(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    );
}

void EmployeeWorkSystem::validateTeamComposition(
    const vector<string>& employeeIds
) const
{
    if (employeeIds.size() != 3)
    {
        throw invalid_argument(
            "A collection team must have exactly 3 employees!"
        );
    }

    int driverCount = 0;
    int janitorCount = 0;

    for (size_t i = 0; i < employeeIds.size(); i++)
    {
        const Employee* employee =
            employeeManager.findEmployeeById(employeeIds[i]);

        if (employee == nullptr)
        {
            throw invalid_argument(
                "Employee does not exist: " + employeeIds[i]
            );
        }

        if (!employee->isActive())
        {
            throw invalid_argument(
                "Inactive employee cannot join an assignment!"
            );
        }

        if (employee->get_position() == "Driver")
        {
            driverCount++;
        }
        else if (employee->get_position() == "Janitor")
        {
            janitorCount++;
        }
        else
        {
            throw invalid_argument(
                "Manager cannot join a collection team!"
            );
        }
    }

    if (driverCount != 1 || janitorCount != 2)
    {
        throw invalid_argument(
            "A team must have exactly 1 Driver and 2 Janitors!"
        );
    }
}

void EmployeeWorkSystem::validateEmployeeAvailability(
    const string& employeeId,
    const string& date,
    const string& shiftId,
    const string& excludeAssignmentId
) const
{
    const Employee* employee =
        employeeManager.findEmployeeById(employeeId);

    if (employee == nullptr)
    {
        throw invalid_argument("Employee does not exist!");
    }

    if (!employee->isActive())
    {
        throw invalid_argument(
            "Inactive employee cannot be assigned!"
        );
    }

    if (employee->get_position() == "Manager")
    {
        throw invalid_argument(
            "Manager cannot join a collection team!"
        );
    }

    if (assignmentManager.checkEmployeeConflict(
        employeeId,
        date,
        shiftId,
        excludeAssignmentId
    ))
    {
        throw invalid_argument(
            "Employee already has another assignment in this shift!"
        );
    }
}

void EmployeeWorkSystem::validateVehicleAvailability(
    const string& vehicleId,
    const string& date,
    const string& shiftId,
    const string& excludeAssignmentId
) const
{
    if (vehicleId.empty())
    {
        throw invalid_argument("Vehicle ID cannot be empty!");
    }

    const Vehicle* vehicle =
        vehicleManager.findVehicleById(vehicleId);

    if (vehicle == nullptr)
    {
        throw invalid_argument("Vehicle does not exist!");
    }

    if (vehicle->getVehicle_status() == "Maintenance")
    {
        throw invalid_argument(
            "Vehicle is under maintenance!"
        );
    }

    if (assignmentManager.checkVehicleConflict(
        vehicleId,
        date,
        shiftId,
        excludeAssignmentId
    ))
    {
        throw invalid_argument(
            "Vehicle already has another assignment in this shift!"
        );
    }
}

// ============================================================
// EMPLOYEE
// ============================================================

void EmployeeWorkSystem::addEmployee(
    const Employee& employee
)
{
    employeeManager.addEmployee(employee);
}

bool EmployeeWorkSystem::editEmployee(
    const string& employeeId,
    const Employee& updatedEmployee
)
{
    const Employee* currentEmployee =
        employeeManager.findEmployeeById(employeeId);

    if (currentEmployee == nullptr)
    {
        return false;
    }

    if (updatedEmployee.get_id() != employeeId)
    {
        throw invalid_argument(
            "Employee ID cannot be changed during editing!"
        );
    }

    // Khong vo hieu hoa hoac doi chuc vu nhan vien dang nam trong
    // Assignment da xac nhan, tranh lam sai co cau 1 Driver + 2 Janitors.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        const Assignment& assignment = assignmentManager[i];

        if (assignment.getAssignment_status() == "Assigned" &&
            assignment.hasEmployee(employeeId))
        {
            if (updatedEmployee.get_status() == "Inactive" &&
                currentEmployee->get_status() != "Inactive")
            {
                return false;
            }

            if (updatedEmployee.get_position() !=
                currentEmployee->get_position())
            {
                return false;
            }
        }
    }

    return employeeManager.editEmployee(
        employeeId,
        updatedEmployee
    );
}

bool EmployeeWorkSystem::deleteEmployee(
    const string& employeeId
)
{
    if (employeeManager.findEmployeeById(employeeId) == nullptr)
    {
        return false;
    }

    // Chi tu choi neu Attendance da phat sinh cham cong/lich su,
    // hoac la ban ghi mo khong con Assignment tuong ung.
    for (int i = 0; i < attendanceManager.getSize(); i++)
    {
        const Attendance& attendance = attendanceManager[i];

        if (attendance.getEmployee_id() != employeeId)
        {
            continue;
        }

        const Assignment* linkedAssignment =
            assignmentManager.findAssignmentById(
                attendance.getAssignment_id()
            );

        if (linkedAssignment == nullptr ||
            !linkedAssignment->hasEmployee(employeeId))
        {
            // Khong xoa neu du lieu Attendance dang lien ket sai.
            return false;
        }

        if (attendance.getAttendance_status() != "Pending" ||
            !attendance.getCheck_in().empty() ||
            !attendance.getCheck_out().empty())
        {
            return false;
        }
    }

    // Kiem tra truoc khi thay doi du lieu.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        const Assignment& assignment = assignmentManager[i];

        if (!assignment.hasEmployee(employeeId))
        {
            continue;
        }

        const string status = assignment.getAssignment_status();

        if (status == "Completed" ||
            status == "Cancelled" ||
            hasShiftStarted(
                assignment.getAssignment_date(),
                assignment.getShift_id()))
        {
            return false;
        }

        const Attendance* attendance = attendanceManager.findAttendance(
            employeeId,
            assignment.getAssignment_id()
        );

        if (attendance != nullptr &&
            (attendance->getAttendance_status() != "Pending" ||
             !attendance->getCheck_in().empty() ||
             !attendance->getCheck_out().empty()))
        {
            return false;
        }
    }

    // Go nhan vien khoi Assignment tuong lai; Attendance Pending di kem
    // duoc xoa. Assignment da Assigned phai quay ve Planned.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        Assignment& assignment = assignmentManager[i];

        if (!assignment.hasEmployee(employeeId))
        {
            continue;
        }

        if (assignment.getAssignment_status() == "Assigned")
        {
            assignment.setAssignment_status("Planned");
        }

        Attendance* attendance = attendanceManager.findAttendance(
            employeeId,
            assignment.getAssignment_id()
        );

        if (attendance != nullptr)
        {
            attendanceManager.deleteAttendance(
                attendance->getAttendance_id()
            );
        }

        assignment.removeEmployee(employeeId);
    }

    return employeeManager.deleteEmployee(employeeId);
}

// Xoa xe qua lop dieu phoi de bao toan lien ket Assignment.
bool EmployeeWorkSystem::deleteVehicle(const string& vehicleId)
{
    if (vehicleManager.findVehicleById(vehicleId) == nullptr)
    {
        return false;
    }

    // Khong xoa vat ly xe da duoc ghi nhan trong lich su.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        const Assignment& assignment = assignmentManager[i];

        if (assignment.getVehicle_id() != vehicleId)
        {
            continue;
        }

        const string status = assignment.getAssignment_status();

        if (status == "Completed" ||
            status == "Cancelled" ||
            hasShiftStarted(
                assignment.getAssignment_date(),
                assignment.getShift_id()))
        {
            return false;
        }
    }

    // Go xe khoi cac Assignment tuong lai. Neu da Assigned thi chuyen
    // ve Planned vi khong con du dieu kien xac nhan.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        Assignment& assignment = assignmentManager[i];

        if (assignment.getVehicle_id() != vehicleId)
        {
            continue;
        }

        if (assignment.getAssignment_status() == "Assigned")
        {
            assignment.setAssignment_status("Planned");
        }

        assignment.setVehicle_id("");
    }

    return vehicleManager.deleteVehicle(vehicleId);
}

Employee* EmployeeWorkSystem::findEmployeeById(
    const string& employeeId
)
{
    return employeeManager.findEmployeeById(employeeId);
}

const Employee* EmployeeWorkSystem::findEmployeeById(
    const string& employeeId
) const
{
    return employeeManager.findEmployeeById(employeeId);
}

// ============================================================
// TAO ASSIGNMENT
// ============================================================

void EmployeeWorkSystem::createAssignment(
    const Assignment& assignment
)
{
    if (assignment.getAssignment_id().empty())
    {
        throw invalid_argument("Assignment ID cannot be empty!");
    }

    // Kiem tra trung ID ngay dau vao, truoc khi validate cac truong khac.
    if (assignmentManager.findAssignmentById(
            assignment.getAssignment_id()) != nullptr)
    {
        throw invalid_argument(
            "Assignment ID already exists: " +
            assignment.getAssignment_id()
        );
    }

    if (assignment.getAssignment_date().empty())
    {
        throw invalid_argument("Assignment date cannot be empty!");
    }

    tm parsedDate = {};

    if (!parseDate(
        assignment.getAssignment_date(),
        parsedDate
    ))
    {
        throw invalid_argument(
            "Invalid date! Use YYYY-MM-DD."
        );
    }

    if (compareDates(
        assignment.getAssignment_date(),
        getCurrentDate()
    ) < 0)
    {
        throw invalid_argument(
            "Cannot create an assignment in the past!"
        );
    }

    int sh, sm, eh, em;

    if (!getShiftTime(
        assignment.getShift_id(),
        sh, sm, eh, em
    ))
    {
        throw invalid_argument("Invalid shift ID!");
    }

    // Khong tao moi Assignment neu ca trong ngay hom nay da bat dau.
    if (hasShiftStarted(
        assignment.getAssignment_date(),
        assignment.getShift_id()
    ))
    {
        throw invalid_argument(
            "Cannot create an assignment after its shift has started!"
        );
    }

    if (assignment.getAssignment_status() != "Planned")
    {
        throw invalid_argument(
            "A new assignment must have Planned status!"
        );
    }

    const vector<string>& employeeIds =
        assignment.getEmployee_ids();

    // Neu co nhan vien thi kiem tra tung nguoi.
    for (size_t i = 0; i < employeeIds.size(); i++)
    {
        validateEmployeeAvailability(
            employeeIds[i],
            assignment.getAssignment_date(),
            assignment.getShift_id()
        );
    }

    // Neu da gan xe thi kiem tra xe.
    if (!assignment.getVehicle_id().empty())
    {
        validateVehicleAvailability(
            assignment.getVehicle_id(),
            assignment.getAssignment_date(),
            assignment.getShift_id()
        );
    }

    // Backup de tranh Assignment duoc them nhung Attendance tao that bai.
    AssignmentManager oldAssignments(assignmentManager);
    AttendanceManager oldAttendances(attendanceManager);

    try
    {
        assignmentManager.addAssignment(assignment);

        for (size_t i = 0; i < employeeIds.size(); i++)
        {
            Attendance attendance(
                generateAttendanceId(attendanceManager),
                employeeIds[i],
                assignment.getAssignment_id(),
                assignment.getAssignment_date(),
                "",
                "",
                "Pending"
            );

            attendanceManager.addAttendance(attendance);
        }
    }
    catch (...)
    {
        assignmentManager = oldAssignments;
        attendanceManager = oldAttendances;
        throw;
    }

    syncVehicleStatuses();
}

// ============================================================
// SUA ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::editAssignment(
    const string& assignmentId,
    const Assignment& updatedAssignment
)
{
    Assignment* current =
        assignmentManager.findAssignmentById(assignmentId);

    if (current == nullptr)
    {
        return false;
    }

    // Edit lich dua tren trang thai Assignment va ca dich moi.
    // Khong dung canModifyAssignment() o day vi ham do kiem tra ca hien tai;
    // viec doi sang ca dich chua bat dau trong cung ngay van duoc phep.
    const string currentStatus = current->getAssignment_status();
    if (currentStatus != "Planned" && currentStatus != "Assigned")
    {
        return false;
    }

    if (updatedAssignment.getAssignment_id() != assignmentId)
    {
        throw invalid_argument(
            "Assignment ID cannot be changed!"
        );
    }

    if (!Assignment::isValidStatus(
            updatedAssignment.getAssignment_status()) ||
        updatedAssignment.getAssignment_status() !=
            current->getAssignment_status())
    {
        throw invalid_argument(
            "Use dedicated workflow functions to change Assignment status!"
        );
    }

    // Khong sua danh sach thanh vien bang ham edit tong quat.
    // Phai su dung addEmployeeToAssignment /
    // removeEmployeeFromAssignment.
    if (current->getEmployee_ids() !=
        updatedAssignment.getEmployee_ids())
    {
        throw invalid_argument(
            "Use member management functions to change the team!"
        );
    }

    tm parsedDate = {};

    if (!parseDate(
        updatedAssignment.getAssignment_date(),
        parsedDate
    ))
    {
        throw invalid_argument(
            "Invalid date! Use YYYY-MM-DD."
        );
    }

    int startHour, startMinute, endHour, endMinute;

    if (!getShiftTime(
        updatedAssignment.getShift_id(),
        startHour,
        startMinute,
        endHour,
        endMinute
    ))
    {
        throw invalid_argument("Invalid shift ID!");
    }

    if (compareDates(
        updatedAssignment.getAssignment_date(),
        getCurrentDate()
    ) < 0)
    {
        throw invalid_argument(
            "Assignment date cannot be in the past!"
        );
    }

    if (hasShiftStarted(
        updatedAssignment.getAssignment_date(),
        updatedAssignment.getShift_id()
    ))
    {
        throw invalid_argument(
            "Cannot move an Assignment to a shift that has already started!"
        );
    }

    for (size_t i = 0;
         i < updatedAssignment.getEmployee_ids().size();
         i++)
    {
        validateEmployeeAvailability(
            updatedAssignment.getEmployee_id(
                static_cast<int>(i)
            ),
            updatedAssignment.getAssignment_date(),
            updatedAssignment.getShift_id(),
            assignmentId
        );
    }

    if (!updatedAssignment.getVehicle_id().empty())
    {
        validateVehicleAvailability(
            updatedAssignment.getVehicle_id(),
            updatedAssignment.getAssignment_date(),
            updatedAssignment.getShift_id(),
            assignmentId
        );
    }

    const bool requiresReconfirmation =
        current->getAssignment_status() == "Assigned" &&
        (current->getAssignment_date() != updatedAssignment.getAssignment_date() ||
         current->getShift_id() != updatedAssignment.getShift_id() ||
         current->getVehicle_id() != updatedAssignment.getVehicle_id());

    // Backup ca hai manager de tranh cap nhat nua chung neu mot buoc
    // dong bo Attendance that bai.
    AssignmentManager oldAssignments(assignmentManager);
    AttendanceManager oldAttendances(attendanceManager);

    try
    {
        bool result = assignmentManager.editAssignment(
            assignmentId,
            updatedAssignment
        );

        if (!result)
        {
            return false;
        }

        // Lay lai con tro sau khi AssignmentManager cap nhat mang noi bo.
        current = assignmentManager.findAssignmentById(assignmentId);

        if (current == nullptr)
        {
            throw runtime_error(
                "Assignment disappeared after update."
            );
        }

        // Thay doi lich/xe cua Assignment da xac nhan thi phai xac nhan lai.
        if (requiresReconfirmation)
        {
            current->setAssignment_status("Planned");
        }

        // Dong bo lai ngay Attendance theo Assignment.
        for (int i = 0; i < attendanceManager.getSize(); i++)
        {
            if (attendanceManager[i].getAssignment_id() ==
                assignmentId)
            {
                attendanceManager[i].setAttendance_date(
                    updatedAssignment.getAssignment_date()
                );
            }
        }

        syncVehicleStatuses();
        return true;
    }
    catch (...)
    {
        assignmentManager = oldAssignments;
        attendanceManager = oldAttendances;
        syncVehicleStatuses();
        throw;
    }
}

// ============================================================
// XOA ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::deleteAssignment(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (assignment->getAssignment_status() != "Planned")
    {
        return false;
    }

    if (hasShiftStarted(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    ))
    {
        return false;
    }

    // Chi xoa Assignment neu tat ca Attendance lien quan
    // van Pending va chua co check-in/check-out.
    for (int i = 0; i < attendanceManager.getSize(); i++)
    {
        const Attendance& attendance = attendanceManager[i];

        if (attendance.getAssignment_id() == assignmentId)
        {
            if (attendance.getAttendance_status() != "Pending" ||
                !attendance.getCheck_in().empty() ||
                !attendance.getCheck_out().empty())
            {
                return false;
            }
        }
    }

    // Xoa Attendance truoc.
    int i = 0;

    while (i < attendanceManager.getSize())
    {
        if (attendanceManager[i].getAssignment_id() ==
            assignmentId)
        {
            string attendanceId =
                attendanceManager[i].getAttendance_id();

            attendanceManager.deleteAttendance(attendanceId);
        }
        else
        {
            i++;
        }
    }

    bool result = assignmentManager.deleteAssignment(assignmentId);

    syncVehicleStatuses();
    return result;
}

// ============================================================
// XAC NHAN ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::confirmAssignment(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (assignment->getAssignment_status() != "Planned")
    {
        return false;
    }

    if (hasShiftStarted(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    ))
    {
        return false;
    }

    validateTeamComposition(assignment->getEmployee_ids());

    if (assignment->getVehicle_id().empty())
    {
        throw invalid_argument(
            "Assignment must have a vehicle before confirmation!"
        );
    }

    validateVehicleAvailability(
        assignment->getVehicle_id(),
        assignment->getAssignment_date(),
        assignment->getShift_id(),
        assignmentId
    );

    for (size_t i = 0;
         i < assignment->getEmployee_ids().size();
         i++)
    {
        validateEmployeeAvailability(
            assignment->getEmployee_id(
                static_cast<int>(i)
            ),
            assignment->getAssignment_date(),
            assignment->getShift_id(),
            assignmentId
        );

        Attendance* attendance =
            attendanceManager.findAttendance(
                assignment->getEmployee_id(
                    static_cast<int>(i)
                ),
                assignmentId
            );

        if (attendance == nullptr ||
            attendance->getAttendance_status() != "Pending")
        {
            throw invalid_argument(
                "Attendance records are not synchronized!"
            );
        }
    }

    assignment->setAssignment_status("Assigned");

    syncVehicleStatuses();
    return true;
}

// ============================================================
// HUY ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::cancelAssignment(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    string status = assignment->getAssignment_status();

    if (status != "Planned" && status != "Assigned")
    {
        return false;
    }

    if (hasShiftStarted(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    ))
    {
        return false;
    }

    assignment->setAssignment_status("Cancelled");

    attendanceManager.cancelAttendanceForAssignment(
        assignmentId
    );

    syncVehicleStatuses();
    return true;
}

// ============================================================
// HOAN THANH ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::completeAssignment(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (assignment->getAssignment_status() != "Assigned")
    {
        return false;
    }

    if (!hasShiftEnded(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    ))
    {
        return false;
    }

    if (!finalizeAttendance(assignmentId))
    {
        return false;
    }

    syncVehicleStatuses();
    return true;
}

// ============================================================
// THEM THANH VIEN VAO ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::addEmployeeToAssignment(
    const string& assignmentId,
    const string& employeeId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (!canModifyAssignment(assignmentId))
    {
        return false;
    }

    if (assignment->hasEmployee(employeeId))
    {
        throw invalid_argument(
            "Employee is already a member of this Assignment: " +
            employeeId
        );
    }

    if (assignment->getEmployeeCount() >= 3)
    {
        throw invalid_argument(
            "A team cannot have more than 3 employees!"
        );
    }

    validateEmployeeAvailability(
        employeeId,
        assignment->getAssignment_date(),
        assignment->getShift_id(),
        assignmentId
    );

    // Kiem tra vai tro de khong vuot qua co cau doi.
    const Employee* employee =
        employeeManager.findEmployeeById(employeeId);

    int driverCount = 0;
    int janitorCount = 0;

    for (int i = 0; i < assignment->getEmployeeCount(); i++)
    {
        const Employee* current =
            employeeManager.findEmployeeById(
                assignment->getEmployee_id(i)
            );

        if (current == nullptr)
        {
            throw invalid_argument(
                "An existing team member does not exist!"
            );
        }

        if (current->get_position() == "Driver")
        {
            driverCount++;
        }
        else if (current->get_position() == "Janitor")
        {
            janitorCount++;
        }
    }

    if (employee->get_position() == "Driver")
    {
        if (driverCount >= 1)
        {
            throw invalid_argument(
                "An assignment can only have one Driver!"
            );
        }
    }
    else if (employee->get_position() == "Janitor")
    {
        if (janitorCount >= 2)
        {
            throw invalid_argument(
                "An assignment can only have two Janitors!"
            );
        }
    }
    else
    {
        throw invalid_argument(
            "Manager cannot join a collection team!"
        );
    }

    if (!assignment->addEmployee(employeeId))
    {
        return false;
    }

    try
    {
        Attendance attendance(
            generateAttendanceId(attendanceManager),
            employeeId,
            assignmentId,
            assignment->getAssignment_date(),
            "",
            "",
            "Pending"
        );

        attendanceManager.addAttendance(attendance);
    }
    catch (...)
    {
        assignment->removeEmployee(employeeId);
        throw;
    }

    // Thay doi doi hinh thi Assignment phai duoc xac nhan lai.
    if (assignment->getAssignment_status() == "Assigned")
    {
        assignment->setAssignment_status("Planned");
    }

    return true;
}

// ============================================================
// XOA THANH VIEN KHOI ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::removeEmployeeFromAssignment(
    const string& assignmentId,
    const string& employeeId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (!canModifyAssignment(assignmentId))
    {
        return false;
    }

    if (!assignment->hasEmployee(employeeId))
    {
        return false;
    }

    Attendance* attendance =
        attendanceManager.findAttendance(
            employeeId,
            assignmentId
        );

    // Chi duoc go thanh vien neu Attendance chua cham cong.
    if (attendance != nullptr)
    {
        if (attendance->getAttendance_status() != "Pending" ||
            !attendance->getCheck_in().empty() ||
            !attendance->getCheck_out().empty())
        {
            return false;
        }

        string attendanceId =
            attendance->getAttendance_id();

        if (!attendanceManager.deleteAttendance(attendanceId))
        {
            return false;
        }
    }

    const bool removed = assignment->removeEmployee(employeeId);
    if (removed && assignment->getAssignment_status() == "Assigned")
    {
        assignment->setAssignment_status("Planned");
    }
    return removed;
}

// ============================================================
// GAN PHUONG TIEN
// ============================================================

bool EmployeeWorkSystem::assignVehicle(
    const string& assignmentId,
    const string& vehicleId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (!canModifyAssignment(assignmentId))
    {
        return false;
    }

    validateVehicleAvailability(
        vehicleId,
        assignment->getAssignment_date(),
        assignment->getShift_id(),
        assignmentId
    );

    const bool vehicleChanged = assignment->getVehicle_id() != vehicleId;
    assignment->setVehicle_id(vehicleId);

    if (vehicleChanged && assignment->getAssignment_status() == "Assigned")
    {
        assignment->setAssignment_status("Planned");
    }

    syncVehicleStatuses();
    return true;
}

// ============================================================
// GO PHUONG TIEN KHOI ASSIGNMENT
// ============================================================

bool EmployeeWorkSystem::removeVehicleFromAssignment(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    if (!canModifyAssignment(assignmentId))
    {
        return false;
    }

    if (assignment->getVehicle_id().empty())
    {
        return false;
    }

    assignment->setVehicle_id("");
    if (assignment->getAssignment_status() == "Assigned")
    {
        assignment->setAssignment_status("Planned");
    }

    syncVehicleStatuses();
    return true;
}

// ============================================================
// ATTENDANCE - EMPLOYEE
// ============================================================

bool EmployeeWorkSystem::checkInByEmployee(
    const string& employeeId,
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr ||
        assignment->getAssignment_status() != "Assigned" ||
        !assignment->hasEmployee(employeeId))
    {
        return false;
    }

    const Employee* employee =
        employeeManager.findEmployeeById(employeeId);

    if (employee == nullptr || !employee->isActive())
    {
        return false;
    }

    const string today = getCurrentDate();
    if (assignment->getAssignment_date() != today)
    {
        return false;
    }

    Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );

    // Employee chi duoc Check-in mot lan cho moi Assignment.
    if (attendance == nullptr ||
        attendance->getAttendance_status() != "Pending" ||
        !attendance->getCheck_in().empty())
    {
        return false;
    }

    int startHour, startMinute, endHour, endMinute;
    if (!getShiftTime(
        assignment->getShift_id(),
        startHour,
        startMinute,
        endHour,
        endMinute
    ))
    {
        return false;
    }

    // Khong gioi han khung gio thao tac; chi phan loai dung gio/tre.
    const string currentTime = getCurrentTime();
    const int currentMinutes = convertTimeToMinutes(currentTime);
    const int shiftStartMinutes = startHour * 60 + startMinute;
    const string status =
        (currentMinutes > shiftStartMinutes) ? "Late" : "OnTime";

    attendanceManager.checkIn(
        employeeId,
        assignmentId,
        today,
        currentTime,
        status
    );

    return true;
}

bool EmployeeWorkSystem::checkOutByEmployee(
    const string& employeeId,
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr ||
        assignment->getAssignment_status() != "Assigned" ||
        !assignment->hasEmployee(employeeId))
    {
        return false;
    }

    const string today = getCurrentDate();
    if (assignment->getAssignment_date() != today)
    {
        return false;
    }

    Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );

    // Employee chi duoc Check-out mot lan va phai Check-in truoc.
    if (attendance == nullptr ||
        (attendance->getAttendance_status() != "OnTime" &&
         attendance->getAttendance_status() != "Late") ||
        attendance->getCheck_in().empty() ||
        !attendance->getCheck_out().empty())
    {
        return false;
    }

    const string currentTime = getCurrentTime();
    if (currentTime <= attendance->getCheck_in())
    {
        return false;
    }

    attendanceManager.checkOut(
        employeeId,
        assignmentId,
        today,
        currentTime
    );

    return true;
}

// ============================================================
// ATTENDANCE - MANAGER
// ============================================================

bool EmployeeWorkSystem::recordCheckInByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& time
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr ||
        (assignment->getAssignment_status() != "Assigned" &&
         assignment->getAssignment_status() != "Completed") ||
        !assignment->hasEmployee(employeeId))
    {
        return false;
    }

    if (!Attendance::isValidTime(time))
    {
        throw invalid_argument("Invalid time! Expected HH:MM.");
    }

    int startHour, startMinute, endHour, endMinute;
    if (!getShiftTime(
        assignment->getShift_id(),
        startHour,
        startMinute,
        endHour,
        endMinute
    ))
    {
        throw invalid_argument("Invalid Assignment shift!");
    }

    Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );
    if (attendance == nullptr)
    {
        throw invalid_argument("Attendance record not found!");
    }

    const int recordedMinutes = convertTimeToMinutes(time);
    const int shiftStartMinutes = startHour * 60 + startMinute;
    const string status =
        (recordedMinutes > shiftStartMinutes) ? "Late" : "OnTime";

    attendanceManager.setCheckInByManager(
        employeeId,
        assignmentId,
        assignment->getAssignment_date(),
        time,
        status
    );

    // Assignment da Completed ma Manager bo sung Check-in ma chua co
    // Check-out thi Attendance van phai the hien MissingCheckOut.
    if (assignment->getAssignment_status() == "Completed" &&
        attendance->getCheck_out().empty())
    {
        attendance->setAttendance_status("MissingCheckOut");
    }

    return true;
}

bool EmployeeWorkSystem::recordCheckOutByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& time
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr ||
        (assignment->getAssignment_status() != "Assigned" &&
         assignment->getAssignment_status() != "Completed") ||
        !assignment->hasEmployee(employeeId))
    {
        return false;
    }

    if (!Attendance::isValidTime(time))
    {
        throw invalid_argument("Invalid time! Expected HH:MM.");
    }

    Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );
    if (attendance == nullptr || attendance->getCheck_in().empty())
    {
        throw invalid_argument(
            "Cannot record Check-out before Check-in!"
        );
    }

    string status = attendance->getAttendance_status();
    if (status == "MissingCheckOut")
    {
        int startHour, startMinute, endHour, endMinute;
        if (!getShiftTime(
            assignment->getShift_id(),
            startHour,
            startMinute,
            endHour,
            endMinute
        ))
        {
            throw invalid_argument("Invalid Assignment shift!");
        }

        const int checkInMinutes =
            convertTimeToMinutes(attendance->getCheck_in());
        const int shiftStartMinutes = startHour * 60 + startMinute;
        status = (checkInMinutes > shiftStartMinutes) ? "Late" : "OnTime";
    }

    if (status != "OnTime" && status != "Late")
    {
        return false;
    }

    attendanceManager.setCheckOutByManager(
        employeeId,
        assignmentId,
        assignment->getAssignment_date(),
        time,
        status
    );

    return true;
}

bool EmployeeWorkSystem::correctCheckInByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& time
)
{
    const Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr || attendance->getCheck_in().empty())
    {
        return false;
    }

    return recordCheckInByManager(employeeId, assignmentId, time);
}

bool EmployeeWorkSystem::correctCheckOutByManager(
    const string& employeeId,
    const string& assignmentId,
    const string& time
)
{
    const Attendance* attendance = attendanceManager.findAttendance(
        employeeId,
        assignmentId
    );

    if (attendance == nullptr || attendance->getCheck_out().empty())
    {
        return false;
    }

    return recordCheckOutByManager(employeeId, assignmentId, time);
}

// ============================================================
// TUONG THICH VOI CODE CU
// ============================================================

bool EmployeeWorkSystem::checkIn(
    const string& employeeId,
    const string& assignmentId
)
{
    return checkInByEmployee(employeeId, assignmentId);
}

bool EmployeeWorkSystem::checkOut(
    const string& employeeId,
    const string& assignmentId
)
{
    return checkOutByEmployee(employeeId, assignmentId);
}

// ============================================================
// CHOT ATTENDANCE
// ============================================================

bool EmployeeWorkSystem::finalizeAttendance(
    const string& assignmentId
)
{
    Assignment* assignment =
        assignmentManager.findAssignmentById(assignmentId);

    if (assignment == nullptr)
    {
        return false;
    }

    // Chi chot mot Assignment dang Assigned va da qua gio ket thuc.
    if (assignment->getAssignment_status() != "Assigned")
    {
        return false;
    }

    if (!hasShiftEnded(
        assignment->getAssignment_date(),
        assignment->getShift_id()
    ))
    {
        return false;
    }

    // Buoc 1: kiem tra toan bo Attendance truoc khi thay doi bat ky du lieu nao.
    for (size_t i = 0;
         i < assignment->getEmployee_ids().size();
         i++)
    {
        const string& employeeId = assignment->getEmployee_ids()[i];
        const Attendance* attendance =
            attendanceManager.findAttendance(employeeId, assignmentId);

        if (attendance == nullptr ||
            attendance->getAttendance_date() !=
                assignment->getAssignment_date())
        {
            throw runtime_error(
                "Attendance record is missing or has an incorrect date!"
            );
        }

        const string status = attendance->getAttendance_status();
        const string checkIn = attendance->getCheck_in();
        const string checkOut = attendance->getCheck_out();

        if (status == "Pending")
        {
            if (!checkIn.empty() || !checkOut.empty())
            {
                throw runtime_error(
                    "Pending Attendance contains check-in/check-out data!"
                );
            }
        }
        else if (status == "OnTime" || status == "Late")
        {
            if (checkIn.empty())
            {
                throw runtime_error(
                    "Checked-in Attendance has no check-in time!"
                );
            }
        }
        else
        {
            throw runtime_error(
                "Attendance has an invalid status for completion!"
            );
        }

        if (!checkOut.empty() && checkIn.empty())
        {
            throw runtime_error(
                "Attendance has check-out without check-in!"
            );
        }

        if (!checkOut.empty() && checkOut <= checkIn)
        {
            throw runtime_error(
                "Attendance check-out must be later than check-in!"
            );
        }
    }

    // Buoc 2: du lieu da hop le, chot cac Attendance.
    for (size_t i = 0;
         i < assignment->getEmployee_ids().size();
         i++)
    {
        const string& employeeId = assignment->getEmployee_ids()[i];
        Attendance* attendance =
            attendanceManager.findAttendance(employeeId, assignmentId);

        if (attendance->getAttendance_status() == "Pending")
        {
            attendanceManager.markAbsent(employeeId, assignmentId);
        }
        else if ((attendance->getAttendance_status() == "OnTime" ||
                  attendance->getAttendance_status() == "Late") &&
                 attendance->getCheck_out().empty())
        {
            attendanceManager.markMissingCheckOut(employeeId, assignmentId);
        }
    }

    // Buoc 3: chi chuyen Completed sau khi Attendance da duoc chot.
    assignment->setAssignment_status("Completed");

    return true;
}

// ============================================================
// DONG BO TRANG THAI PHUONG TIEN
// ============================================================

void EmployeeWorkSystem::syncVehicleStatuses()
{
    vector<string> vehiclesInUse;

    // Tim cac xe dang phuc vu Assignment trong khung gio hien tai.
    for (int i = 0; i < assignmentManager.getSize(); i++)
    {
        const Assignment& assignment = assignmentManager[i];

        if (assignment.getAssignment_status() != "Assigned")
        {
            continue;
        }

        if (assignment.getVehicle_id().empty())
        {
            continue;
        }

        bool started = false;
        bool ended = false;

        // Bo qua ban ghi TXT co ngay/ca khong hop le de tranh crash
        // khi khoi tao he thong; khong tu y sua du lieu nguon.
        try
        {
            started = hasShiftStarted(
                assignment.getAssignment_date(),
                assignment.getShift_id()
            );

            ended = hasShiftEnded(
                assignment.getAssignment_date(),
                assignment.getShift_id()
            );
        }
        catch (const exception&)
        {
            continue;
        }

        if (started && !ended)
        {
            if (find(
                vehiclesInUse.begin(),
                vehiclesInUse.end(),
                assignment.getVehicle_id()
            ) == vehiclesInUse.end())
            {
                vehiclesInUse.push_back(
                    assignment.getVehicle_id()
                );
            }
        }
    }

    // Cap nhat trang thai xe.
    for (int i = 0; i < vehicleManager.getSize(); i++)
    {
        Vehicle& vehicle = vehicleManager[i];

        // Khong tu dong thay doi xe dang bao duong.
        if (vehicle.getVehicle_status() == "Maintenance")
        {
            continue;
        }

        bool inUse =
            find(
                vehiclesInUse.begin(),
                vehiclesInUse.end(),
                vehicle.getVehicle_id()
            ) != vehiclesInUse.end();

        if (inUse)
        {
            vehicle.setVehicle_status("InUse");
        }
        else if (vehicle.getVehicle_status() == "InUse")
        {
            vehicle.setVehicle_status("Available");
        }
    }
}

void EmployeeWorkSystem::updateVehicleStatus(
    const string& assignmentId
)
{
    if (assignmentManager.findAssignmentById(assignmentId) == nullptr)
    {
        return;
    }

    syncVehicleStatuses();
}

void EmployeeWorkSystem::releaseVehicle(
    const string& vehicleId
)
{
    Vehicle* vehicle =
        vehicleManager.findVehicleById(vehicleId);

    if (vehicle == nullptr)
    {
        return;
    }

    if (vehicle->getVehicle_status() == "InUse")
    {
        vehicle->setVehicle_status("Available");
    }
}

// ============================================================
// GETTERS
// ============================================================

EmployeeManager& EmployeeWorkSystem::getEmployeeManager()
{
    return employeeManager;
}

AssignmentManager& EmployeeWorkSystem::getAssignmentManager()
{
    return assignmentManager;
}

AttendanceManager& EmployeeWorkSystem::getAttendanceManager()
{
    return attendanceManager;
}

VehicleManager& EmployeeWorkSystem::getVehicleManager()
{
    return vehicleManager;
}
