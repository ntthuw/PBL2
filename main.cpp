#include "raylib.h"

#include "EmployeeWorkSystem.h"
#include "Managers/AuthManager.h"
#include "Managers/CustomerManager.h"
#include "Managers/ExpenseManager.h"
#include "Managers/InvoiceManager.h"
#include "Policies/FeePolicy.h"
#include "Objects/Driver.h"
#include "Objects/Janitor.h"
#include "Objects/Manager.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

const int WINDOW_WIDTH = 1440;
const int WINDOW_HEIGHT = 900;
const int SIDEBAR_WIDTH = 238;

const Color BG = { 244, 247, 251, 255 };
const Color SURFACE = { 255, 255, 255, 255 };
const Color INK = { 30, 41, 59, 255 };
const Color MUTED = { 112, 126, 145, 255 };
const Color LINE = { 226, 232, 240, 255 };
const Color UI_BLUE = { 44, 105, 210, 255 };
const Color UI_BLUE_LIGHT = { 232, 241, 255, 255 };
const Color UI_GREEN = { 35, 145, 102, 255 };
const Color UI_AMBER = { 198, 130, 25, 255 };
const Color UI_RED = { 196, 70, 70, 255 };

enum Page {
    PAGE_DASHBOARD,
    PAGE_EMPLOYEES,
    PAGE_ASSIGNMENTS,
    PAGE_ATTENDANCE,
    PAGE_VEHICLES,
    PAGE_CUSTOMERS,
    PAGE_INVOICES,
    PAGE_EXPENSES,
    PAGE_ACCOUNTS,
    PAGE_REPORTS
};

struct AppData {
    EmployeeManager employees;
    AssignmentManager assignments;
    AttendanceManager attendance;
    VehicleManager vehicles;
    EmployeeWorkSystem work;
    CustomerManager customers;
    InvoiceManager invoices;
    ExpenseManager expenses;
    AuthManager auth;
    FeePolicy fees;

    AppData()
        : work(employees, assignments, attendance, vehicles),
          fees("data/fee.txt")
    {
        loadEmployees("data/employee.txt");
        loadVehicles("data/vehicle.txt");
        loadAssignments("data/assignment.txt");
        loadCustomers("data/customer.txt");
        invoices.loadFromFile("data/invoice.txt");
        expenses.loadFromFile("data/expenses.txt");
        auth.loadFromFile("data/demo_users.txt");
        checkInvoiceRelations();
    }

