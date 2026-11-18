#pragma once

#include <iostream>
#include <string>

using namespace std;

class Attendance
{
private:
    string attendance_id;
    string employee_id;
    string assignment_id;
    string attendance_date;
    string check_in;
    string check_out;
    string attendance_status;

public:
    // Constructor
    Attendance();

    // Giu constructor cu de han che anh huong code hien tai
    Attendance(
        const string& attendance_id,
        const string& employee_id,
        const string& assignment_id,
        const string& attendance_date,
        const string& check_in,
        const string& check_out
    );

    // Constructor co trang thai
    Attendance(
        const string& attendance_id,
        const string& employee_id,
        const string& assignment_id,
        const string& attendance_date,
        const string& check_in,
        const string& check_out,
        const string& attendance_status
    );

    // Getter / Setter
    string getAttendance_id() const;
    void setAttendance_id(const string& attendance_id);

    string getEmployee_id() const;
    void setEmployee_id(const string& employee_id);

    string getAssignment_id() const;
    void setAssignment_id(const string& assignment_id);

    string getAttendance_date() const;
    void setAttendance_date(const string& attendance_date);

    string getCheck_in() const;
    void setCheck_in(const string& check_in);

    string getCheck_out() const;
    void setCheck_out(const string& check_out);

    string getAttendance_status() const;
    void setAttendance_status(const string& attendance_status);

    static bool isValidStatus(const string& status);
    static bool isValidTime(const string& time);

    // Hien thi
    void Show() const;
};