    vector<vector<string> > readRows(const string& path, size_t columns)
    {
        ifstream file(path.c_str());
        if (!file.is_open()) throw runtime_error("Khong mo duoc " + path);
        vector<vector<string> > rows;
        string line;
        while (getline(file, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (line.empty() || line[0] == '#') continue;
            vector<string> fields;
            string field;
            stringstream stream(line);
            while (getline(stream, field, '|')) fields.push_back(field);
            if (!line.empty() && line[line.size() - 1] == '|') fields.push_back("");
            if (!fields.empty() && (fields[0] == "EmployeeID" || fields[0] == "VehicleID" || fields[0] == "AssignmentID")) continue;
            if (fields.size() != columns) throw runtime_error(path + ": cot du lieu khong hop le");
            rows.push_back(fields);
        }
        return rows;
    }

    void loadEmployees(const string& path)
    {
        vector<vector<string> > rows = readRows(path, 5);
        for (size_t i = 0; i < rows.size(); ++i) {
            Employee* employee = nullptr;
            if (rows[i][3] == "Driver") employee = new Driver(rows[i][0], rows[i][1], rows[i][2]);
            else if (rows[i][3] == "Janitor") employee = new Janitor(rows[i][0], rows[i][1], rows[i][2]);
            else if (rows[i][3] == "Manager") employee = new Manager(rows[i][0], rows[i][1], rows[i][2]);
            else throw runtime_error("Chuc vu khong hop le: " + rows[i][3]);
            try {
                employee->set_status(rows[i][4]);
                work.addEmployee(*employee);
            } catch (...) {
                delete employee;
                throw;
            }
            delete employee;
        }
    }

    void loadVehicles(const string& path)
    {
        vector<vector<string> > rows = readRows(path, 4);
        for (size_t i = 0; i < rows.size(); ++i) {
            Vehicle vehicle(rows[i][0], rows[i][1], stod(rows[i][2]), stod(rows[i][3]));
            vehicles.addVehicle(vehicle);
        }
        work.syncVehicleStatuses();
    }

    string resolveDate(const string& value)
    {
        if (value.compare(0, 6, "TODAY+") != 0) return value;
        const int offset = stoi(value.substr(6));
        time_t now = time(nullptr);
        tm date = {};
#ifdef _WIN32
        localtime_s(&date, &now);
#else
        tm* local = localtime(&now);
        if (!local) throw runtime_error("Khong doc duoc ngay hien tai");
        date = *local;
#endif
        date.tm_hour = 12;
        date.tm_mday += offset;
        date.tm_isdst = -1;
        mktime(&date);
        ostringstream out;
        out << setfill('0') << setw(4) << date.tm_year + 1900 << '-'
            << setw(2) << date.tm_mon + 1 << '-' << setw(2) << date.tm_mday;
        return out.str();
    }

    void loadAssignments(const string& path)
    {
        vector<vector<string> > rows = readRows(path, 7);
        for (size_t i = 0; i < rows.size(); ++i) {
            Assignment assignment(rows[i][0], vector<string>(), "", rows[i][2], resolveDate(rows[i][1]), "Planned");
            work.createAssignment(assignment);
            for (int member = 4; member <= 6; ++member) {
                if (!work.addEmployeeToAssignment(rows[i][0], rows[i][member]))
                    throw runtime_error("Khong tao duoc nhom cho " + rows[i][0]);
            }
            if (!work.assignVehicle(rows[i][0], rows[i][3]) || !work.confirmAssignment(rows[i][0]))
                throw runtime_error("Khong xac nhan duoc " + rows[i][0]);
        }
        work.syncVehicleStatuses();
    }

    void loadCustomers(const string& path)
    {
        vector<vector<string> > rows = readRows(path, 6);
        for (size_t i = 0; i < rows.size(); ++i) {
            Customer customer(rows[i][0], rows[i][1], rows[i][2], rows[i][3], rows[i][4], rows[i][5]);
            customer.validate();
            (void)fees.getFee(customer.getCustomer_type(), customer.getService_area());
            customers.addCustomer(customer);
        }
    }

    void checkInvoiceRelations() const
    {
        for (int i = 0; i < invoices.getSize(); ++i)
            if (!customers.findCustomerById(invoices[i].getCustomer_id()))
                throw runtime_error("Hoa don co ma khach hang khong ton tai");
    }
};

struct UiState {
    Page page;
    int selectedRow;
    int scroll;
    string search;
    string message;
    float messageTimer;
    bool loginMode;
    string username;
    string password;
    int loginField;
    int formKind;
    int formField;
    bool formEdit;
    string formKey;
    vector<string> formValues;
    UiState() : page(PAGE_DASHBOARD), selectedRow(-1), scroll(0), messageTimer(0), loginMode(true), loginField(0), formKind(0), formField(0), formEdit(false) {}
};

enum FormKind {
    FORM_NONE,
    FORM_EMPLOYEE,
    FORM_VEHICLE,
    FORM_CUSTOMER,
    FORM_INVOICE,
    FORM_EXPENSE,
    FORM_ACCOUNT,
    FORM_ASSIGNMENT,
    FORM_ADD_MEMBER,
    FORM_ASSIGN_VEHICLE
};

string lowerText(string value)
{
    for (size_t i = 0; i < value.size(); ++i) value[i] = static_cast<char>(tolower(static_cast<unsigned char>(value[i])));
    return value;
}

string money(long long amount)
{
    string raw = to_string(amount);
    string result;
    for (size_t i = 0; i < raw.size(); ++i) {
        result += raw[i];
        if ((raw.size() - i - 1) % 3 == 0 && i + 1 < raw.size()) result += ',';
    }
    return result + " VND";
}

bool button(Rectangle rect, const string& label, bool primary = false)
{
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    DrawRectangleRounded(rect, 0.18f, 8, primary ? (hovered ? UI_BLUE : Color{52, 112, 218, 255}) : (hovered ? UI_BLUE_LIGHT : SURFACE));
    DrawRectangleRoundedLines(rect, 0.18f, 8, primary ? UI_BLUE : LINE);
    int width = MeasureText(label.c_str(), 15);
    DrawText(label.c_str(), static_cast<int>(rect.x + (rect.width - width) / 2), static_cast<int>(rect.y + (rect.height - 16) / 2), 15, primary ? WHITE : INK);
    return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void label(const string& text, int x, int y, int size = 16, Color color = INK)
{
    DrawText(text.c_str(), x, y, size, color);
}

void drawCard(Rectangle rect, const string& title, const string& value, const string& hint, Color accent)
{
    DrawRectangleRounded(rect, 0.08f, 10, SURFACE);
    DrawRectangleRoundedLines(rect, 0.08f, 10, LINE);
    DrawRectangle( static_cast<int>(rect.x + 18), static_cast<int>(rect.y + 19), 4, 42, accent);
    label(title, static_cast<int>(rect.x + 34), static_cast<int>(rect.y + 17), 14, MUTED);
    label(value, static_cast<int>(rect.x + 34), static_cast<int>(rect.y + 40), 24, INK);
    label(hint, static_cast<int>(rect.x + 34), static_cast<int>(rect.y + 76), 12, MUTED);
}

void drawSidebar(UiState& ui, const AuthManager& auth)
{
    DrawRectangle(0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT, Color{18, 32, 56, 255});
    DrawCircle(33, 34, 15, Color{88, 153, 255, 255});
    DrawText("P", 27, 23, 24, WHITE);
    label("PBL2  |  CLEAN CITY", 58, 24, 16, WHITE);
    label("OPERATIONS", 24, 83, 11, Color{147, 165, 190, 255});

    struct NavItem { Page page; const char* title; Permission permission; };
    const NavItem items[] = {
        { PAGE_DASHBOARD, "Tong quan", Permission::ViewDashboard },
        { PAGE_EMPLOYEES, "Nhan vien", Permission::ManageEmployees },
        { PAGE_ASSIGNMENTS, "Phan cong", Permission::ManageAssignments },
        { PAGE_ATTENDANCE, "Cham cong", Permission::ViewAllAttendance },
        { PAGE_VEHICLES, "Phuong tien", Permission::ManageVehicles },
        { PAGE_CUSTOMERS, "Khach hang", Permission::ManageCustomers },
        { PAGE_INVOICES, "Hoa don", Permission::ManageInvoices },
        { PAGE_EXPENSES, "Chi phi", Permission::ManageExpenses },
        { PAGE_ACCOUNTS, "Tai khoan", Permission::ManageAccounts },
        { PAGE_REPORTS, "Bao cao", Permission::ViewReports }
    };
    int y = 108;
    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); ++i) {
        bool allowed = auth.hasPermission(items[i].permission);
        if (!allowed) {
            if (items[i].page == PAGE_ATTENDANCE && auth.hasPermission(Permission::ViewOwnAttendance)) allowed = true;
            if (items[i].page == PAGE_ASSIGNMENTS && auth.hasPermission(Permission::ViewOwnWork)) allowed = true;
            if (items[i].page == PAGE_INVOICES && auth.hasPermission(Permission::ViewOwnInvoices)) allowed = true;
        }
        if (!allowed) continue;
        Rectangle rect = { 12, static_cast<float>(y), 214, 42 };
        bool active = ui.page == items[i].page;
        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, rect);
        if (active || hover) DrawRectangleRounded(rect, 0.14f, 8, active ? Color{42, 64, 96, 255} : Color{30, 48, 75, 255});
        DrawText(items[i].title, 28, y + 13, 15, active ? WHITE : Color{196, 208, 224, 255});
        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            ui.page = items[i].page;
            ui.selectedRow = -1;
            ui.scroll = 0;
            ui.search.clear();
        }
        y += 48;
    }

    const User* user = auth.getCurrentUser();
    DrawLine(18, WINDOW_HEIGHT - 72, SIDEBAR_WIDTH - 18, WINDOW_HEIGHT - 72, Color{57, 75, 102, 255});
    if (user) {
        label(user->getUsername(), 24, WINDOW_HEIGHT - 56, 15, WHITE);
        label(User::roleToString(user->getRole()), 24, WINDOW_HEIGHT - 34, 12, Color{147, 165, 190, 255});
    }
}

void drawHeader(UiState& ui)
{
    DrawRectangle(SIDEBAR_WIDTH, 0, WINDOW_WIDTH - SIDEBAR_WIDTH, 82, SURFACE);
    DrawLine(SIDEBAR_WIDTH, 81, WINDOW_WIDTH, 81, LINE);
    const char* titles[] = { "Tong quan", "Nhan vien", "Phan cong", "Cham cong", "Phuong tien", "Khach hang", "Hoa don", "Chi phi", "Tai khoan", "Bao cao" };
    label(titles[ui.page], SIDEBAR_WIDTH + 30, 19, 22, INK);
    label("He thong dieu hanh thu gom va dich vu", SIDEBAR_WIDTH + 30, 50, 13, MUTED);
    Rectangle search = { WINDOW_WIDTH - 300.0f, 20.0f, 264.0f, 38.0f };
    DrawRectangleRounded(search, 0.18f, 8, BG);
    label(ui.search.empty() ? "Tim theo ma / ten..." : ui.search, static_cast<int>(search.x + 12), static_cast<int>(search.y + 11), 13, ui.search.empty() ? MUTED : INK);
    if (CheckCollisionPointRec(GetMousePosition(), search) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 2;
}

void readSearch(UiState& ui)
{
    if (ui.loginField != 2) return;
    const string previous = ui.search;
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 && ui.search.size() < 48) ui.search.push_back(static_cast<char>(key));
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !ui.search.empty()) ui.search.erase(ui.search.size() - 1);
    if (IsKeyPressed(KEY_ESCAPE)) ui.loginField = 0;
    if (ui.search != previous) { ui.scroll = 0; ui.selectedRow = -1; }
}

template <typename RowFn>
void tableBase(UiState& ui, const string& subtitle, const vector<string>& headers, int total, RowFn rowFn)
{
    label(subtitle, SIDEBAR_WIDTH + 30, 108, 14, MUTED);
    Rectangle panel = { SIDEBAR_WIDTH + 30.0f, 140.0f, WINDOW_WIDTH - SIDEBAR_WIDTH - 60.0f, 590.0f };
    DrawRectangleRounded(panel, 0.025f, 6, SURFACE);
    DrawRectangleRoundedLines(panel, 0.025f, 6, LINE);
    float left = panel.x + 20;
    float top = panel.y + 18;
    float usableWidth = panel.width - 40;
    float colWidth = usableWidth / static_cast<float>(headers.size());
    for (size_t c = 0; c < headers.size(); ++c) label(headers[c], static_cast<int>(left + c * colWidth), static_cast<int>(top), 12, MUTED);
    DrawLine(static_cast<int>(left), static_cast<int>(top + 29), static_cast<int>(left + usableWidth), static_cast<int>(top + 29), LINE);
    int visible = 10;
    int maxScroll = max(0, total - visible);
    ui.scroll = max(0, min(ui.scroll, maxScroll));
    int shown = 0;
    int matchingIndex = 0;
    for (int i = 0; i < total; ++i) {
        vector<string> values = rowFn(i);
        string query = lowerText(ui.search);
        string combined;
        for (size_t j = 0; j < values.size(); ++j) combined += " " + values[j];
        if (!query.empty() && lowerText(combined).find(query) == string::npos) continue;
        if (matchingIndex++ < ui.scroll) continue;
        if (shown >= visible) break;
        float y = top + 39 + shown * 48.0f;
        Rectangle row = { left - 8, y - 8, usableWidth + 16, 44 };
        bool selected = ui.selectedRow == i;
        bool hover = CheckCollisionPointRec(GetMousePosition(), row);
        if (selected || hover) DrawRectangleRounded(row, 0.12f, 6, selected ? UI_BLUE_LIGHT : BG);
        for (size_t c = 0; c < headers.size() && c < values.size(); ++c) {
            string text = values[c];
            if (text.size() > 27) text = text.substr(0, 24) + "...";
            Color color = (c == 0) ? INK : MUTED;
            if (text == "Assigned" || text == "Paid" || text == "Available" || text == "Active" || text == "OnTime") color = UI_GREEN;
            if (text == "Planned" || text == "Unpaid" || text == "Maintenance" || text == "Late") color = UI_AMBER;
            if (text == "Cancelled" || text == "Absent" || text == "Inactive") color = UI_RED;
            label(text, static_cast<int>(left + c * colWidth), static_cast<int>(y + 5), 13, color);
        }
        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ui.selectedRow = i;
        ++shown;
    }
    if (total == 0) label("Chua co du lieu.", static_cast<int>(left + 4), static_cast<int>(top + 64), 14, MUTED);
    if (total > visible) {
        if (button({ panel.x + panel.width - 180, panel.y + panel.height - 48, 72, 30 }, "Truoc")) ui.scroll = max(0, ui.scroll - visible);
        if (button({ panel.x + panel.width - 98, panel.y + panel.height - 48, 72, 30 }, "Sau")) ui.scroll = min(maxScroll, ui.scroll + visible);
        label(to_string(min(total, ui.scroll + visible)) + " / " + to_string(total), static_cast<int>(panel.x + 20), static_cast<int>(panel.y + panel.height - 40), 12, MUTED);
    }
}

void drawDashboard(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    int x = SIDEBAR_WIDTH + 30;
    int gap = 18;
    int cardWidth = (WINDOW_WIDTH - SIDEBAR_WIDTH - 60 - 3 * gap) / 4;
    if (user->getRole() != UserRole::Manager) {
        int ownAssignments = 0, ownAttendance = 0, ownInvoices = 0;
        long long ownUnpaid = 0;
        for (int i = 0; i < app.assignments.getSize(); ++i)
            if (app.assignments[i].hasEmployee(user->getLinkedId())) ++ownAssignments;
        for (int i = 0; i < app.attendance.getSize(); ++i)
            if (app.attendance[i].getEmployee_id() == user->getLinkedId()) ++ownAttendance;
        for (int i = 0; i < app.invoices.getSize(); ++i) {
            const Invoice& invoice = app.invoices[i];
            if (invoice.getCustomer_id() == user->getLinkedId()) {
                ++ownInvoices;
                if (invoice.getInvoice_status() != "Paid") ownUnpaid += invoice.getTotal_amount();
            }
        }
        Rectangle panel = { (float)x, 242, (float)(WINDOW_WIDTH - x - 60), 390 };
        DrawRectangleRounded(panel, 0.025f, 8, SURFACE);
        DrawRectangleRoundedLines(panel, 0.025f, 8, LINE);
        if (user->getRole() == UserRole::Employee) {
            drawCard({ (float)x, 110, (float)cardWidth, 108 }, "Cong viec cua toi", to_string(ownAssignments), "Assignment duoc phan cong", UI_BLUE);
            drawCard({ (float)(x + cardWidth + gap), 110, (float)cardWidth, 108 }, "Cham cong cua toi", to_string(ownAttendance), "Ban ghi Attendance", UI_GREEN);
            label("Cong viec va cham cong", (int)panel.x + 24, (int)panel.y + 26, 18, INK);
            label("Mo muc Phan cong de xem lich lam viec cua ban.", (int)panel.x + 24, (int)panel.y + 86, 15, MUTED);
            label("Mo muc Cham cong de check-in / check-out trong ca lam.", (int)panel.x + 24, (int)panel.y + 124, 15, MUTED);
        } else {
            drawCard({ (float)x, 110, (float)cardWidth, 108 }, "Hoa don cua toi", to_string(ownInvoices), "Tong so hoa don", UI_BLUE);
            drawCard({ (float)(x + cardWidth + gap), 110, (float)cardWidth, 108 }, "Can thanh toan", money(ownUnpaid), "Cong no cua tai khoan", UI_AMBER);
            label("Tai khoan khach hang", (int)panel.x + 24, (int)panel.y + 26, 18, INK);
            label("Mo muc Hoa don de xem chi tiet va tinh trang thanh toan.", (int)panel.x + 24, (int)panel.y + 86, 15, MUTED);
        }
        return;
    }
    drawCard({ (float)x, 110, (float)cardWidth, 108 }, "Nhan vien", to_string(app.employees.getSize()), "Tong so ho so", UI_BLUE);
    drawCard({ (float)(x + cardWidth + gap), 110, (float)cardWidth, 108 }, "Assignment", to_string(app.assignments.getSize()), "Lich phan cong", UI_GREEN);
    drawCard({ (float)(x + 2 * (cardWidth + gap)), 110, (float)cardWidth, 108 }, "Hoa don chua thu", money(app.invoices.calculateTotalInvoiced() - app.invoices.calculateCollectedRevenue()), "Cong no hien tai", UI_AMBER);
    drawCard({ (float)(x + 3 * (cardWidth + gap)), 110, (float)cardWidth, 108 }, "Doanh thu da thu", money(app.invoices.calculateCollectedRevenue()), "Hoa don da thanh toan", UI_GREEN);

    Rectangle left = { (float)x, 242, 650, 450 };
    Rectangle right = { (float)(x + 670), 242, (float)(WINDOW_WIDTH - x - 700), 450 };
    DrawRectangleRounded(left, 0.025f, 8, SURFACE); DrawRectangleRoundedLines(left, 0.025f, 8, LINE);
    DrawRectangleRounded(right, 0.025f, 8, SURFACE); DrawRectangleRoundedLines(right, 0.025f, 8, LINE);
    label("Tinh hinh phan cong", (int)left.x + 22, (int)left.y + 20, 17, INK);
    int assigned = 0, planned = 0, completed = 0;
    for (int i = 0; i < app.assignments.getSize(); ++i) {
        const string status = app.assignments[i].getAssignment_status();
        if (status == "Assigned") ++assigned;
        else if (status == "Planned") ++planned;
        else if (status == "Completed") ++completed;
    }
    drawCard({ left.x + 20, left.y + 66, 185, 106 }, "Da xac nhan", to_string(assigned), "Assignment Assigned", UI_GREEN);
    drawCard({ left.x + 220, left.y + 66, 185, 106 }, "Dang cho", to_string(planned), "Assignment Planned", UI_AMBER);
    drawCard({ left.x + 420, left.y + 66, 205, 106 }, "Hoan tat", to_string(completed), "Assignment Completed", UI_BLUE);
    label("Chi phi phat sinh", (int)left.x + 22, (int)left.y + 205, 13, MUTED);
    label(money(app.expenses.calculateTotalExpenses()), (int)left.x + 22, (int)left.y + 231, 22, INK);
    label("Quy luong co ban", (int)left.x + 22, (int)left.y + 287, 13, MUTED);
    label(money(app.employees.calculateTotalSalaryFund()), (int)left.x + 22, (int)left.y + 313, 22, INK);
    label("Khach hang", (int)left.x + 22, (int)left.y + 369, 13, MUTED);
    label(to_string(app.customers.getSize()) + " ho so dang quan ly", (int)left.x + 22, (int)left.y + 393, 16, INK);

    label("Truy cap nhanh", (int)right.x + 22, (int)right.y + 20, 17, INK);
    const char* names[] = { "Nhan vien", "Phan cong", "Cham cong", "Phuong tien", "Khach hang", "Hoa don", "Chi phi", "Tai khoan", "Bao cao" };
    const Page pages[] = { PAGE_EMPLOYEES, PAGE_ASSIGNMENTS, PAGE_ATTENDANCE, PAGE_VEHICLES, PAGE_CUSTOMERS, PAGE_INVOICES, PAGE_EXPENSES, PAGE_ACCOUNTS, PAGE_REPORTS };
    const Permission permissions[] = { Permission::ManageEmployees, Permission::ManageAssignments, Permission::ViewAllAttendance, Permission::ManageVehicles, Permission::ManageCustomers, Permission::ManageInvoices, Permission::ManageExpenses, Permission::ManageAccounts, Permission::ViewReports };
    for (int i = 0; i < 9; ++i) {
        int col = i % 2, row = i / 2;
        Rectangle tile = { right.x + 20 + col * 165.0f, right.y + 65 + row * 78.0f, 150, 62 };
        bool allowed = app.auth.hasPermission(permissions[i]);
        if (!allowed && pages[i] == PAGE_ATTENDANCE) allowed = app.auth.hasPermission(Permission::ViewOwnAttendance);
        if (!allowed && pages[i] == PAGE_ASSIGNMENTS) allowed = app.auth.hasPermission(Permission::ViewOwnWork);
        if (!allowed && pages[i] == PAGE_INVOICES) allowed = app.auth.hasPermission(Permission::ViewOwnInvoices);
        if (!allowed) continue;
        if (button(tile, names[i])) ui.page = pages[i];
    }
    if (user) label("Xin chao, " + user->getUsername(), (int)right.x + 22, (int)right.y + 400, 13, MUTED);
}

void drawEmployees(AppData& app, UiState& ui)
{
    tableBase(ui, "Danh sach nhan vien", {"Ma NV", "Ho ten", "Chuc vu", "Trang thai", "Luong co ban"}, app.employees.getSize(), [&](int i) {
        const Employee* e = app.employees[i];
        return vector<string>{ e->get_id(), e->get_name(), e->get_position(), e->get_status(), money(e->get_salary()) };
    });
}

void drawAssignments(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    vector<int> indices;
    for (int i = 0; i < app.assignments.getSize(); ++i) {
        const Assignment& a = app.assignments[i];
        if (user->getRole() == UserRole::Manager || a.hasEmployee(user->getLinkedId())) indices.push_back(i);
    }
    tableBase(ui, "Lich lam viec va phan cong nhom", {"Ma", "Ngay", "Ca", "Xe", "So NV", "Trang thai"}, (int)indices.size(), [&](int i) {
        const Assignment& a = app.assignments[indices[i]];
        return vector<string>{a.getAssignment_id(), a.getAssignment_date(), a.getShift_id(), a.getVehicle_id().empty() ? "-" : a.getVehicle_id(), to_string(a.getEmployeeCount()), a.getAssignment_status()};
    });
    if (user->getRole() != UserRole::Manager) return;
}

void drawAttendance(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    vector<int> indices;
    for (int i = 0; i < app.attendance.getSize(); ++i)
        if (user->getRole() == UserRole::Manager || app.attendance[i].getEmployee_id() == user->getLinkedId()) indices.push_back(i);
    tableBase(ui, "Theo doi vao ca / tan ca", {"Ma NV", "Assignment", "Ngay", "Check-in", "Check-out", "Trang thai"}, (int)indices.size(), [&](int i) {
        const Attendance& a = app.attendance[indices[i]];
        return vector<string>{a.getEmployee_id(), a.getAssignment_id(), a.getAttendance_date(), a.getCheck_in().empty() ? "-" : a.getCheck_in(), a.getCheck_out().empty() ? "-" : a.getCheck_out(), a.getAttendance_status()};
    });
    float y = 748;
    if (user->getRole() == UserRole::Employee) {
        if (button({ SIDEBAR_WIDTH + 30.0f, y, 156, 38 }, "Check-in", true) && ui.selectedRow >= 0 && ui.selectedRow < (int)indices.size()) {
            const Attendance& a = app.attendance[indices[ui.selectedRow]];
            try { ui.message = app.work.checkInByEmployee(user->getLinkedId(), a.getAssignment_id()) ? "Check-in thanh cong" : "Khong the check-in"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
        if (button({ SIDEBAR_WIDTH + 198.0f, y, 156, 38 }, "Check-out") && ui.selectedRow >= 0 && ui.selectedRow < (int)indices.size()) {
            const Attendance& a = app.attendance[indices[ui.selectedRow]];
            try { ui.message = app.work.checkOutByEmployee(user->getLinkedId(), a.getAssignment_id()) ? "Check-out thanh cong" : "Khong the check-out"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
    }
}

void drawVehicles(AppData& app, UiState& ui)
{
    tableBase(ui, "Trang thai phuong tien", {"Ma xe", "Trang thai", "Tai trong", "So km"}, app.vehicles.getSize(), [&](int i) {
        const Vehicle& v = app.vehicles[i];
        return vector<string>{v.getVehicle_id(), v.getVehicle_status(), to_string(v.getVehicle_capacity()) + " tan", to_string((long long)v.getVehicle_km()) + " km"};
    });
}

void drawCustomers(AppData& app, UiState& ui)
{
    tableBase(ui, "Ho so khach hang", {"Ma KH", "Ten khach hang", "Loai", "SDT", "Khu vuc"}, app.customers.getSize(), [&](int i) {
        const Customer& c = app.customers[i];
        return vector<string>{c.getCustomer_id(), c.getCustomer_name(), c.getCustomer_type(), c.getCustomer_phone(), c.getService_area()};
    });
}

void drawInvoices(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    vector<int> indices;
    for (int i = 0; i < app.invoices.getSize(); ++i)
        if (user->getRole() == UserRole::Manager || app.invoices[i].getCustomer_id() == user->getLinkedId()) indices.push_back(i);
    tableBase(ui, "Hoa don va tinh trang thanh toan", {"Ma HD", "Ma KH", "Ky", "Ngay lap", "So tien", "Trang thai"}, (int)indices.size(), [&](int i) {
        const Invoice& inv = app.invoices[indices[i]];
        return vector<string>{inv.getInvoice_id(), inv.getCustomer_id(), inv.getBilling_month(), inv.getInvoice_date(), money(inv.getTotal_amount()), inv.getInvoice_status()};
    });
}

void drawExpenses(AppData& app, UiState& ui)
{
    tableBase(ui, "Cac khoan chi phi phat sinh", {"Ma CP", "Hang muc", "Ngay", "So tien", "Mo ta"}, app.expenses.getSize(), [&](int i) {
        const Expense& e = app.expenses[i];
        return vector<string>{e.getExpense_id(), e.getCategory(), e.getDate(), money(e.getAmount()), e.getDescription()};
    });
}

void drawAccounts(AppData& app, UiState& ui)
{
    vector<UserSummary> users = app.auth.listUsers();
    tableBase(ui, "Tai khoan va quyen truy cap", {"Ma user", "Username", "Vai tro", "Lien ket", "Trang thai"}, (int)users.size(), [&](int i) {
        const UserSummary& u = users[i];
        return vector<string>{u.user_id, u.username, User::roleToString(u.role), u.linked_id, u.active ? "Active" : "Inactive"};
    });
}

void drawReports(AppData& app)
{
    int x = SIDEBAR_WIDTH + 30;
    drawCard({ (float)x, 118, 350, 112 }, "Tong hoa don", money(app.invoices.calculateTotalInvoiced()), "Gia tri da phat hanh", UI_BLUE);
    drawCard({ (float)(x + 370), 118, 350, 112 }, "Da thu", money(app.invoices.calculateCollectedRevenue()), "Doanh thu da thanh toan", UI_GREEN);
    drawCard({ (float)(x + 740), 118, 350, 112 }, "Chi phi phat sinh", money(app.expenses.calculateTotalExpenses()), "Khong gom quy luong", UI_AMBER);
    Rectangle panel = { (float)x, 254, (float)(WINDOW_WIDTH - SIDEBAR_WIDTH - 60), 410 };
    DrawRectangleRounded(panel, 0.025f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.025f, 8, LINE);
    label("Tong hop van hanh", (int)panel.x + 24, (int)panel.y + 24, 18, INK);
    const long long net = app.invoices.calculateCollectedRevenue() - app.expenses.calculateTotalExpenses();
    label("So nhan vien", (int)panel.x + 28, (int)panel.y + 84, 14, MUTED); label(to_string(app.employees.getSize()), (int)panel.x + 300, (int)panel.y + 84, 16, INK);
    label("So Assignment", (int)panel.x + 28, (int)panel.y + 128, 14, MUTED); label(to_string(app.assignments.getSize()), (int)panel.x + 300, (int)panel.y + 128, 16, INK);
    label("So Attendance", (int)panel.x + 28, (int)panel.y + 172, 14, MUTED); label(to_string(app.attendance.getSize()), (int)panel.x + 300, (int)panel.y + 172, 16, INK);
    label("So khach hang", (int)panel.x + 28, (int)panel.y + 216, 14, MUTED); label(to_string(app.customers.getSize()), (int)panel.x + 300, (int)panel.y + 216, 16, INK);
    label("Quy luong co ban", (int)panel.x + 28, (int)panel.y + 260, 14, MUTED); label(money(app.employees.calculateTotalSalaryFund()), (int)panel.x + 300, (int)panel.y + 260, 16, INK);
    label("Thu da thu - chi phi", (int)panel.x + 28, (int)panel.y + 316, 14, MUTED); label(money(net), (int)panel.x + 300, (int)panel.y + 316, 18, net >= 0 ? UI_GREEN : UI_RED);
}

vector<string> formLabels(int kind)
{
    switch (kind) {
        case FORM_EMPLOYEE: return {"Ma nhan vien", "Ho ten", "Dia chi", "Chuc vu (Driver/Janitor/Manager)", "Trang thai (Active/Inactive)"};
        case FORM_VEHICLE: return {"Ma xe", "Trang thai (Available/Maintenance)", "Tai trong", "So km"};
        case FORM_CUSTOMER: return {"Ma khach hang", "Loai (Household/Business)", "Ten khach hang", "So dien thoai", "Dia chi", "Khu vuc (Area1/Area2/Type1/Type2)"};
        case FORM_INVOICE: return {"Ma hoa don", "Ma khach hang", "Ngay lap (YYYY-MM-DD)", "Ky hoa don (YYYY-MM)"};
        case FORM_EXPENSE: return {"Ma chi phi", "Hang muc", "Ngay (YYYY-MM-DD)", "So tien VND", "Mo ta"};
        case FORM_ACCOUNT: return {"Ma user", "Username", "Mat khau", "Role (Manager/Employee/Customer)", "Linked ID", "Trang thai (Active/Disabled)"};
        case FORM_ASSIGNMENT: return {"Ma Assignment", "Ngay (YYYY-MM-DD)", "Ca (CA01/CA02/CA03)"};
        case FORM_ADD_MEMBER: return {"Ma Assignment", "Ma nhan vien"};
        case FORM_ASSIGN_VEHICLE: return {"Ma Assignment", "Ma xe"};
        default: return vector<string>();
    }
}

void openForm(UiState& ui, int kind, const string& context = "")
{
    ui.formKind = kind;
    ui.formEdit = false;
    ui.formKey.clear();
    ui.formField = context.empty() ? 0 : 1;
    ui.formValues = vector<string>(formLabels(kind).size());
    if (!context.empty() && !ui.formValues.empty()) ui.formValues[0] = context;
}

void openEditForm(AppData& app, UiState& ui)
{
    if (ui.selectedRow < 0) { ui.message = "Hay chon mot dong truoc"; ui.messageTimer = 4; return; }
    ui.formEdit = true;
    ui.formField = 1;
    if (ui.page == PAGE_EMPLOYEES) {
        Employee* e = app.employees[ui.selectedRow];
        openForm(ui, FORM_EMPLOYEE);
        ui.formEdit = true; ui.formKey = e->get_id();
        ui.formValues = {e->get_id(), e->get_name(), e->get_address(), e->get_position(), e->get_status()};
    } else if (ui.page == PAGE_VEHICLES) {
        const Vehicle& v = app.vehicles[ui.selectedRow];
        openForm(ui, FORM_VEHICLE);
        ui.formEdit = true; ui.formKey = v.getVehicle_id();
        ui.formValues = {v.getVehicle_id(), v.getVehicle_status(), to_string(v.getVehicle_capacity()), to_string(v.getVehicle_km())};
    } else if (ui.page == PAGE_CUSTOMERS) {
        const Customer& c = app.customers[ui.selectedRow];
        if (app.invoices.hasInvoicesForCustomer(c.getCustomer_id())) {
            ui.message = "Khach hang co hoa don; khong cho sua thong tin tinh phi"; ui.messageTimer = 4; return;
        }
        openForm(ui, FORM_CUSTOMER);
        ui.formEdit = true; ui.formKey = c.getCustomer_id();
        ui.formValues = {c.getCustomer_id(), c.getCustomer_type(), c.getCustomer_name(), c.getCustomer_phone(), c.getCustomer_address(), c.getService_area()};
    } else if (ui.page == PAGE_EXPENSES) {
        const Expense& e = app.expenses[ui.selectedRow];
        openForm(ui, FORM_EXPENSE);
        ui.formEdit = true; ui.formKey = e.getExpense_id();
        ui.formValues = {e.getExpense_id(), e.getCategory(), e.getDate(), to_string(e.getAmount()), e.getDescription()};
    } else if (ui.page == PAGE_ASSIGNMENTS) {
        const Assignment& a = app.assignments[ui.selectedRow];
        openForm(ui, FORM_ASSIGNMENT);
        ui.formEdit = true; ui.formKey = a.getAssignment_id();
        ui.formValues = {a.getAssignment_id(), a.getAssignment_date(), a.getShift_id()};
    } else {
        ui.formEdit = false;
        ui.message = "Muc nay chua ho tro chinh sua"; ui.messageTimer = 4;
    }
}

void saveForm(AppData& app, UiState& ui)
{
    try {
        const vector<string>& f = ui.formValues;
        switch (ui.formKind) {
            case FORM_EMPLOYEE: {
                if (ui.formEdit && f[0] != ui.formKey) throw invalid_argument("Khong duoc doi ma nhan vien");
                Employee* item = nullptr;
                if (f[3] == "Driver") item = new Driver(f[0], f[1], f[2]);
                else if (f[3] == "Janitor") item = new Janitor(f[0], f[1], f[2]);
                else if (f[3] == "Manager") item = new Manager(f[0], f[1], f[2]);
                else throw invalid_argument("Chuc vu phai la Driver, Janitor hoac Manager");
                try {
                    item->set_status(f[4]);
                    if (ui.formEdit) { if (!app.work.editEmployee(ui.formKey, *item)) throw invalid_argument("Khong sua duoc nhan vien"); }
                    else app.work.addEmployee(*item);
                }
                catch (...) { delete item; throw; }
                delete item;
                break;
            }
            case FORM_VEHICLE: {
                Vehicle item(f[0], f[1], stod(f[2]), stod(f[3]));
                if (ui.formEdit) {
                    if (f[0] != ui.formKey || !app.vehicles.editVehicle(ui.formKey, item)) throw invalid_argument("Khong sua duoc phuong tien");
                } else app.vehicles.addVehicle(item);
                app.work.syncVehicleStatuses();
                break;
            }
            case FORM_CUSTOMER: {
                Customer item(f[0], f[1], f[2], f[3], f[4], f[5]);
                item.validate();
                (void)app.fees.getFee(item.getCustomer_type(), item.getService_area());
                if (ui.formEdit) {
                    if (f[0] != ui.formKey || !app.customers.editCustomer(ui.formKey, item)) throw invalid_argument("Khong sua duoc khach hang");
                } else app.customers.addCustomer(item);
                break;
            }
            case FORM_INVOICE: {
                const Customer* customer = app.customers.findCustomerById(f[1]);
                if (!customer) throw invalid_argument("Khong tim thay khach hang");
                long long amount = app.fees.getFee(customer->getCustomer_type(), customer->getService_area());
                app.invoices.addInvoice(Invoice(f[0], f[1], f[2], amount, f[3], "Unpaid"));
                break;
            }
            case FORM_EXPENSE: {
                Expense item(f[0], f[1], stoll(f[3]), f[2], f[4]);
                if (ui.formEdit) {
                    if (f[0] != ui.formKey || !app.expenses.editExpense(ui.formKey, item)) throw invalid_argument("Khong sua duoc chi phi");
                } else app.expenses.addExpense(item);
                break;
            }
            case FORM_ACCOUNT:
                if (f[5] != "Active" && f[5] != "Disabled") throw invalid_argument("Trang thai tai khoan phai la Active hoac Disabled");
                app.auth.addUser(User(f[0], f[1], f[2], User::roleFromString(f[3]), f[4], f[5] != "Disabled"));
                break;
            case FORM_ASSIGNMENT:
                if (ui.formEdit) {
                    if (f[0] != ui.formKey) throw invalid_argument("Khong duoc doi ma Assignment");
                    Assignment* current = app.assignments.findAssignmentById(ui.formKey);
                    if (!current) throw invalid_argument("Khong tim thay Assignment");
                    Assignment updated(ui.formKey, current->getEmployee_ids(), current->getVehicle_id(), f[2], f[1], current->getAssignment_status());
                    if (!app.work.editAssignment(ui.formKey, updated)) throw invalid_argument("Khong sua duoc Assignment");
                } else app.work.createAssignment(Assignment(f[0], vector<string>(), "", f[2], f[1], "Planned"));
                break;
            case FORM_ADD_MEMBER:
                if (!app.work.addEmployeeToAssignment(f[0], f[1])) throw invalid_argument("Khong them duoc nhan vien vao Assignment");
                break;
            case FORM_ASSIGN_VEHICLE:
                if (!app.work.assignVehicle(f[0], f[1])) throw invalid_argument("Khong gan duoc xe vao Assignment");
                break;
            default: throw logic_error("Form khong hop le");
        }
        ui.message = "Da cap nhat du lieu trong phien hien tai";
        ui.messageTimer = 4;
        ui.formKind = FORM_NONE;
        ui.formEdit = false;
        ui.formKey.clear();
        ui.formValues.clear();
    } catch (const exception& e) {
        ui.message = e.what();
        ui.messageTimer = 5;
    }
}

void readForm(UiState& ui, AppData& app)
{
    if (ui.formKind == FORM_NONE) return;
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 && ui.formField >= 0 && ui.formField < (int)ui.formValues.size() && ui.formValues[ui.formField].size() < 70)
            ui.formValues[ui.formField].push_back(static_cast<char>(key));
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && ui.formField >= 0 && ui.formField < (int)ui.formValues.size() && !ui.formValues[ui.formField].empty())
        ui.formValues[ui.formField].erase(ui.formValues[ui.formField].size() - 1);
    if (IsKeyPressed(KEY_TAB) && !ui.formValues.empty()) ui.formField = (ui.formField + 1) % (int)ui.formValues.size();
    if (IsKeyPressed(KEY_ESCAPE)) { ui.formKind = FORM_NONE; ui.formValues.clear(); }
    if (IsKeyPressed(KEY_ENTER)) saveForm(app, ui);
}

void drawForm(AppData& app, UiState& ui)
{
    if (ui.formKind == FORM_NONE) return;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{15, 25, 42, 125});
    vector<string> labels = formLabels(ui.formKind);
    float height = 150 + (float)labels.size() * 62;
    Rectangle panel = { 420, (WINDOW_HEIGHT - height) / 2, 600, height };
    DrawRectangleRounded(panel, 0.035f, 8, SURFACE);
    DrawRectangleRoundedLines(panel, 0.035f, 8, LINE);
    const char* titles[] = {"", "Them nhan vien", "Them phuong tien", "Them khach hang", "Lap hoa don", "Them chi phi", "Tao tai khoan", "Tao Assignment Planned", "Them thanh vien", "Gan phuong tien"};
    const string formTitle = ui.formEdit ? string("Chinh sua: ") + titles[ui.formKind] : titles[ui.formKind];
    label(formTitle, (int)panel.x + 26, (int)panel.y + 22, 20, INK);
    for (size_t i = 0; i < labels.size(); ++i) {
        float y = panel.y + 66 + i * 62;
        label(labels[i], (int)panel.x + 28, (int)y, 12, MUTED);
        Rectangle input = { panel.x + 26, y + 18, panel.width - 52, 34 };
        bool active = ui.formField == (int)i;
        DrawRectangleRounded(input, 0.14f, 6, active ? UI_BLUE_LIGHT : BG);
        DrawRectangleRoundedLines(input, 0.14f, 6, active ? UI_BLUE : LINE);
        string value = ui.formValues[i];
        if (ui.formKind == FORM_INVOICE && i == 1 && value.empty()) value = "Nhap ma khach hang truoc";
        label(value, (int)input.x + 10, (int)input.y + 9, 13, value.empty() ? MUTED : INK);
        if (CheckCollisionPointRec(GetMousePosition(), input) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.formField = (int)i;
    }
    if (button({panel.x + panel.width - 250, panel.y + panel.height - 54, 104, 34}, "Huy")) { ui.formKind = FORM_NONE; ui.formValues.clear(); }
    if (button({panel.x + panel.width - 132, panel.y + panel.height - 54, 106, 34}, "Luu", true)) saveForm(app, ui);
}

void deleteSelected(AppData& app, UiState& ui)
{
    bool removed = false;
    try {
        if (ui.selectedRow < 0) throw invalid_argument("Hay chon mot dong truoc");
        switch (ui.page) {
            case PAGE_EMPLOYEES: removed = app.work.deleteEmployee(app.employees[ui.selectedRow]->get_id()); break;
            case PAGE_VEHICLES: removed = app.work.deleteVehicle(app.vehicles[ui.selectedRow].getVehicle_id()); break;
            case PAGE_CUSTOMERS: {
                const string id = app.customers[ui.selectedRow].getCustomer_id();
                if (app.invoices.hasInvoicesForCustomer(id)) throw invalid_argument("Khach hang dang co hoa don, khong the xoa");
                removed = app.customers.deleteCustomer(id);
                break;
            }
            case PAGE_INVOICES: removed = app.invoices.deleteInvoice(app.invoices[ui.selectedRow].getInvoice_id()); break;
            case PAGE_EXPENSES: removed = app.expenses.deleteExpense(app.expenses[ui.selectedRow].getExpense_id()); break;
            case PAGE_ASSIGNMENTS: removed = app.work.deleteAssignment(app.assignments[ui.selectedRow].getAssignment_id()); break;
            default: throw invalid_argument("Muc nay khong ho tro xoa");
        }
        ui.message = removed ? "Da xoa ban ghi" : "Khong the xoa ban ghi";
    } catch (const exception& e) { ui.message = e.what(); }
    ui.messageTimer = 4;
    ui.selectedRow = -1;
}

string currentClockTime()
{
    time_t now = time(nullptr);
    tm local = {};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    tm* value = localtime(&now);
    if (!value) return "00:00";
    local = *value;
#endif
    ostringstream out;
    out << setfill('0') << setw(2) << local.tm_hour << ':' << setw(2) << local.tm_min;
    return out.str();
}

void drawActions(AppData& app, UiState& ui)
{
    if (!app.auth.hasPermission(Permission::ManageEmployees)) return;
    const float x = SIDEBAR_WIDTH + 30.0f;
    const float y = 748.0f;
    const float w = 136.0f;
    const float gap = 10.0f;
    if (ui.page == PAGE_EMPLOYEES || ui.page == PAGE_VEHICLES || ui.page == PAGE_CUSTOMERS || ui.page == PAGE_EXPENSES) {
        int kind = ui.page == PAGE_EMPLOYEES ? FORM_EMPLOYEE : ui.page == PAGE_VEHICLES ? FORM_VEHICLE : ui.page == PAGE_CUSTOMERS ? FORM_CUSTOMER : FORM_EXPENSE;
        if (button({x, y, w, 38}, "Them moi", true)) openForm(ui, kind);
        if (button({x + w + gap, y, w, 38}, "Sua dong chon")) openEditForm(app, ui);
        if (button({x + 2 * (w + gap), y, w, 38}, "Xoa dong chon")) deleteSelected(app, ui);
    } else if (ui.page == PAGE_ASSIGNMENTS) {
        const float aw = 126.0f, ag = 8.0f;
        if (button({x, y, aw, 38}, "Tao phan cong", true)) openForm(ui, FORM_ASSIGNMENT);
        string selectedId;
        if (ui.selectedRow >= 0 && ui.selectedRow < app.assignments.getSize()) selectedId = app.assignments[ui.selectedRow].getAssignment_id();
        if (button({x + (aw + ag), y, aw, 38}, "Sua lich") && !selectedId.empty()) openEditForm(app, ui);
        if (button({x + 2 * (aw + ag), y, aw, 38}, "Them nhan vien") && !selectedId.empty()) openForm(ui, FORM_ADD_MEMBER, selectedId);
        if (button({x + 3 * (aw + ag), y, aw, 38}, "Gan xe") && !selectedId.empty()) openForm(ui, FORM_ASSIGN_VEHICLE, selectedId);
        if (button({x + 4 * (aw + ag), y, aw, 38}, "Xac nhan") && !selectedId.empty()) {
            try { ui.message = app.work.confirmAssignment(selectedId) ? "Da xac nhan Assignment" : "Khong the xac nhan Assignment"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
        if (button({x + 5 * (aw + ag), y, aw, 38}, "Huy phan cong") && !selectedId.empty()) {
            ui.message = app.work.cancelAssignment(selectedId) ? "Da huy Assignment" : "Khong the huy Assignment";
            ui.messageTimer = 4;
        }
        if (button({x + 6 * (aw + ag), y, aw, 38}, "Hoan tat ca") && !selectedId.empty()) {
            try { ui.message = app.work.completeAssignment(selectedId) ? "Da chot cham cong" : "Chua the hoan tat"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
        if (button({x + 7 * (aw + ag), y, aw, 38}, "Xoa") && !selectedId.empty()) deleteSelected(app, ui);
    } else if (ui.page == PAGE_INVOICES) {
        if (button({x, y, w, 38}, "Lap hoa don", true)) openForm(ui, FORM_INVOICE);
        if (button({x + w + gap, y, w, 38}, "Danh dau da thu") && ui.selectedRow >= 0 && ui.selectedRow < app.invoices.getSize()) {
            try { ui.message = app.invoices.markAsPaid(app.invoices[ui.selectedRow].getInvoice_id()) ? "Da cap nhat thanh toan" : "Khong the cap nhat"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
        if (button({x + 2 * (w + gap), y, w, 38}, "Xoa hoa don")) deleteSelected(app, ui);
    } else if (ui.page == PAGE_ACCOUNTS) {
        if (button({x, y, w, 38}, "Them tai khoan", true)) openForm(ui, FORM_ACCOUNT);
        vector<UserSummary> users = app.auth.listUsers();
        if (button({x + w + gap, y, w + 35, 38}, "Vo hieu hoa") && ui.selectedRow >= 0 && ui.selectedRow < (int)users.size()) {
            try { ui.message = app.auth.disableUser(users[ui.selectedRow].user_id) ? "Da vo hieu hoa tai khoan" : "Khong the vo hieu hoa"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
    } else if (ui.page == PAGE_ATTENDANCE) {
        if (button({x, y, w + 35, 38}, "Ghi check-in", true) && ui.selectedRow >= 0 && ui.selectedRow < app.attendance.getSize()) {
            const Attendance& record = app.attendance[ui.selectedRow];
            try { ui.message = app.work.recordCheckInByManager(record.getEmployee_id(), record.getAssignment_id(), currentClockTime()) ? "Da ghi check-in" : "Khong the ghi check-in"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
        if (button({x + 155, y, w + 35, 38}, "Ghi check-out") && ui.selectedRow >= 0 && ui.selectedRow < app.attendance.getSize()) {
            const Attendance& record = app.attendance[ui.selectedRow];
            try { ui.message = app.work.recordCheckOutByManager(record.getEmployee_id(), record.getAssignment_id(), currentClockTime()) ? "Da ghi check-out" : "Khong the ghi check-out"; }
            catch (const exception& e) { ui.message = e.what(); }
            ui.messageTimer = 4;
        }
    }
}

void drawLogin(UiState& ui, AuthManager& auth)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{238, 243, 250, 255});
    Rectangle panel = { 460, 190, 520, 500 };
    DrawRectangleRounded(panel, 0.04f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.04f, 8, LINE);
    DrawCircle(720, 244, 24, UI_BLUE); DrawText("P", 710, 227, 34, WHITE);
    label("PBL2 Operations", 620, 286, 24, INK);
    label("Dang nhap de vao he thong", 625, 322, 14, MUTED);
    Rectangle userBox = { 530, 382, 380, 48 }, passBox = { 530, 452, 380, 48 };
    DrawRectangleRounded(userBox, 0.12f, 8, ui.loginField == 0 ? UI_BLUE_LIGHT : BG);
    DrawRectangleRoundedLines(userBox, 0.12f, 8, ui.loginField == 0 ? UI_BLUE : LINE);
    DrawRectangleRounded(passBox, 0.12f, 8, ui.loginField == 1 ? UI_BLUE_LIGHT : BG);
    DrawRectangleRoundedLines(passBox, 0.12f, 8, ui.loginField == 1 ? UI_BLUE : LINE);
    label(ui.username.empty() ? "Ten dang nhap" : ui.username, 548, 398, 15, ui.username.empty() ? MUTED : INK);
    string hidden(ui.password.size(), '*');
    label(hidden.empty() ? "Mat khau" : hidden, 548, 468, 15, hidden.empty() ? MUTED : INK);
    if (CheckCollisionPointRec(GetMousePosition(), userBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 0;
    if (CheckCollisionPointRec(GetMousePosition(), passBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 1;
    bool login = button({ 530, 530, 380, 46 }, "Dang nhap", true) || IsKeyPressed(KEY_ENTER);
    if (login) {
        if (auth.login(ui.username, ui.password)) {
            ui.loginMode = false; ui.page = PAGE_DASHBOARD; ui.message.clear(); ui.loginField = 0;
        } else { ui.message = "Sai tai khoan / mat khau hoac tai khoan bi khoa"; ui.messageTimer = 5; }
    }
    label("Tai khoan demo: admin  |  Mat khau: Admin@1234", 543, 600, 12, MUTED);
    if (ui.loginField == 0 || ui.loginField == 1) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && (ui.loginField == 0 ? ui.username.size() : ui.password.size()) < 48) {
                if (ui.loginField == 0) ui.username.push_back(static_cast<char>(key)); else ui.password.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            string& value = ui.loginField == 0 ? ui.username : ui.password;
            if (!value.empty()) value.erase(value.size() - 1);
        }
        if (IsKeyPressed(KEY_TAB)) ui.loginField = 1 - ui.loginField;
    }
}

} // namespace

int main()
{
    try {
        AppData app;
        UiState ui;
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PBL2 | Operations Management");
        SetTargetFPS(60);

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            if (ui.messageTimer > 0) ui.messageTimer -= dt;
            if (ui.loginMode) {
                BeginDrawing(); ClearBackground(BG); drawLogin(ui, app.auth);
                if (ui.messageTimer > 0) { DrawRectangle(0, WINDOW_HEIGHT - 52, WINDOW_WIDTH, 52, Color{255, 238, 238, 255}); label(ui.message, 30, WINDOW_HEIGHT - 35, 14, UI_RED); }
                EndDrawing(); continue;
            }
            if (ui.formKind != FORM_NONE) {
                readForm(ui, app);
                BeginDrawing(); ClearBackground(BG); drawForm(app, ui); EndDrawing();
                continue;
            }
            readSearch(ui);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.loginField != 2) {
                Rectangle searchRect = { WINDOW_WIDTH - 300.0f, 20.0f, 264.0f, 38.0f };
                if (!CheckCollisionPointRec(GetMousePosition(), searchRect)) ui.loginField = 0;
            }
            float wheel = GetMouseWheelMove();
            if (wheel != 0) ui.scroll = max(0, ui.scroll - (int)wheel);

            BeginDrawing();
            ClearBackground(BG);
            drawSidebar(ui, app.auth);
            drawHeader(ui);
            switch (ui.page) {
                case PAGE_DASHBOARD: drawDashboard(app, ui); break;
                case PAGE_EMPLOYEES: drawEmployees(app, ui); break;
                case PAGE_ASSIGNMENTS: drawAssignments(app, ui); break;
                case PAGE_ATTENDANCE: drawAttendance(app, ui); break;
                case PAGE_VEHICLES: drawVehicles(app, ui); break;
                case PAGE_CUSTOMERS: drawCustomers(app, ui); break;
                case PAGE_INVOICES: drawInvoices(app, ui); break;
                case PAGE_EXPENSES: drawExpenses(app, ui); break;
                case PAGE_ACCOUNTS: drawAccounts(app, ui); break;
                case PAGE_REPORTS: drawReports(app); break;
            }
            drawActions(app, ui);
            if (button({ WINDOW_WIDTH - 126.0f, WINDOW_HEIGHT - 56.0f, 100, 36 }, "Dang xuat")) {
                app.auth.logout(); ui.loginMode = true; ui.username.clear(); ui.password.clear(); ui.loginField = 0;
            }
            if (ui.messageTimer > 0 && !ui.message.empty()) {
                DrawRectangleRounded({ (float)SIDEBAR_WIDTH + 30, (float)WINDOW_HEIGHT - 58, (float)WINDOW_WIDTH - SIDEBAR_WIDTH - 180, 40 }, 0.16f, 6, Color{232, 247, 240, 255});
                label(ui.message, SIDEBAR_WIDTH + 46, WINDOW_HEIGHT - 45, 13, UI_GREEN);
            }
            EndDrawing();
        }
        CloseWindow();
    } catch (const exception& e) {
        cerr << "PBL2 GUI startup error: " << e.what() << endl;
        InitWindow(900, 300, "PBL2 - Loi khoi dong");
        SetTargetFPS(30);
        while (!WindowShouldClose()) {
            BeginDrawing(); ClearBackground(Color{250, 244, 244, 255});
            DrawText("Khong the khoi dong du lieu", 30, 30, 24, UI_RED);
            DrawText(e.what(), 30, 80, 16, INK);
            DrawText("Kiem tra du lieu trong thu muc data/", 30, 130, 16, MUTED);
            EndDrawing();
        }
        CloseWindow();
        return 1;
    }
    return 0;
}
