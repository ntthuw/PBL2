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

namespace UITheme {
const int WindowWidth = 1440;
const int WindowHeight = 900;
const int SidebarWidth = 250;
const int SpaceXS = 8;
const int SpaceSM = 12;
const int SpaceMD = 18;
const int SpaceLG = 24;
const float FontPageTitle = 28;
const float FontSectionTitle = 20;
const float FontSidebar = 18;
const float FontBody = 18;
const float FontTable = 17;
const float FontButton = 17;
const float ButtonHeight = 42;
const float FontCaption = 16;
const float FontMetric = 30;
const Color Background = { 248, 250, 249, 255 };
const Color Surface = { 255, 255, 255, 255 };
const Color Text = { 23, 43, 36, 255 };
const Color Muted = { 71, 85, 105, 255 };
const Color Border = { 221, 229, 225, 255 };
const Color Green = { 22, 132, 71, 255 };
const Color GreenPale = { 232, 245, 236, 255 };
const Color Sidebar = { 7, 84, 57, 255 };
const Color SidebarHover = { 17, 101, 70, 255 };
const Color SidebarActive = { 22, 132, 71, 255 };
const Color Info = { 44, 105, 210, 255 };
const Color Warning = { 198, 130, 25, 255 };
const Color Error = { 220, 53, 69, 255 };
}
using namespace UITheme;

const int WINDOW_WIDTH = UITheme::WindowWidth;
const int WINDOW_HEIGHT = UITheme::WindowHeight;
const int SIDEBAR_WIDTH = UITheme::SidebarWidth;
const Color BG = UITheme::Background;
const Color SURFACE = UITheme::Surface;
const Color INK = UITheme::Text;
const Color MUTED = UITheme::Muted;
const Color LINE = UITheme::Border;
const Color UI_BLUE = UITheme::Info;
const Color UI_BLUE_LIGHT = UITheme::GreenPale;
const Color UI_GREEN = UITheme::Green;
const Color UI_AMBER = UITheme::Warning;
const Color UI_RED = UITheme::Error;
const Color NAV_BG = UITheme::Sidebar;
const Color NAV_HOVER = UITheme::SidebarHover;
const Color NAV_ACTIVE = UITheme::SidebarActive;
const Color GREEN_PALE = UITheme::GreenPale;

enum FontWeight { FONT_REGULAR, FONT_MEDIUM, FONT_SEMIBOLD, FONT_BOLD, FONT_AUTO };
Font UI_FONTS[4] = {};
bool UI_FONT_LOADED[4] = { false, false, false, false };
Camera2D UI_CAMERA = {};

enum FeedbackKind { FEEDBACK_SUCCESS, FEEDBACK_ERROR, FEEDBACK_WARNING, FEEDBACK_INFO };
Color feedbackColor(FeedbackKind kind)
{
    switch (kind) {
        case FEEDBACK_SUCCESS: return UI_GREEN;
        case FEEDBACK_ERROR: return UI_RED;
        case FEEDBACK_WARNING: return UI_AMBER;
        default: return UI_BLUE;
    }
}

void setMessage(struct UiState& ui, const string& text, FeedbackKind kind, float seconds = 4.5f);
struct AppData;
void exportReport(AppData& app, struct UiState& ui);

Vector2 logicalMouse()
{
    Vector2 p = GetMousePosition();
    const float zoom = UI_CAMERA.zoom > 0 ? UI_CAMERA.zoom : 1.0f;
    return Vector2{ (p.x - UI_CAMERA.offset.x) / zoom, (p.y - UI_CAMERA.offset.y) / zoom };
}

FontWeight resolvedWeight(float size, FontWeight weight)
{
    if (weight != FONT_AUTO) return weight;
    if (size >= UITheme::FontPageTitle) return FONT_BOLD;
    if (size >= UITheme::FontSectionTitle) return FONT_SEMIBOLD;
    return FONT_REGULAR;
}

Font uiFont(float size, FontWeight weight)
{
    const FontWeight actual = resolvedWeight(size, weight);
    const int index = static_cast<int>(actual);
    if (index >= 0 && index < 4 && UI_FONT_LOADED[index]) return UI_FONTS[index];
    if (UI_FONT_LOADED[FONT_REGULAR]) return UI_FONTS[FONT_REGULAR];
    return GetFontDefault();
}

void drawText(const string& text, float x, float y, float size, Color color, FontWeight weight = FONT_AUTO)
{
    DrawTextEx(uiFont(size, weight), text.c_str(), Vector2{ x, y }, size, 0.0f, color);
}

float textWidth(const string& text, float size, FontWeight weight = FONT_AUTO)
{
    return MeasureTextEx(uiFont(size, weight), text.c_str(), size, 0.0f).x;
}

string fitText(const string& text, float width, float size, FontWeight weight = FONT_AUTO)
{
    if (textWidth(text, size, weight) <= width) return text;
    string shortened = text;
    while (!shortened.empty() && textWidth(shortened + "...", size, weight) > width) {
        size_t at = shortened.size() - 1;
        while (at > 0 && (static_cast<unsigned char>(shortened[at]) & 0xC0) == 0x80) --at;
        shortened.erase(at);
    }
    return shortened + "...";
}

void appendCodepoint(string& target, int codepoint, size_t maxBytes)
{
    if (codepoint < 32 || codepoint == 127) return;
    int length = 0;
    const char* encoded = CodepointToUTF8(codepoint, &length);
    if (length > 0 && target.size() + static_cast<size_t>(length) <= maxBytes)
        target.append(encoded, static_cast<size_t>(length));
}

void loadUIFont()
{
    vector<int> points;
    for (int c = 32; c <= 126; ++c) points.push_back(c);
    for (int c = 160; c <= 255; ++c) points.push_back(c);
    for (int c = 256; c <= 591; ++c) points.push_back(c);
    for (int c = 0x1E00; c <= 0x1EFF; ++c) points.push_back(c);
    for (int c = 768; c <= 879; ++c) points.push_back(c);
    for (int c = 0x2000; c <= 0x206F; ++c) points.push_back(c);
    for (int c = 0x2190; c <= 0x2192; ++c) points.push_back(c);
    const char* files[] = {
        "assets/fonts/BeVietnamPro-Regular.ttf",
        "assets/fonts/BeVietnamPro-Medium.ttf",
        "assets/fonts/BeVietnamPro-SemiBold.ttf",
        "assets/fonts/BeVietnamPro-Bold.ttf"
    };
    for (int i = 0; i < 4; ++i) {
        if (!FileExists(files[i])) continue;
        UI_FONTS[i] = LoadFontEx(files[i], 48, points.data(), static_cast<int>(points.size()));
        UI_FONT_LOADED[i] = UI_FONTS[i].texture.id != 0;
    }
}

void unloadUIFont()
{
    for (int i = 0; i < 4; ++i) if (UI_FONT_LOADED[i]) UnloadFont(UI_FONTS[i]);
}

void updateUICamera()
{
    const float scaleX = static_cast<float>(GetScreenWidth()) / WINDOW_WIDTH;
    const float scaleY = static_cast<float>(GetScreenHeight()) / WINDOW_HEIGHT;
    UI_CAMERA.target = Vector2{ 0, 0 };
    UI_CAMERA.rotation = 0;
    UI_CAMERA.zoom = min(scaleX, scaleY);
    UI_CAMERA.offset = Vector2{
        (GetScreenWidth() - WINDOW_WIDTH * UI_CAMERA.zoom) * 0.5f,
        (GetScreenHeight() - WINDOW_HEIGHT * UI_CAMERA.zoom) * 0.5f
    };
}

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
    PAGE_REPORTS,
    PAGE_PROFIT
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
        if (!file.is_open()) throw runtime_error("Không mở được " + path);
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
            if (fields.size() != columns) throw runtime_error(path + ": cột dữ liệu không hợp lệ");
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
            else throw runtime_error("Chức vụ không hợp lệ: " + rows[i][3]);
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
        if (!local) throw runtime_error("Không đọc được ngày hiện tại");
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
                    throw runtime_error("Không tạo được nhóm cho " + rows[i][0]);
            }
            if (!work.assignVehicle(rows[i][0], rows[i][3]) || !work.confirmAssignment(rows[i][0]))
                throw runtime_error("Không xác nhận được " + rows[i][0]);
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
                throw runtime_error("Hóa đơn có mã khách hàng không tồn tại");
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
    int loginStep;
    int selectedRole;
    string username;
    string password;
    int loginField;
    int formKind;
    int formField;
    bool formEdit;
    string formKey;
    vector<string> formValues;
    FeedbackKind messageKind;
    vector<bool> groupOpen;
    bool showPassword;
    bool confirmDelete;
    int formErrorField;
    UiState() : page(PAGE_DASHBOARD), selectedRow(-1), scroll(0), messageTimer(0), loginMode(true), loginStep(0), selectedRole(-1), loginField(0), formKind(0), formField(0), formEdit(false), messageKind(FEEDBACK_INFO), groupOpen(5, true), showPassword(false), confirmDelete(false), formErrorField(-1) {}
};

void setMessage(UiState& ui, const string& text, FeedbackKind kind, float seconds)
{
    ui.message = text;
    ui.messageKind = kind;
    ui.messageTimer = seconds;
}

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
    FORM_ASSIGN_VEHICLE,
    FORM_MANAGER_CHECKIN,
    FORM_MANAGER_CHECKOUT
};

string lowerText(string value)
{
    int count = 0;
    int* points = LoadCodepoints(value.c_str(), &count);
    string lowered;
    for (int i = 0; i < count; ++i) {
        int c = points[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        else if ((c >= 0x00C0 && c <= 0x00D6) || (c >= 0x00D8 && c <= 0x00DE)) c += 32;
        else if ((c >= 0x0100 && c <= 0x012F && c % 2 == 0) ||
                 (c >= 0x0132 && c <= 0x0137 && c % 2 == 0) ||
                 (c >= 0x014A && c <= 0x0177 && c % 2 == 0) ||
                 (c >= 0x1E00 && c <= 0x1EFF && c % 2 == 0)) c += 1;
        else if (c == 0x0178) c = 0x00FF;
        else if (c == 0x0110) c = 0x0111;
        int length = 0;
        const char* encoded = CodepointToUTF8(c, &length);
        if (length > 0) lowered.append(encoded, static_cast<size_t>(length));
    }
    if (points) MemFree(points);
    return lowered;
}

Color statusColor(const string& text)
{
    if (text == "Paid" || text == "Available" || text == "Active" || text == "OnTime" || text == "Completed") return UI_GREEN;
    if (text == "Assigned" || text == "InUse") return UI_BLUE;
    if (text == "Planned" || text == "Unpaid" || text == "Maintenance" || text == "Late" || text == "Pending") return UI_AMBER;
    if (text == "Cancelled" || text == "Absent" || text == "MissingCheckOut") return UI_RED;
    if (text == "Inactive" || text == "Disabled") return MUTED;
    return INK;
}

bool isKnownStatus(const string& text)
{
    return text == "Paid" || text == "Unpaid" || text == "Available" || text == "InUse" ||
           text == "Maintenance" || text == "Active" || text == "Inactive" || text == "Disabled" ||
           text == "OnTime" || text == "Late" || text == "Absent" || text == "MissingCheckOut" ||
           text == "Pending" || text == "Assigned" || text == "Planned" || text == "Completed" || text == "Cancelled";
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

bool button(Rectangle rect, const string& label, bool primary = false, bool danger = false)
{
    Vector2 mouse = logicalMouse();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    const Color fill = danger ? (hovered ? Color{183, 40, 53, 255} : UI_RED) : primary ? (hovered ? Color{17, 110, 58, 255} : UI_GREEN) : (hovered ? GREEN_PALE : SURFACE);
    DrawRectangleRounded(rect, 0.18f, 8, fill);
    DrawRectangleRoundedLines(rect, 0.18f, 8, danger ? UI_RED : primary ? UI_GREEN : LINE);
    const float fontSize = UITheme::FontButton;
    const string fitted = fitText(label, rect.width - 2 * UITheme::SpaceSM, fontSize, FONT_MEDIUM);
    const float width = textWidth(fitted, fontSize, FONT_MEDIUM);
    drawText(fitted, rect.x + (rect.width - width) / 2, rect.y + (rect.height - fontSize) / 2, fontSize, (primary || danger) ? WHITE : INK, FONT_MEDIUM);
    return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void disabledButton(Rectangle rect, const string& text)
{
    DrawRectangleRounded(rect, 0.18f, 8, Color{238, 241, 239, 255});
    DrawRectangleRoundedLines(rect, 0.18f, 8, LINE);
    const float size = UITheme::FontButton;
    const string fitted = fitText(text, rect.width - 16, size, FONT_MEDIUM);
    const float width = textWidth(fitted, size, FONT_MEDIUM);
    drawText(fitted, rect.x + (rect.width - width) / 2, rect.y + (rect.height - size) / 2, size, MUTED, FONT_MEDIUM);
}

void label(const string& text, int x, int y, int size = 16, Color color = INK, FontWeight weight = FONT_AUTO)
{
    drawText(text, static_cast<float>(x), static_cast<float>(y), static_cast<float>(size), color, weight);
}

void drawCard(Rectangle rect, const string& title, const string& value, const string& hint, Color accent)
{
    DrawRectangleRounded(rect, 0.08f, 10, SURFACE);
    DrawRectangleRoundedLines(rect, 0.08f, 10, LINE);
    label(fitText(title, rect.width - 2 * UITheme::SpaceMD, 17, FONT_SEMIBOLD), static_cast<int>(rect.x + UITheme::SpaceMD), static_cast<int>(rect.y + 7), 17, MUTED, FONT_SEMIBOLD);
    label(fitText(value, rect.width - 2 * UITheme::SpaceMD, UITheme::FontMetric, FONT_BOLD), static_cast<int>(rect.x + UITheme::SpaceMD), static_cast<int>(rect.y + 29), (int)UITheme::FontMetric, accent, FONT_BOLD);
    label(fitText(hint, rect.width - 2 * UITheme::SpaceMD, 15), static_cast<int>(rect.x + UITheme::SpaceMD), static_cast<int>(rect.y + 68), 15, MUTED, FONT_REGULAR);
}

void drawSidebar(UiState& ui, AppData& app)
{
    const AuthManager& auth = app.auth;
    DrawRectangle(0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT, NAV_BG);
    label("PBL2", 24, 20, 23, WHITE, FONT_BOLD);
    label("HỆ THỐNG THU GOM", 24, 52, 15, WHITE, FONT_MEDIUM);
    label("ĐIỀU HÀNH", 24, 83, 15, WHITE, FONT_SEMIBOLD);

    struct Item { Page page; const char* name; Permission permission; bool exportAction; };
    struct Group { const char* name; vector<Item> children; };
    vector<Group> groups;
    Group people = {"Nhân sự và lịch làm việc", {{PAGE_EMPLOYEES,"Nhân viên",Permission::ManageEmployees,false},{PAGE_ASSIGNMENTS,"Phân công",Permission::ManageAssignments,false},{PAGE_ATTENDANCE,"Chấm công",Permission::ViewAllAttendance,false}}};
    Group vehicles = {"Phương tiện", {{PAGE_VEHICLES,"Quản lý phương tiện",Permission::ManageVehicles,false}}};
    Group billing = {"Khách hàng và hóa đơn", {{PAGE_CUSTOMERS,"Khách hàng",Permission::ManageCustomers,false},{PAGE_INVOICES,"Hóa đơn",Permission::ManageInvoices,false}}};
    Group finance = {"Báo cáo - thu chi", {{PAGE_REPORTS,"Doanh thu",Permission::ViewReports,false},{PAGE_EXPENSES,"Chi phí",Permission::ManageExpenses,false},{PAGE_PROFIT,"Lợi nhuận",Permission::ViewReports,false},{PAGE_REPORTS,"Xuất báo cáo",Permission::ViewReports,true}}};
    Group accounts = {"Tài khoản", {{PAGE_ACCOUNTS,"Quản lý tài khoản",Permission::ManageAccounts,false}}};
    groups.push_back(people); groups.push_back(vehicles); groups.push_back(billing); groups.push_back(finance); groups.push_back(accounts);

    const Vector2 mouse = logicalMouse();
    int y = 102;
    Rectangle home = { 12, (float)y, 226, 38 };
    bool homeHover = CheckCollisionPointRec(mouse, home);
    if (ui.page == PAGE_DASHBOARD || homeHover) DrawRectangleRounded(home, 0.12f, 7, ui.page == PAGE_DASHBOARD ? NAV_ACTIVE : NAV_HOVER);
    label("Tổng quan", 29, y + 9, 18, WHITE, FONT_MEDIUM);
    if (homeHover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { ui.page = PAGE_DASHBOARD; ui.search.clear(); ui.selectedRow = -1; }
    y += 45;

    for (size_t g = 0; g < groups.size(); ++g) {
        bool groupAllowed = false;
        for (size_t i = 0; i < groups[g].children.size(); ++i) {
            Permission p = groups[g].children[i].permission;
            bool allowed = auth.hasPermission(p);
            if (groups[g].children[i].page == PAGE_ATTENDANCE) allowed = allowed || auth.hasPermission(Permission::ViewOwnAttendance);
            if (groups[g].children[i].page == PAGE_ASSIGNMENTS) allowed = allowed || auth.hasPermission(Permission::ViewOwnWork);
            if (groups[g].children[i].page == PAGE_INVOICES) allowed = allowed || auth.hasPermission(Permission::ViewOwnInvoices);
            groupAllowed = groupAllowed || allowed;
        }
        if (!groupAllowed) continue;
        Rectangle groupRect = { 12, (float)y, 226, 36 };
        bool hoverGroup = CheckCollisionPointRec(mouse, groupRect);
        if (hoverGroup) DrawRectangleRounded(groupRect, 0.12f, 7, NAV_HOVER);
        label(fitText(string(ui.groupOpen[g] ? "v  " : ">  ") + groups[g].name, groupRect.width - 18, 17, FONT_SEMIBOLD), 22, y + 7, 17, WHITE, FONT_SEMIBOLD);
        if (hoverGroup && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ui.groupOpen[g] = !ui.groupOpen[g];
        y += 37;
        if (!ui.groupOpen[g]) continue;
        for (size_t i = 0; i < groups[g].children.size(); ++i) {
            const Item& item = groups[g].children[i];
            bool allowed = auth.hasPermission(item.permission);
            if (item.page == PAGE_ATTENDANCE) allowed = allowed || auth.hasPermission(Permission::ViewOwnAttendance);
            if (item.page == PAGE_ASSIGNMENTS) allowed = allowed || auth.hasPermission(Permission::ViewOwnWork);
            if (item.page == PAGE_INVOICES) allowed = allowed || auth.hasPermission(Permission::ViewOwnInvoices);
            if (!allowed) continue;
            Rectangle child = { 22, (float)y, 216, 32 };
            bool active = !item.exportAction && ui.page == item.page;
            bool hover = CheckCollisionPointRec(mouse, child);
            if (active || hover) DrawRectangleRounded(child, 0.12f, 7, active ? NAV_ACTIVE : NAV_HOVER);
            label(fitText(item.name, child.width - 28, UITheme::FontSidebar, FONT_MEDIUM), 36, y + 6, (int)UITheme::FontSidebar, WHITE, FONT_MEDIUM);
            if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (item.exportAction) exportReport(app, ui);
                else { ui.page = item.page; ui.selectedRow = -1; ui.scroll = 0; ui.search.clear(); }
            }
            y += 33;
        }
        y += 3;
    }

    const User* user = auth.getCurrentUser();
    DrawLine(18, WINDOW_HEIGHT - 72, SIDEBAR_WIDTH - 18, WINDOW_HEIGHT - 72, Color{93, 139, 119, 255});
    if (user) {
        label(fitText(user->getUsername(), SIDEBAR_WIDTH - 48, 17, FONT_MEDIUM), 24, WINDOW_HEIGHT - 56, 17, WHITE, FONT_MEDIUM);
        label(User::roleToString(user->getRole()), 24, WINDOW_HEIGHT - 34, 15, WHITE, FONT_REGULAR);
    }
}

void drawHeader(UiState& ui, AppData& app)
{
    DrawRectangle(SIDEBAR_WIDTH, 0, WINDOW_WIDTH - SIDEBAR_WIDTH, 82, SURFACE);
    DrawLine(SIDEBAR_WIDTH, 81, WINDOW_WIDTH, 81, LINE);
    const char* titles[] = { "Tổng quan", "Quản lý nhân viên", "Quản lý phân công", "Quản lý chấm công", "Quản lý phương tiện", "Quản lý khách hàng", "Quản lý hóa đơn", "Quản lý chi phí", "Quản lý tài khoản", "Báo cáo doanh thu", "Báo cáo lợi nhuận" };
    label(titles[ui.page], SIDEBAR_WIDTH + 30, 10, (int)UITheme::FontPageTitle, INK, FONT_BOLD);
    label("Hệ thống quản lý thu gom rác thải đô thị", SIDEBAR_WIDTH + 30, 48, 15, MUTED, FONT_REGULAR);

    const User* user = app.auth.getCurrentUser();
    if (user) {
        string displayName = user->getUsername();
        if (user->getRole() == UserRole::Manager || user->getRole() == UserRole::Employee) {
            const Employee* employee = app.employees.findEmployeeById(user->getLinkedId());
            if (employee) displayName = employee->get_name();
        } else {
            const Customer* customer = app.customers.findCustomerById(user->getLinkedId());
            if (customer) displayName = customer->getCustomer_name();
        }
        label(fitText(user->getLinkedId() + " · " + displayName, 250, 16, FONT_MEDIUM), 1000, 18, 16, INK, FONT_MEDIUM);
        label(User::roleToString(user->getRole()), 1000, 44, 15, MUTED, FONT_REGULAR);
    }
    if (button({ 1280, 20, 132, 42 }, "Đăng xuất")) {
        app.auth.logout();
        ui.loginMode = true; ui.loginStep = 0; ui.selectedRole = -1;
        ui.username.clear(); ui.password.clear(); ui.loginField = 0; ui.showPassword = false;
    }
}

void readSearch(UiState& ui)
{
    if (ui.loginField != 2) return;
    const string previous = ui.search;
    int key = GetCharPressed();
    while (key > 0) {
        appendCodepoint(ui.search, key, 128);
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !ui.search.empty()) {
        size_t at = ui.search.size() - 1;
        while (at > 0 && (static_cast<unsigned char>(ui.search[at]) & 0xC0) == 0x80) --at;
        ui.search.erase(at);
    }
    if (IsKeyPressed(KEY_ESCAPE)) ui.loginField = 0;
    if (ui.search != previous) { ui.scroll = 0; ui.selectedRow = -1; }
}

template <typename RowFn>
void tableBase(UiState& ui, const string& subtitle, const vector<string>& headers, int total, RowFn rowFn)
{
    label(subtitle, SIDEBAR_WIDTH + UITheme::SpaceLG, 106, 20, INK, FONT_SEMIBOLD);
    Rectangle search = { WINDOW_WIDTH - 365.0f, 98.0f, 335.0f, 40.0f };
    DrawRectangleRounded(search, 0.16f, 8, SURFACE);
    DrawRectangleRoundedLines(search, 0.16f, 8, LINE);
    label(ui.search.empty() ? "Tìm theo mã, tên hoặc trạng thái..." : fitText(ui.search, search.width - 20, UITheme::FontCaption), (int)search.x + 12, (int)search.y + 10, (int)UITheme::FontCaption, ui.search.empty() ? MUTED : INK, FONT_REGULAR);
    if (CheckCollisionPointRec(logicalMouse(), search) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 2;
    Rectangle panel = { SIDEBAR_WIDTH + 30.0f, 150.0f, WINDOW_WIDTH - SIDEBAR_WIDTH - 60.0f, 574.0f };
    DrawRectangleRounded(panel, 0.025f, 6, SURFACE);
    DrawRectangleRoundedLines(panel, 0.025f, 6, LINE);
    float left = panel.x + UITheme::SpaceLG;
    float top = panel.y + UITheme::SpaceLG;
    float usableWidth = panel.width - 2 * UITheme::SpaceLG;
    float colWidth = usableWidth / static_cast<float>(headers.size());
    for (size_t c = 0; c < headers.size(); ++c) {
        const string fitted = fitText(headers[c], colWidth - 12, UITheme::FontTable, FONT_SEMIBOLD);
        label(fitted, static_cast<int>(left + c * colWidth), static_cast<int>(top), (int)UITheme::FontTable, MUTED, FONT_SEMIBOLD);
    }
    DrawLine(static_cast<int>(left), static_cast<int>(top + 29), static_cast<int>(left + usableWidth), static_cast<int>(top + 29), LINE);
    int visible = 9;
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
        float y = top + 42 + shown * 52.0f;
        Rectangle row = { left - 8, y - 8, usableWidth + 16, 46 };
        bool selected = ui.selectedRow == i;
        bool hover = CheckCollisionPointRec(logicalMouse(), row);
        if (selected || hover) DrawRectangleRounded(row, 0.12f, 6, selected ? UI_BLUE_LIGHT : BG);
        for (size_t c = 0; c < headers.size() && c < values.size(); ++c) {
            string text = fitText(values[c], colWidth - 12, UITheme::FontTable, FONT_REGULAR);
            if (isKnownStatus(values[c])) {
                const Color tone = statusColor(values[c]);
                const float badgeW = min(colWidth - 12, textWidth(text, UITheme::FontCaption, FONT_MEDIUM) + 20);
                DrawRectangleRounded({left + c * colWidth, y, badgeW, 30}, 0.25f, 8, Color{tone.r, tone.g, tone.b, 30});
                label(text, static_cast<int>(left + c * colWidth + 10), static_cast<int>(y + 6), (int)UITheme::FontCaption, tone, FONT_MEDIUM);
            } else {
                Color color = c == 0 ? INK : MUTED;
                label(text, static_cast<int>(left + c * colWidth), static_cast<int>(y + 3), (int)UITheme::FontTable, color, FONT_REGULAR);
            }
        }
        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ui.selectedRow = i;
        ++shown;
    }
    if (total == 0) label("Chưa có dữ liệu.", static_cast<int>(left + 4), static_cast<int>(top + 64), 16, MUTED);
    else if (shown == 0) label("Không tìm thấy dòng phù hợp.", static_cast<int>(left + 4), static_cast<int>(top + 64), 16, MUTED);
    if (total > visible) {
        if (button({ panel.x + panel.width - 190, panel.y + panel.height - 50, 82, 40 }, "Trước")) ui.scroll = max(0, ui.scroll - visible);
        if (button({ panel.x + panel.width - 98, panel.y + panel.height - 50, 82, 40 }, "Sau")) ui.scroll = min(maxScroll, ui.scroll + visible);
        label(to_string(min(total, ui.scroll + visible)) + " / " + to_string(total), static_cast<int>(panel.x + UITheme::SpaceLG), static_cast<int>(panel.y + panel.height - 40), 16, MUTED, FONT_REGULAR);
    }
}

void drawDashboard(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    if (!user) return;
    const int x = SIDEBAR_WIDTH + 30;
    const int contentWidth = WINDOW_WIDTH - SIDEBAR_WIDTH - 60;
    const int gap = 16;
    const int cardWidth = (contentWidth - 2 * gap) / 3;

    if (user->getRole() == UserRole::Manager) {
        label("Chào mừng trở lại, " + user->getUsername(), x, 92, 17, MUTED);
        const long long revenue = app.invoices.calculateCollectedRevenue();
        const long long costs = app.expenses.calculateTotalExpenses();
        const long long payroll = app.employees.calculateTotalSalaryFund();
        const long long profit = revenue - costs - payroll;
        drawCard({ (float)x, 116, (float)cardWidth, 92 }, "Nhân viên", to_string(app.employees.getSize()), "Hồ sơ trong EmployeeManager", UI_GREEN);
        drawCard({ (float)(x + cardWidth + gap), 116, (float)cardWidth, 92 }, "Phương tiện", to_string(app.vehicles.getSize()), "Phương tiện đang quản lý", UI_GREEN);
        drawCard({ (float)(x + 2 * (cardWidth + gap)), 116, (float)cardWidth, 92 }, "Khách hàng", to_string(app.customers.getSize()), "Hồ sơ khách hàng", UI_GREEN);
        drawCard({ (float)x, 218, (float)cardWidth, 92 }, "Hóa đơn", to_string(app.invoices.getSize()), "Tổng số hóa đơn", UI_BLUE);
        drawCard({ (float)(x + cardWidth + gap), 218, (float)cardWidth, 92 }, "Doanh thu đã thu", money(revenue), "Hóa đơn đã thanh toán", UI_GREEN);
        drawCard({ (float)(x + 2 * (cardWidth + gap)), 218, (float)cardWidth, 92 }, "Lợi nhuận ước tính", money(profit), "Đã trừ chi phí và quỹ lương", profit >= 0 ? UI_GREEN : UI_RED);

        Rectangle left = { (float)x, 326, 650, 380 };
        Rectangle right = { (float)(x + 670), 326, (float)(contentWidth - 670), 380 };
        DrawRectangleRounded(left, 0.035f, 8, SURFACE); DrawRectangleRoundedLines(left, 0.035f, 8, LINE);
        DrawRectangleRounded(right, 0.035f, 8, SURFACE); DrawRectangleRoundedLines(right, 0.035f, 8, LINE);
        label("Tình hình phân công", (int)left.x + 22, (int)left.y + 20, 20, INK);
        int assigned = 0, planned = 0, completed = 0;
        for (int i = 0; i < app.assignments.getSize(); ++i) {
            const string status = app.assignments[i].getAssignment_status();
            if (status == "Assigned") ++assigned;
            else if (status == "Planned") ++planned;
            else if (status == "Completed") ++completed;
        }
        const int statusW = 188;
        drawCard({ left.x + 20, left.y + 62, (float)statusW, 98 }, "Đang thực hiện", to_string(assigned), "Assignment Assigned", UI_GREEN);
        drawCard({ left.x + 222, left.y + 62, (float)statusW, 98 }, "Chờ xác nhận", to_string(planned), "Assignment Planned", UI_AMBER);
        drawCard({ left.x + 424, left.y + 62, (float)statusW, 98 }, "Đã hoàn tất", to_string(completed), "Assignment Completed", UI_BLUE);
        label("Chi phí vận hành", (int)left.x + 24, (int)left.y + 192, 16, MUTED);
        label(money(costs), (int)left.x + 250, (int)left.y + 190, 18, INK);
        label("Quỹ lương cơ bản", (int)left.x + 24, (int)left.y + 237, 16, MUTED);
        label(money(payroll), (int)left.x + 250, (int)left.y + 235, 18, INK);
        label("Công nợ hóa đơn", (int)left.x + 24, (int)left.y + 282, 16, MUTED);
        label(money(app.invoices.calculateTotalInvoiced() - revenue), (int)left.x + 250, (int)left.y + 280, 18, UI_AMBER);

        label("Truy cập nhanh", (int)right.x + 22, (int)right.y + 20, 20, INK);
        const char* names[] = { "Nhân viên", "Phân công", "Chấm công", "Phương tiện", "Khách hàng", "Hóa đơn", "Chi phí", "Tài khoản", "Doanh thu", "Lợi nhuận" };
        const Page pages[] = { PAGE_EMPLOYEES, PAGE_ASSIGNMENTS, PAGE_ATTENDANCE, PAGE_VEHICLES, PAGE_CUSTOMERS, PAGE_INVOICES, PAGE_EXPENSES, PAGE_ACCOUNTS, PAGE_REPORTS, PAGE_PROFIT };
        const Permission permissions[] = { Permission::ManageEmployees, Permission::ManageAssignments, Permission::ViewAllAttendance, Permission::ManageVehicles, Permission::ManageCustomers, Permission::ManageInvoices, Permission::ManageExpenses, Permission::ManageAccounts, Permission::ViewReports, Permission::ViewReports };
        for (int i = 0; i < 10; ++i) {
            const int col = i % 2, row = i / 2;
            Rectangle tile = { right.x + 18 + col * 214.0f, right.y + 63 + row * 57.0f, 198, 46 };
            if (app.auth.hasPermission(permissions[i]) && button(tile, names[i])) {
                ui.page = pages[i]; ui.search.clear(); ui.selectedRow = -1; ui.scroll = 0;
            }
        }
        return;
    }

    if (user->getRole() == UserRole::Employee) {
        const string today = app.resolveDate("TODAY+0");
        const Assignment* todayAssignment = nullptr;
        int todayCount = 0, ownCount = 0;
        for (int i = 0; i < app.assignments.getSize(); ++i) {
            const Assignment& a = app.assignments[i];
            if (!a.hasEmployee(user->getLinkedId())) continue;
            ++ownCount;
            if (a.getAssignment_date() == today) { ++todayCount; if (!todayAssignment) todayAssignment = &a; }
        }
        const Attendance* todayRecord = todayAssignment ? app.attendance.findAttendance(user->getLinkedId(), todayAssignment->getAssignment_id()) : nullptr;
        const string attendanceState = todayRecord ? todayRecord->getAttendance_status() : "Chưa có ca";
        const int cardW = (contentWidth - 2 * gap) / 3;
        drawCard({ (float)x, 112, (float)cardW, 100 }, "Nhiệm vụ hôm nay", to_string(todayCount), "Tổng lịch cá nhân: " + to_string(ownCount), UI_GREEN);
        drawCard({ (float)(x + cardW + gap), 112, (float)cardW, 100 }, "Chấm công hôm nay", attendanceState, todayRecord && !todayRecord->getCheck_in().empty() ? "Check-in " + todayRecord->getCheck_in() : "Chưa ghi nhận Check-in", statusColor(attendanceState));
        drawCard({ (float)(x + 2 * (cardW + gap)), 112, (float)cardW, 100 }, "Ca làm hôm nay", todayAssignment ? todayAssignment->getShift_id() : "Chưa có ca", todayAssignment ? todayAssignment->getAssignment_id() + " · " + todayAssignment->getAssignment_status() : today, todayAssignment ? statusColor(todayAssignment->getAssignment_status()) : MUTED);
        Rectangle panel = { (float)x, 232, (float)contentWidth, 474 };
        DrawRectangleRounded(panel, 0.035f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.035f, 8, LINE);
        label("Lịch làm việc của bạn", (int)panel.x + 24, (int)panel.y + 22, 21, INK);
        label("Assignment", (int)panel.x + 26, (int)panel.y + 72, 15, MUTED);
        label("Ngày / ca", (int)panel.x + 250, (int)panel.y + 72, 15, MUTED);
        label("Phương tiện", (int)panel.x + 520, (int)panel.y + 72, 15, MUTED);
        label("Trạng thái", (int)panel.x + 770, (int)panel.y + 72, 15, MUTED);
        int row = 0;
        for (int i = 0; i < app.assignments.getSize() && row < 6; ++i) {
            const Assignment& a = app.assignments[i];
            if (!a.hasEmployee(user->getLinkedId())) continue;
            const float ry = panel.y + 108 + row * 44.0f;
            if (row % 2 == 0) DrawRectangleRounded({ panel.x + 16, ry - 5, panel.width - 32, 38 }, 0.12f, 6, BG);
            label(a.getAssignment_id(), (int)panel.x + 26, (int)ry + 2, 17, INK);
            label(a.getAssignment_date() + " · " + a.getShift_id(), (int)panel.x + 250, (int)ry + 2, 16, MUTED);
            label(a.getVehicle_id().empty() ? "Chưa gán" : a.getVehicle_id(), (int)panel.x + 520, (int)ry + 2, 16, MUTED);
            label(a.getAssignment_status(), (int)panel.x + 770, (int)ry + 2, 16, statusColor(a.getAssignment_status()));
            ++row;
        }
        if (row == 0) label("Hiện chưa có Assignment được phân công.", (int)panel.x + 26, (int)panel.y + 112, 16, MUTED);
        const float checkY = panel.y + panel.height - 64;
        if (todayAssignment && todayRecord && todayAssignment->getAssignment_status() == "Assigned") {
            const bool canCheckIn = todayRecord->getCheck_in().empty();
            const bool canCheckOut = !todayRecord->getCheck_in().empty() && todayRecord->getCheck_out().empty();
            if (canCheckIn && button({ panel.x + 24, checkY, 150, 42 }, "Check-in", true)) {
                try { const bool ok = app.work.checkInByEmployee(user->getLinkedId(), todayAssignment->getAssignment_id()); setMessage(ui, ok ? "Check-in thành công" : "Không thể Check-in: kiểm tra ngày và Assignment", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR); }
                catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
            } else if (!canCheckIn) disabledButton({ panel.x + 24, checkY, 150, 42 }, "Check-in đã ghi nhận");
            if (canCheckOut && button({ panel.x + 190, checkY, 150, 42 }, "Check-out", true)) {
                try { const bool ok = app.work.checkOutByEmployee(user->getLinkedId(), todayAssignment->getAssignment_id()); setMessage(ui, ok ? "Check-out thành công" : "Không thể Check-out: cần Check-in và Assignment phù hợp", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR); }
                catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
            } else if (!canCheckOut) disabledButton({ panel.x + 190, checkY, 150, 42 }, todayRecord->getCheck_out().empty() ? "Check-out sau Check-in" : "Check-out đã ghi nhận");
        } else {
            label("Nút chấm công sẽ khả dụng khi có Assignment đang thực hiện hôm nay.", (int)panel.x + 24, (int)checkY + 8, 15, MUTED);
        }
        return;
    }

    int invoiceCount = 0, unpaidCount = 0;
    long long unpaidAmount = 0;
    for (int i = 0; i < app.invoices.getSize(); ++i) {
        const Invoice& inv = app.invoices[i];
        if (inv.getCustomer_id() != user->getLinkedId()) continue;
        ++invoiceCount;
        if (inv.getInvoice_status() != "Paid") { ++unpaidCount; unpaidAmount += inv.getTotal_amount(); }
    }
    const int customerCardW = (contentWidth - 2 * gap) / 3;
    drawCard({ (float)x, 112, (float)customerCardW, 100 }, "Hóa đơn của tôi", to_string(invoiceCount), "Tổng số hóa đơn", UI_GREEN);
    drawCard({ (float)(x + customerCardW + gap), 112, (float)customerCardW, 100 }, "Chưa thanh toán", to_string(unpaidCount), "Hóa đơn đang chờ thanh toán", UI_AMBER);
    drawCard({ (float)(x + 2 * (customerCardW + gap)), 112, (float)customerCardW, 100 }, "Số tiền còn lại", money(unpaidAmount), "Theo hóa đơn Unpaid", UI_AMBER);
    Rectangle panel = { (float)x, 232, (float)contentWidth, 474 };
    DrawRectangleRounded(panel, 0.035f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.035f, 8, LINE);
    label("Hóa đơn gần đây", (int)panel.x + 24, (int)panel.y + 22, 21, INK);
    label("Mã hóa đơn", (int)panel.x + 26, (int)panel.y + 72, 15, MUTED);
    label("Kỳ", (int)panel.x + 280, (int)panel.y + 72, 15, MUTED);
    label("Ngày lập", (int)panel.x + 500, (int)panel.y + 72, 15, MUTED);
    label("Số tiền", (int)panel.x + 730, (int)panel.y + 72, 15, MUTED);
    label("Trạng thái", (int)panel.x + 940, (int)panel.y + 72, 15, MUTED);
    int row = 0;
    for (int i = 0; i < app.invoices.getSize() && row < 7; ++i) {
        const Invoice& inv = app.invoices[i];
        if (inv.getCustomer_id() != user->getLinkedId()) continue;
        const float ry = panel.y + 108 + row * 44.0f;
        if (row % 2 == 0) DrawRectangleRounded({ panel.x + 16, ry - 5, panel.width - 32, 38 }, 0.12f, 6, BG);
        label(inv.getInvoice_id(), (int)panel.x + 26, (int)ry + 2, 17, INK);
        label(inv.getBilling_month(), (int)panel.x + 280, (int)ry + 2, 16, MUTED);
        label(inv.getInvoice_date(), (int)panel.x + 500, (int)ry + 2, 16, MUTED);
        label(fitText(money(inv.getTotal_amount()), 190, 16), (int)panel.x + 730, (int)ry + 2, 16, INK);
        label(inv.getInvoice_status(), (int)panel.x + 940, (int)ry + 2, 16, statusColor(inv.getInvoice_status()));
        ++row;
    }
    if (row == 0) label("Chưa có hóa đơn nào được liên kết với tài khoản này.", (int)panel.x + 26, (int)panel.y + 112, 16, MUTED);
}

void drawEmployees(AppData& app, UiState& ui)
{
    tableBase(ui, "Danh sách nhân viên", {"STT", "Mã nhân viên", "Họ tên", "Chức vụ", "Trạng thái", "Lương cơ bản"}, app.employees.getSize(), [&](int i) {
        const Employee* e = app.employees[i];
        return vector<string>{ to_string(i + 1), e->get_id(), e->get_name(), e->get_position(), e->get_status(), money(e->get_salary()) };
    });
    if (ui.selectedRow >= 0 && ui.selectedRow < app.employees.getSize()) {
        const Employee* e = app.employees[ui.selectedRow];
        label("Chi tiết: " + e->get_id() + "  |  " + e->get_name() + "  |  " + e->get_address(), SIDEBAR_WIDTH + 32, 728, 15, INK);
    }
}

void drawAssignments(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    vector<int> indices;
    for (int i = 0; i < app.assignments.getSize(); ++i) {
        const Assignment& a = app.assignments[i];
        if (user->getRole() == UserRole::Manager || a.hasEmployee(user->getLinkedId())) indices.push_back(i);
    }
    tableBase(ui, "Lịch phân công và thành viên đội", {"Mã phân công", "Ngày", "Ca", "Mã xe", "Đội nhân viên", "Số NV", "Trạng thái"}, (int)indices.size(), [&](int i) {
        const Assignment& a = app.assignments[indices[i]];
        string team;
        for (int j = 0; j < a.getEmployeeCount(); ++j) {
            if (!team.empty()) team += " / ";
            team += a.getEmployee_id(j);
        }
        return vector<string>{a.getAssignment_id(), a.getAssignment_date(), a.getShift_id(), a.getVehicle_id().empty() ? "-" : a.getVehicle_id(), team.empty() ? "Chưa xếp đội" : team, to_string(a.getEmployeeCount()), a.getAssignment_status()};
    });
    if (ui.selectedRow >= 0 && ui.selectedRow < (int)indices.size()) {
        const Assignment& a = app.assignments[indices[ui.selectedRow]];
        string names;
        for (int i = 0; i < a.getEmployeeCount(); ++i) {
            const Employee* e = app.employees.findEmployeeById(a.getEmployee_id(i));
            if (!names.empty()) names += "  |  ";
            names += a.getEmployee_id(i) + (e ? " " + e->get_name() : "");
        }
        if (names.empty()) names = "Chưa có nhân viên";
        label("Đội: " + fitText(names, WINDOW_WIDTH - SIDEBAR_WIDTH - 70, 15), SIDEBAR_WIDTH + 32, 728, 15, INK);
    }
    if (user->getRole() != UserRole::Manager) return;
}

void drawAttendance(AppData& app, UiState& ui)
{
    const User* user = app.auth.getCurrentUser();
    vector<int> indices;
    for (int i = 0; i < app.attendance.getSize(); ++i)
        if (user->getRole() == UserRole::Manager || app.attendance[i].getEmployee_id() == user->getLinkedId()) indices.push_back(i);
    tableBase(ui, "Theo dõi vào ca / tan ca", {"Mã NV", "Assignment", "Ngày", "Check-in", "Check-out", "Trạng thái"}, (int)indices.size(), [&](int i) {
        const Attendance& a = app.attendance[indices[i]];
        return vector<string>{a.getEmployee_id(), a.getAssignment_id(), a.getAttendance_date(), a.getCheck_in().empty() ? "-" : a.getCheck_in(), a.getCheck_out().empty() ? "-" : a.getCheck_out(), a.getAttendance_status()};
    });
    float y = 748;
    if (user->getRole() == UserRole::Employee) {
        if (button({ SIDEBAR_WIDTH + 30.0f, y, 156, 38 }, "Check-in", true) && ui.selectedRow >= 0 && ui.selectedRow < (int)indices.size()) {
            const Attendance& a = app.attendance[indices[ui.selectedRow]];
            try {
                bool ok = app.work.checkInByEmployee(user->getLinkedId(), a.getAssignment_id());
                setMessage(ui, ok ? "Check-in thành công" : "Không thể Check-in: kiểm tra ngày làm và trạng thái Assignment", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {SIDEBAR_WIDTH + 30.0f, y, 156, UITheme::ButtonHeight}) && ui.selectedRow < 0) {
            setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
        }
        if (button({ SIDEBAR_WIDTH + 198.0f, y, 156, 38 }, "Check-out") && ui.selectedRow >= 0 && ui.selectedRow < (int)indices.size()) {
            const Attendance& a = app.attendance[indices[ui.selectedRow]];
            try {
                bool ok = app.work.checkOutByEmployee(user->getLinkedId(), a.getAssignment_id());
                setMessage(ui, ok ? "Check-out thành công" : "Không thể Check-out: cần Check-in và Assignment phù hợp", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {SIDEBAR_WIDTH + 198.0f, y, 156, UITheme::ButtonHeight}) && ui.selectedRow < 0) {
            setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
        }
    }
}

void drawVehicles(AppData& app, UiState& ui)
{
    tableBase(ui, "Trạng thái phương tiện", {"Mã xe", "Trạng thái", "Tải trọng", "Số km"}, app.vehicles.getSize(), [&](int i) {
        const Vehicle& v = app.vehicles[i];
        return vector<string>{v.getVehicle_id(), v.getVehicle_status(), to_string(v.getVehicle_capacity()) + " tan", to_string((long long)v.getVehicle_km()) + " km"};
    });
}

void drawCustomers(AppData& app, UiState& ui)
{
    tableBase(ui, "Hồ sơ khách hàng", {"Mã KH", "Tên khách hàng", "Loại", "SĐT", "Khu vực"}, app.customers.getSize(), [&](int i) {
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
    tableBase(ui, "Hóa đơn và trạng thái thanh toán", {"Mã HĐ", "Mã KH", "Kỳ", "Ngày lập", "Số tiền", "Trạng thái"}, (int)indices.size(), [&](int i) {
        const Invoice& inv = app.invoices[indices[i]];
        return vector<string>{inv.getInvoice_id(), inv.getCustomer_id(), inv.getBilling_month(), inv.getInvoice_date(), money(inv.getTotal_amount()), inv.getInvoice_status()};
    });
}

void drawExpenses(AppData& app, UiState& ui)
{
    tableBase(ui, "Các khoản chi phí phát sinh", {"Mã CP", "Hạng mục", "Ngày", "Số tiền", "Mô tả"}, app.expenses.getSize(), [&](int i) {
        const Expense& e = app.expenses[i];
        return vector<string>{e.getExpense_id(), e.getCategory(), e.getDate(), money(e.getAmount()), e.getDescription()};
    });
}

void drawAccounts(AppData& app, UiState& ui)
{
    vector<UserSummary> users = app.auth.listUsers();
    tableBase(ui, "Tài khoản và quyền truy cập", {"Mã user", "Tên đăng nhập", "Vai trò", "Mã liên kết", "Trạng thái"}, (int)users.size(), [&](int i) {
        const UserSummary& u = users[i];
        return vector<string>{u.user_id, u.username, User::roleToString(u.role), u.linked_id, u.active ? "Active" : "Inactive"};
    });
}

void drawReports(AppData& app)
{
    int x = SIDEBAR_WIDTH + 30;
    const long long total = app.invoices.calculateTotalInvoiced();
    const long long collected = app.invoices.calculateCollectedRevenue();
    drawCard({ (float)x, 118, 350, 112 }, "Tổng đã lập hóa đơn", money(total), "Giá trị hóa đơn đã phát hành", UI_BLUE);
    drawCard({ (float)(x + 370), 118, 350, 112 }, "Doanh thu đã thu", money(collected), "Hóa đơn Paid", UI_GREEN);
    drawCard({ (float)(x + 740), 118, 350, 112 }, "Công nợ còn lại", money(total - collected), "Hóa đơn chưa thanh toán", UI_AMBER);
    Rectangle panel = { (float)x, 254, (float)(WINDOW_WIDTH - SIDEBAR_WIDTH - 60), 410 };
    DrawRectangleRounded(panel, 0.025f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.025f, 8, LINE);
    label("Tổng hợp doanh thu", (int)panel.x + 24, (int)panel.y + 24, 20, INK);
    label("Số hóa đơn", (int)panel.x + 28, (int)panel.y + 88, 16, MUTED); label(to_string(app.invoices.getSize()), (int)panel.x + 360, (int)panel.y + 88, 17, INK);
    label("Khách hàng có hóa đơn", (int)panel.x + 28, (int)panel.y + 142, 16, MUTED); label(to_string(app.customers.getSize()), (int)panel.x + 360, (int)panel.y + 142, 17, INK);
    label("Doanh thu đã ghi nhận", (int)panel.x + 28, (int)panel.y + 196, 16, MUTED); label(money(collected), (int)panel.x + 360, (int)panel.y + 196, 17, UI_GREEN);
    label("Công nợ", (int)panel.x + 28, (int)panel.y + 250, 16, MUTED); label(money(total - collected), (int)panel.x + 360, (int)panel.y + 250, 17, UI_AMBER);
    label("Chọn 'Xuất báo cáo' trong thanh điều hướng để tạo file TXT.", (int)panel.x + 28, (int)panel.y + 324, 15, MUTED);
}

void drawProfitReport(AppData& app)
{
    int x = SIDEBAR_WIDTH + 30;
    const long long revenue = app.invoices.calculateCollectedRevenue();
    const long long cost = app.expenses.calculateTotalExpenses();
    const long long payroll = app.employees.calculateTotalSalaryFund();
    const long long profit = revenue - cost - payroll;
    drawCard({ (float)x, 118, 350, 112 }, "Doanh thu đã thu", money(revenue), "Hóa đơn đã thanh toán", UI_GREEN);
    drawCard({ (float)(x + 370), 118, 350, 112 }, "Chi phí phát sinh", money(cost), "Không gồm lương", UI_AMBER);
    drawCard({ (float)(x + 740), 118, 350, 112 }, "Quỹ lương cơ bản", money(payroll), "Theo EmployeeManager", UI_BLUE);
    Rectangle panel = { (float)x, 254, (float)(WINDOW_WIDTH - SIDEBAR_WIDTH - 60), 300 };
    DrawRectangleRounded(panel, 0.025f, 8, SURFACE); DrawRectangleRoundedLines(panel, 0.025f, 8, LINE);
    label("Lợi nhuận ước tính", (int)panel.x + 28, (int)panel.y + 30, 20, INK);
    label("Doanh thu đã thu - chi phí phát sinh - quỹ lương cơ bản", (int)panel.x + 28, (int)panel.y + 82, 16, MUTED);
    label(money(profit), (int)panel.x + 28, (int)panel.y + 142, 32, profit >= 0 ? UI_GREEN : UI_RED);
}

void exportReport(AppData& app, UiState& ui)
{
    try {
        ofstream file("data/report_export.txt", ios::out | ios::trunc);
        if (!file.is_open()) throw runtime_error("Không tạo được data/report_export.txt");
        file << "PBL2 Operations Management\n"
             << "Revenue collected: " << app.invoices.calculateCollectedRevenue() << " VND\n"
             << "Total invoiced: " << app.invoices.calculateTotalInvoiced() << " VND\n"
             << "Outstanding: " << app.invoices.calculateTotalInvoiced() - app.invoices.calculateCollectedRevenue() << " VND\n"
             << "Operating expenses: " << app.expenses.calculateTotalExpenses() << " VND\n"
             << "Base salary fund: " << app.employees.calculateTotalSalaryFund() << " VND\n"
             << "Estimated profit: " << app.invoices.calculateCollectedRevenue() - app.expenses.calculateTotalExpenses() - app.employees.calculateTotalSalaryFund() << " VND\n";
        file.close();
        if (!file) throw runtime_error("Lỗi khi ghi data/report_export.txt");
        setMessage(ui, "Đã xuất báo cáo: data/report_export.txt", FEEDBACK_SUCCESS);
    } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
}

vector<string> formLabels(int kind)
{
    switch (kind) {
        case FORM_EMPLOYEE: return {"Mã nhân viên", "Họ tên", "Địa chỉ", "Chức vụ (Driver/Janitor/Manager)", "Trạng thái (Active/Inactive)"};
        case FORM_VEHICLE: return {"Mã xe", "Trạng thái (Available/Maintenance)", "Tải trọng", "Số km"};
        case FORM_CUSTOMER: return {"Mã khách hàng", "Loại (Household/Business)", "Tên khách hàng", "Số điện thoại", "Địa chỉ", "Khu vực (Area1/Area2/Type1/Type2)"};
        case FORM_INVOICE: return {"Mã hóa đơn", "Mã khách hàng", "Ngày lập (YYYY-MM-DD)", "Kỳ hóa đơn (YYYY-MM)"};
        case FORM_EXPENSE: return {"Mã chi phí", "Hạng mục", "Ngày (YYYY-MM-DD)", "Số tiền VND", "Mô tả"};
        case FORM_ACCOUNT: return {"Mã tài khoản", "Tên đăng nhập", "Mật khẩu", "Vai trò (Manager/Employee/Customer)", "Mã liên kết", "Trạng thái (Active/Disabled)"};
        case FORM_ASSIGNMENT: return {"Mã Assignment", "Ngày (YYYY-MM-DD)", "Ca (CA01/CA02/CA03)"};
        case FORM_ADD_MEMBER: return {"Mã Assignment", "Mã nhân viên"};
        case FORM_ASSIGN_VEHICLE: return {"Mã Assignment", "Mã xe"};
        case FORM_MANAGER_CHECKIN: return {"Mã nhân viên", "Mã Assignment", "Check-in (HH:MM)"};
        case FORM_MANAGER_CHECKOUT: return {"Mã nhân viên", "Mã Assignment", "Check-out (HH:MM)"};
        default: return vector<string>();
    }
}

void openForm(UiState& ui, int kind, const string& context = "")
{
    ui.formKind = kind;
    ui.formEdit = false;
    ui.formKey.clear();
    ui.formField = context.empty() ? 0 : 1;
    ui.formErrorField = -1;
    ui.formValues = vector<string>(formLabels(kind).size());
    if (!context.empty() && !ui.formValues.empty()) ui.formValues[0] = context;
}

void openEditForm(AppData& app, UiState& ui)
{
    if (ui.selectedRow < 0) { setMessage(ui, "Hãy chọn một dòng trước", FEEDBACK_WARNING); return; }
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
            setMessage(ui, "Khách hàng có hóa đơn; không thể sửa thông tin ảnh hưởng phí", FEEDBACK_WARNING); return;
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
        setMessage(ui, "Mục này chưa hỗ trợ chỉnh sửa", FEEDBACK_WARNING);
    }
}

void saveForm(AppData& app, UiState& ui)
{
    try {
        ui.formErrorField = -1;
        const vector<string>& f = ui.formValues;
        const vector<string> fields = formLabels(ui.formKind);
        for (size_t i = 0; i < f.size(); ++i) {
            if (f[i].empty()) { ui.formErrorField = (int)i; throw invalid_argument(fields[i] + " là trường bắt buộc"); }
            if (f[i].find_first_of("|\r\n") != string::npos) { ui.formErrorField = (int)i; throw invalid_argument(fields[i] + " chứa ký tự không hợp lệ"); }
        }
        bool assignmentNeedsReconfirmation = false;
        switch (ui.formKind) {
            case FORM_EMPLOYEE: {
                if (ui.formEdit && f[0] != ui.formKey) throw invalid_argument("Không được đổi mã nhân viên");
                Employee* item = nullptr;
                if (f[3] == "Driver") item = new Driver(f[0], f[1], f[2]);
                else if (f[3] == "Janitor") item = new Janitor(f[0], f[1], f[2]);
                else if (f[3] == "Manager") item = new Manager(f[0], f[1], f[2]);
                else throw invalid_argument("Chức vụ phải là Driver, Janitor hoặc Manager");
                try {
                    item->set_status(f[4]);
                    if (ui.formEdit) { if (!app.work.editEmployee(ui.formKey, *item)) throw invalid_argument("Không sửa được nhân viên theo trạng thái hiện tại"); }
                    else app.work.addEmployee(*item);
                }
                catch (...) { delete item; throw; }
                delete item;
                break;
            }
            case FORM_VEHICLE: {
                Vehicle item(f[0], f[1], stod(f[2]), stod(f[3]));
                if (ui.formEdit) {
                if (f[0] != ui.formKey || !app.vehicles.editVehicle(ui.formKey, item)) throw invalid_argument("Không sửa được phương tiện");
                } else app.vehicles.addVehicle(item);
                app.work.syncVehicleStatuses();
                break;
            }
            case FORM_CUSTOMER: {
                Customer item(f[0], f[1], f[2], f[3], f[4], f[5]);
                item.validate();
                (void)app.fees.getFee(item.getCustomer_type(), item.getService_area());
                if (ui.formEdit) {
                    if (f[0] != ui.formKey || !app.customers.editCustomer(ui.formKey, item)) throw invalid_argument("Không sửa được khách hàng theo trạng thái hiện tại");
                } else app.customers.addCustomer(item);
                break;
            }
            case FORM_INVOICE: {
                const Customer* customer = app.customers.findCustomerById(f[1]);
                if (!customer) throw invalid_argument("Không tìm thấy khách hàng");
                long long amount = app.fees.getFee(customer->getCustomer_type(), customer->getService_area());
                app.invoices.addInvoice(Invoice(f[0], f[1], f[2], amount, f[3], "Unpaid"));
                break;
            }
            case FORM_EXPENSE: {
                Expense item(f[0], f[1], stoll(f[3]), f[2], f[4]);
                if (ui.formEdit) {
                    if (f[0] != ui.formKey || !app.expenses.editExpense(ui.formKey, item)) throw invalid_argument("Không sửa được chi phí theo trạng thái hiện tại");
                } else app.expenses.addExpense(item);
                break;
            }
            case FORM_ACCOUNT:
                if (f[5] != "Active" && f[5] != "Disabled") throw invalid_argument("Trạng thái tài khoản phải là Active hoặc Disabled");
                app.auth.addUser(User(f[0], f[1], f[2], User::roleFromString(f[3]), f[4], f[5] != "Disabled"));
                break;
            case FORM_ASSIGNMENT:
                if (ui.formEdit) {
                    if (f[0] != ui.formKey) throw invalid_argument("Không được đổi mã Assignment");
                    Assignment* current = app.assignments.findAssignmentById(ui.formKey);
                    if (!current) throw invalid_argument("Không tìm thấy Assignment");
                    Assignment updated(ui.formKey, current->getEmployee_ids(), current->getVehicle_id(), f[2], f[1], current->getAssignment_status());
                    const bool wasAssigned = current->getAssignment_status() == "Assigned";
                    if (!app.work.editAssignment(ui.formKey, updated)) throw invalid_argument("Không sửa được Assignment");
                    const Assignment* after = app.assignments.findAssignmentById(ui.formKey);
                    assignmentNeedsReconfirmation = wasAssigned && after && after->getAssignment_status() == "Planned";
                } else app.work.createAssignment(Assignment(f[0], vector<string>(), "", f[2], f[1], "Planned"));
                break;
            case FORM_ADD_MEMBER: {
                const Assignment* before = app.assignments.findAssignmentById(f[0]);
                const bool wasAssigned = before && before->getAssignment_status() == "Assigned";
                if (!app.work.addEmployeeToAssignment(f[0], f[1])) throw invalid_argument("Không thêm được nhân viên: kiểm tra Assignment, trạng thái ca và thời gian chỉnh sửa");
                const Assignment* after = app.assignments.findAssignmentById(f[0]);
                assignmentNeedsReconfirmation = wasAssigned && after && after->getAssignment_status() == "Planned";
                break;
            }
            case FORM_ASSIGN_VEHICLE: {
                const Assignment* before = app.assignments.findAssignmentById(f[0]);
                const bool wasAssigned = before && before->getAssignment_status() == "Assigned";
                if (!app.work.assignVehicle(f[0], f[1])) throw invalid_argument("Không gán được xe: kiểm tra xe, trạng thái bảo trì và lịch trùng");
                const Assignment* after = app.assignments.findAssignmentById(f[0]);
                assignmentNeedsReconfirmation = wasAssigned && after && after->getAssignment_status() == "Planned";
                break;
            }
            case FORM_MANAGER_CHECKIN: {
                const Attendance* record = app.attendance.findAttendance(f[0], f[1]);
                bool ok = record && !record->getCheck_in().empty()
                    ? app.work.correctCheckInByManager(f[0], f[1], f[2])
                    : app.work.recordCheckInByManager(f[0], f[1], f[2]);
                if (!ok) throw invalid_argument("Không thể ghi Check-in cho nhân viên/Assignment này");
                break;
            }
            case FORM_MANAGER_CHECKOUT: {
                const Attendance* record = app.attendance.findAttendance(f[0], f[1]);
                bool ok = record && !record->getCheck_out().empty()
                    ? app.work.correctCheckOutByManager(f[0], f[1], f[2])
                    : app.work.recordCheckOutByManager(f[0], f[1], f[2]);
                if (!ok) throw invalid_argument("Không thể ghi Check-out cho nhân viên/Assignment này");
                break;
            }
            default: throw logic_error("Form không hợp lệ");
        }
        setMessage(ui, assignmentNeedsReconfirmation
            ? "Đã cập nhật. Assignment chuyển về Planned và cần xác nhận lại."
            : "Đã cập nhật dữ liệu trong phiên hiện tại",
            assignmentNeedsReconfirmation ? FEEDBACK_WARNING : FEEDBACK_SUCCESS);
        ui.formKind = FORM_NONE;
        ui.formEdit = false;
        ui.formKey.clear();
        ui.formValues.clear();
    } catch (const exception& e) {
        setMessage(ui, e.what(), FEEDBACK_ERROR, 5);
    }
}

void readForm(UiState& ui, AppData& app)
{
    if (ui.formKind == FORM_NONE) return;
    int key = GetCharPressed();
    while (key > 0) {
        if (ui.formField >= 0 && ui.formField < (int)ui.formValues.size()) {
            appendCodepoint(ui.formValues[ui.formField], key, 256);
            if (ui.formErrorField == ui.formField) { ui.formErrorField = -1; ui.message.clear(); ui.messageTimer = 0; }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && ui.formField >= 0 && ui.formField < (int)ui.formValues.size() && !ui.formValues[ui.formField].empty()) {
        string& value = ui.formValues[ui.formField];
        size_t at = value.size() - 1;
        while (at > 0 && (static_cast<unsigned char>(value[at]) & 0xC0) == 0x80) --at;
        value.erase(at);
        if (ui.formErrorField == ui.formField) { ui.formErrorField = -1; ui.message.clear(); ui.messageTimer = 0; }
    }
    if (IsKeyPressed(KEY_TAB) && !ui.formValues.empty()) ui.formField = (ui.formField + 1) % (int)ui.formValues.size();
    if (IsKeyPressed(KEY_ESCAPE)) { ui.formKind = FORM_NONE; ui.formValues.clear(); }
    if (IsKeyPressed(KEY_ENTER)) saveForm(app, ui);
}

void drawForm(AppData& app, UiState& ui)
{
    if (ui.formKind == FORM_NONE) return;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{15, 25, 42, 125});
    vector<string> labels = formLabels(ui.formKind);
    float height = 190 + (float)labels.size() * 76;
    Rectangle panel = { 410, (WINDOW_HEIGHT - height) / 2, 620, height };
    DrawRectangleRounded(panel, 0.035f, 8, SURFACE);
    DrawRectangleRoundedLines(panel, 0.035f, 8, LINE);
    const char* titles[] = {"", "Thêm nhân viên", "Thêm phương tiện", "Thêm khách hàng", "Lập hóa đơn", "Thêm chi phí", "Tạo tài khoản", "Tạo Assignment Planned", "Thêm thành viên", "Gán phương tiện", "Ghi / sửa Check-in", "Ghi / sửa Check-out"};
    const string formTitle = ui.formEdit ? string("Chỉnh sửa: ") + titles[ui.formKind] : titles[ui.formKind];
    label(formTitle, (int)panel.x + 26, (int)panel.y + 22, (int)UITheme::FontSectionTitle, INK, FONT_SEMIBOLD);
    for (size_t i = 0; i < labels.size(); ++i) {
        float y = panel.y + 66 + i * 76;
        label(labels[i], (int)panel.x + UITheme::SpaceLG, (int)y, (int)UITheme::FontCaption, MUTED, FONT_MEDIUM);
        Rectangle input = { panel.x + UITheme::SpaceLG, y + 23, panel.width - 2 * UITheme::SpaceLG, 40 };
        bool active = ui.formField == (int)i;
        const bool invalid = ui.formErrorField == (int)i;
        DrawRectangleRounded(input, 0.14f, 6, invalid ? Color{252, 232, 234, 255} : active ? GREEN_PALE : BG);
        DrawRectangleRoundedLines(input, 0.14f, 6, invalid ? UI_RED : active ? UI_GREEN : LINE);
        string value = ui.formValues[i];
        if (ui.formKind == FORM_INVOICE && i == 1 && value.empty()) value = "Nhập mã khách hàng trước";
        label(fitText(value, input.width - 2 * UITheme::SpaceXS, UITheme::FontBody), (int)input.x + UITheme::SpaceXS, (int)input.y + 9, (int)UITheme::FontBody, value.empty() ? MUTED : INK, FONT_REGULAR);
        if (invalid && ui.messageTimer > 0 && !ui.message.empty())
            label(fitText(ui.message, panel.width - 60, UITheme::FontCaption), (int)panel.x + 30, (int)(y + 65), (int)UITheme::FontCaption, UI_RED, FONT_MEDIUM);
        if (CheckCollisionPointRec(logicalMouse(), input) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.formField = (int)i;
    }
        if (ui.formErrorField < 0 && ui.messageTimer > 0 && !ui.message.empty())
        label(fitText(ui.message, panel.width - 52, UITheme::FontCaption), (int)panel.x + 26, (int)(panel.y + panel.height - 82), (int)UITheme::FontCaption, feedbackColor(ui.messageKind), FONT_MEDIUM);
    if (button({panel.x + panel.width - 250, panel.y + panel.height - 54, 104, UITheme::ButtonHeight}, "Hủy")) { setMessage(ui, "Đã hủy thao tác", FEEDBACK_INFO); ui.formKind = FORM_NONE; ui.formValues.clear(); }
    if (button({panel.x + panel.width - 132, panel.y + panel.height - 54, 106, UITheme::ButtonHeight}, "Lưu", true)) saveForm(app, ui);
}

void deleteSelected(AppData& app, UiState& ui)
{
    bool removed = false;
    try {
        if (ui.selectedRow < 0) throw invalid_argument("Hãy chọn một dòng trước");
        switch (ui.page) {
            case PAGE_EMPLOYEES: removed = app.work.deleteEmployee(app.employees[ui.selectedRow]->get_id()); break;
            case PAGE_VEHICLES: removed = app.work.deleteVehicle(app.vehicles[ui.selectedRow].getVehicle_id()); break;
            case PAGE_CUSTOMERS: {
                const string id = app.customers[ui.selectedRow].getCustomer_id();
                if (app.invoices.hasInvoicesForCustomer(id)) throw invalid_argument("Khách hàng đang có hóa đơn, không thể xóa");
                removed = app.customers.deleteCustomer(id);
                break;
            }
            case PAGE_INVOICES: removed = app.invoices.deleteInvoice(app.invoices[ui.selectedRow].getInvoice_id()); break;
            case PAGE_EXPENSES: removed = app.expenses.deleteExpense(app.expenses[ui.selectedRow].getExpense_id()); break;
            case PAGE_ASSIGNMENTS: removed = app.work.deleteAssignment(app.assignments[ui.selectedRow].getAssignment_id()); break;
            default: throw invalid_argument("Mục này không hỗ trợ xóa");
        }
        setMessage(ui, removed ? "Đã xóa bản ghi" : "Không thể xóa bản ghi theo trạng thái hiện tại", removed ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
    } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
    if (removed) ui.selectedRow = -1;
}

void requestDelete(UiState& ui)
{
    if (ui.selectedRow < 0) {
        setMessage(ui, "Hãy chọn một dòng trước khi xóa", FEEDBACK_WARNING);
        return;
    }
    ui.confirmDelete = true;
}

void drawDeleteDialog(AppData& app, UiState& ui)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{245, 247, 246, 255});
    DrawRectangle(0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT, NAV_BG);
    DrawRectangle(SIDEBAR_WIDTH, 0, WINDOW_WIDTH - SIDEBAR_WIDTH, 82, SURFACE);
    DrawRectangle(SIDEBAR_WIDTH, 82, WINDOW_WIDTH - SIDEBAR_WIDTH, WINDOW_HEIGHT - 82, BG);
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{22, 42, 32, 105});
    Rectangle panel = { 420, 292, 600, 292 };
    DrawRectangleRounded(panel, 0.05f, 10, SURFACE);
    DrawRectangleRoundedLines(panel, 0.05f, 10, LINE);
    label("Xác nhận xóa dữ liệu", 470, 334, (int)UITheme::FontSectionTitle, INK, FONT_SEMIBOLD);
    string target = "bản ghi đang chọn";
    try {
        if (ui.page == PAGE_EMPLOYEES) target = "nhân viên " + app.employees[ui.selectedRow]->get_id() + " · " + app.employees[ui.selectedRow]->get_name();
        else if (ui.page == PAGE_VEHICLES) target = "phương tiện " + app.vehicles[ui.selectedRow].getVehicle_id();
        else if (ui.page == PAGE_CUSTOMERS) target = "khách hàng " + app.customers[ui.selectedRow].getCustomer_id();
        else if (ui.page == PAGE_INVOICES) target = "hóa đơn " + app.invoices[ui.selectedRow].getInvoice_id();
        else if (ui.page == PAGE_EXPENSES) target = "khoản chi " + app.expenses[ui.selectedRow].getExpense_id();
        else if (ui.page == PAGE_ASSIGNMENTS) target = "Assignment " + app.assignments[ui.selectedRow].getAssignment_id();
    } catch (...) { target = "bản ghi đang chọn"; }
    label(fitText("Bạn có chắc chắn muốn xóa " + target + "?", 540, 17), 450, 386, 17, INK, FONT_REGULAR);
    label("Hệ thống vẫn áp dụng các ràng buộc nghiệp vụ khi xóa.", 450, 420, 16, MUTED, FONT_REGULAR);
    if (button({ 548, 514, 142, 44 }, "Hủy")) {
        ui.confirmDelete = false;
        setMessage(ui, "Đã hủy thao tác xóa", FEEDBACK_INFO);
    }
    if (button({ 708, 514, 184, 44 }, "Xóa bản ghi", false, true)) {
        ui.confirmDelete = false;
        deleteSelected(app, ui);
    }
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
        if (button({x, y, w, UITheme::ButtonHeight}, "Thêm mới", true)) openForm(ui, kind);
        if (button({x + w + gap, y, w, UITheme::ButtonHeight}, "Sửa dòng chọn")) openEditForm(app, ui);
        if (button({x + 2 * (w + gap), y, w, UITheme::ButtonHeight}, "Xóa dòng chọn", false, true)) requestDelete(ui);
        if (button({x + 3 * (w + gap), y, 132, UITheme::ButtonHeight}, "Làm mới")) {
            ui.search.clear(); ui.selectedRow = -1; ui.scroll = 0;
            setMessage(ui, "Danh sách đã được làm mới", FEEDBACK_INFO);
        }
    } else if (ui.page == PAGE_ASSIGNMENTS) {
        const float aw = 126.0f, ag = 8.0f;
        if (button({x, y, aw, UITheme::ButtonHeight}, "Tạo phân công", true)) openForm(ui, FORM_ASSIGNMENT);
        string selectedId;
        if (ui.selectedRow >= 0 && ui.selectedRow < app.assignments.getSize()) selectedId = app.assignments[ui.selectedRow].getAssignment_id();
        if (button({x + (aw + ag), y, aw, UITheme::ButtonHeight}, "Sửa lịch")) {
            if (selectedId.empty()) setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
            else openEditForm(app, ui);
        }
        if (button({x + 2 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Thêm nhân viên")) {
            if (selectedId.empty()) setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
            else openForm(ui, FORM_ADD_MEMBER, selectedId);
        }
        if (button({x + 3 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Gán xe")) {
            if (selectedId.empty()) setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
            else openForm(ui, FORM_ASSIGN_VEHICLE, selectedId);
        }
        if (button({x + 4 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Xác nhận") && !selectedId.empty()) {
            try {
                bool ok = app.work.confirmAssignment(selectedId);
                setMessage(ui, ok ? "Đã xác nhận Assignment" : "Không thể xác nhận: kiểm tra trạng thái Planned và thời điểm bắt đầu ca", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + 4 * (aw + ag), y, aw, UITheme::ButtonHeight}) && selectedId.empty()) {
            setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
        }
        if (button({x + 5 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Hủy phân công") && !selectedId.empty()) {
            bool ok = app.work.cancelAssignment(selectedId);
            setMessage(ui, ok ? "Đã hủy Assignment" : "Không thể hủy: ca có thể đã bắt đầu hoặc kết thúc", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + 5 * (aw + ag), y, aw, UITheme::ButtonHeight}) && selectedId.empty()) {
            setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
        }
        if (button({x + 6 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Hoàn tất ca") && !selectedId.empty()) {
            try {
                bool ok = app.work.completeAssignment(selectedId);
                setMessage(ui, ok ? "Đã hoàn tất ca và chốt chấm công" : "Chưa thể hoàn tất: ca chưa Assigned hoặc chưa kết thúc", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + 6 * (aw + ag), y, aw, UITheme::ButtonHeight}) && selectedId.empty()) {
            setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
        }
        if (button({x + 7 * (aw + ag), y, aw, UITheme::ButtonHeight}, "Xóa", false, true) && !selectedId.empty()) requestDelete(ui);
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + 7 * (aw + ag), y, aw, UITheme::ButtonHeight}) && selectedId.empty()) setMessage(ui, "Hãy chọn Assignment trước", FEEDBACK_WARNING);
    } else if (ui.page == PAGE_INVOICES) {
        if (button({x, y, w, UITheme::ButtonHeight}, "Lập hóa đơn", true)) openForm(ui, FORM_INVOICE);
        if (button({x + w + gap, y, w, UITheme::ButtonHeight}, "Đánh dấu đã thu") && ui.selectedRow >= 0 && ui.selectedRow < app.invoices.getSize()) {
            try {
                bool ok = app.invoices.markAsPaid(app.invoices[ui.selectedRow].getInvoice_id());
                setMessage(ui, ok ? "Đã cập nhật thanh toán" : "Không thể cập nhật trạng thái hóa đơn", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + w + gap, y, w, UITheme::ButtonHeight}) && ui.selectedRow < 0) {
            setMessage(ui, "Hãy chọn hóa đơn trước", FEEDBACK_WARNING);
        }
        if (button({x + 2 * (w + gap), y, w, UITheme::ButtonHeight}, "Xóa hóa đơn", false, true)) requestDelete(ui);
    } else if (ui.page == PAGE_ACCOUNTS) {
        if (button({x, y, w, UITheme::ButtonHeight}, "Thêm tài khoản", true)) openForm(ui, FORM_ACCOUNT);
        vector<UserSummary> users = app.auth.listUsers();
        if (button({x + w + gap, y, w + 35, UITheme::ButtonHeight}, "Vô hiệu hóa") && ui.selectedRow >= 0 && ui.selectedRow < (int)users.size()) {
            try {
                bool ok = app.auth.disableUser(users[ui.selectedRow].user_id);
                setMessage(ui, ok ? "Đã vô hiệu hóa tài khoản" : "Tài khoản đã bị vô hiệu hóa hoặc không thể tắt", ok ? FEEDBACK_SUCCESS : FEEDBACK_ERROR);
            } catch (const exception& e) { setMessage(ui, e.what(), FEEDBACK_ERROR); }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(logicalMouse(), {x + w + gap, y, w + 35, UITheme::ButtonHeight}) && ui.selectedRow < 0) setMessage(ui, "Hãy chọn tài khoản trước", FEEDBACK_WARNING);
    } else if (ui.page == PAGE_ATTENDANCE) {
        if (button({x, y, 190, UITheme::ButtonHeight}, "Ghi / sửa Check-in", true)) {
            if (ui.selectedRow < 0 || ui.selectedRow >= app.attendance.getSize()) setMessage(ui, "Hãy chọn bản ghi chấm công", FEEDBACK_WARNING);
            else {
                const Attendance& record = app.attendance[ui.selectedRow];
                openForm(ui, FORM_MANAGER_CHECKIN);
                ui.formValues[0] = record.getEmployee_id(); ui.formValues[1] = record.getAssignment_id(); ui.formValues[2] = record.getCheck_in();
            }
        }
        if (button({x + 202, y, 190, UITheme::ButtonHeight}, "Ghi / sửa Check-out")) {
            if (ui.selectedRow < 0 || ui.selectedRow >= app.attendance.getSize()) setMessage(ui, "Hãy chọn bản ghi chấm công", FEEDBACK_WARNING);
            else {
                const Attendance& record = app.attendance[ui.selectedRow];
                openForm(ui, FORM_MANAGER_CHECKOUT);
                ui.formValues[0] = record.getEmployee_id(); ui.formValues[1] = record.getAssignment_id(); ui.formValues[2] = record.getCheck_out();
            }
        }
    }
}

void drawLogin(UiState& ui, AuthManager& auth)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, BG);
    if (ui.loginStep == 0) {
        Rectangle panel = { 150, 170, 1140, 560 };
        DrawRectangleRounded(panel, 0.025f, 12, SURFACE);
        DrawRectangleRoundedLines(panel, 0.025f, 12, LINE);
        drawText("HỆ THỐNG QUẢN LÝ THU GOM RÁC THẢI ĐÔ THỊ", panel.x + 48, panel.y + 34, UITheme::FontPageTitle, INK, FONT_BOLD);
        drawText("Chọn vai trò người dùng", panel.x + 48, panel.y + 154, UITheme::FontSectionTitle, INK, FONT_SEMIBOLD);
        drawText("Chọn vai trò để tiếp tục đến bước đăng nhập.", panel.x + 48, panel.y + 190, 17, MUTED, FONT_REGULAR);
        const char* roles[] = { "Quản lý", "Nhân viên", "Khách hàng" };
        const char* hints[] = { "Điều hành toàn bộ hệ thống", "Lịch làm việc và chấm công", "Hóa đơn và dịch vụ" };
        const float cardW = 330, cardH = 210, cardGap = 24;
        const float firstX = (WINDOW_WIDTH - (3 * cardW + 2 * cardGap)) / 2.0f;
        for (int i = 0; i < 3; ++i) {
            Rectangle card = { firstX + i * (cardW + cardGap), panel.y + 264, cardW, cardH };
            const bool hover = CheckCollisionPointRec(logicalMouse(), card);
            const Color border = hover ? UI_GREEN : LINE;
            DrawRectangleRounded(card, 0.07f, 10, hover ? GREEN_PALE : SURFACE);
            DrawRectangleRoundedLines(card, 0.07f, 10, border);
            const float titleWidth = textWidth(roles[i], 24, FONT_SEMIBOLD);
            drawText(roles[i], card.x + (card.width - titleWidth) / 2, card.y + 52, 24, INK, FONT_SEMIBOLD);
            const float hintWidth = textWidth(hints[i], 17, FONT_REGULAR);
            drawText(hints[i], card.x + (card.width - hintWidth) / 2, card.y + 100, 17, MUTED, FONT_REGULAR);
            drawText("Chọn vai trò", card.x + 24, card.y + 154, 17, UI_GREEN, FONT_MEDIUM);
            if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                ui.selectedRole = i; ui.loginStep = 1; ui.loginField = 0; ui.message.clear(); ui.messageTimer = 0;
            }
        }
    } else {
        Rectangle panel = { 390, 105, 660, 690 };
        DrawRectangleRounded(panel, 0.035f, 12, SURFACE);
        DrawRectangleRoundedLines(panel, 0.035f, 12, LINE);
        if (button({panel.x + 24, panel.y + 22, 122, UITheme::ButtonHeight}, "Quay lại")) {
            ui.loginStep = 0; ui.loginField = 0; ui.showPassword = false;
        }
        drawText("Đăng nhập với tư cách", 550, 204, 20, MUTED, FONT_REGULAR);
        const char* roles[] = { "Quản lý", "Nhân viên", "Khách hàng" };
        const string roleName = roles[max(0, min(2, ui.selectedRole))];
        const float roleWidth = textWidth(roleName, 30, FONT_BOLD);
        drawText(roleName, (WINDOW_WIDTH - roleWidth) / 2, 237, 30, NAV_BG, FONT_BOLD);

        drawText("Tài khoản", panel.x + 48, 300, 16, MUTED, FONT_MEDIUM);
        drawText("Mật khẩu", panel.x + 48, 408, 16, MUTED, FONT_MEDIUM);
        Rectangle userBox = { panel.x + 40, 326, panel.width - 80, 56 };
        Rectangle passBox = { panel.x + 40, 434, panel.width - 80, 56 };
        DrawRectangleRounded(userBox, 0.14f, 8, ui.loginField == 0 ? GREEN_PALE : BG);
        DrawRectangleRoundedLines(userBox, 0.14f, 8, ui.loginField == 0 ? UI_GREEN : LINE);
        DrawRectangleRounded(passBox, 0.14f, 8, ui.loginField == 1 ? GREEN_PALE : BG);
        DrawRectangleRoundedLines(passBox, 0.14f, 8, ui.loginField == 1 ? UI_GREEN : LINE);
        drawText(ui.username.empty() ? "Nhập tên đăng nhập" : fitText(ui.username, userBox.width - 28, 18), userBox.x + 16, userBox.y + 18, 18, ui.username.empty() ? MUTED : INK, FONT_REGULAR);
        const string passDisplay = ui.password.empty() ? "Nhập mật khẩu" : ui.showPassword ? ui.password : string(ui.password.size(), '*');
        drawText(fitText(passDisplay, passBox.width - 96, 18), passBox.x + 16, passBox.y + 18, 18, ui.password.empty() ? MUTED : INK, FONT_REGULAR);
        Rectangle reveal = { passBox.x + passBox.width - 76, passBox.y + 8, 62, 40 };
        const string revealText = ui.showPassword ? "Ẩn" : "Hiện";
        const float revealWidth = textWidth(revealText, 16, FONT_MEDIUM);
        drawText(revealText, reveal.x + (reveal.width - revealWidth) / 2, reveal.y + 11, 16, UI_GREEN, FONT_MEDIUM);
        if (CheckCollisionPointRec(logicalMouse(), userBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 0;
        if (CheckCollisionPointRec(logicalMouse(), passBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ui.loginField = 1;
        if (CheckCollisionPointRec(logicalMouse(), reveal) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ui.showPassword = !ui.showPassword;
        if (button({ panel.x + 40, 526, panel.width - 80, 50 }, "Đăng nhập", true) || (IsKeyPressed(KEY_ENTER) && ui.loginField == 1)) {
            if (auth.login(ui.username, ui.password)) {
                const User* user = auth.getCurrentUser();
                const UserRole requested = ui.selectedRole == 0 ? UserRole::Manager : ui.selectedRole == 1 ? UserRole::Employee : UserRole::Customer;
                if (user && user->getRole() == requested) {
                    ui.loginMode = false; ui.page = PAGE_DASHBOARD; ui.message.clear(); ui.loginField = 0;
                    ui.search.clear(); ui.selectedRow = -1; ui.scroll = 0;
                } else {
                    auth.logout(); setMessage(ui, "Tài khoản không thuộc vai trò đã chọn", FEEDBACK_ERROR, 5);
                }
            } else setMessage(ui, "Sai tên đăng nhập, mật khẩu hoặc tài khoản đã bị khóa", FEEDBACK_ERROR, 5);
        }
        if (ui.messageTimer > 0 && !ui.message.empty()) {
            DrawRectangleRounded({panel.x + 40, 620, panel.width - 80, 46}, 0.12f, 8, Color{252, 232, 234, 255});
            drawText(fitText(ui.message, panel.width - 104, 15), panel.x + 54, 634, 15, UI_RED);
        }
        if (ui.loginField == 0 || ui.loginField == 1) {
            int key = GetCharPressed();
            while (key > 0) {
                string& value = ui.loginField == 0 ? ui.username : ui.password;
                appendCodepoint(value, key, 128);
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                string& value = ui.loginField == 0 ? ui.username : ui.password;
                if (!value.empty()) {
                    size_t at = value.size() - 1;
                    while (at > 0 && (static_cast<unsigned char>(value[at]) & 0xC0) == 0x80) --at;
                    value.erase(at);
                }
            }
            if (IsKeyPressed(KEY_TAB)) ui.loginField = 1 - ui.loginField;
        }
    }
}

} // namespace

int main()
{
    try {
        AppData app;
        UiState ui;
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED | FLAG_MSAA_4X_HINT);
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PBL2 | Operations Management");
        SetWindowMinSize(1100, 720);
        loadUIFont();
        SetTargetFPS(60);

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            updateUICamera();
            if (ui.messageTimer > 0) ui.messageTimer -= dt;
            if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
            if (ui.loginMode) {
                BeginDrawing(); ClearBackground(BG); BeginMode2D(UI_CAMERA); drawLogin(ui, app.auth); EndMode2D();
                EndDrawing(); continue;
            }
            if (ui.formKind != FORM_NONE) {
                readForm(ui, app);
                BeginDrawing(); ClearBackground(BG); BeginMode2D(UI_CAMERA); drawForm(app, ui); EndMode2D(); EndDrawing();
                continue;
            }
            readSearch(ui);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Rectangle searchRect = { WINDOW_WIDTH - 365.0f, 98.0f, 335.0f, 40.0f };
                if (!CheckCollisionPointRec(logicalMouse(), searchRect)) ui.loginField = 0;
            }
            float wheel = GetMouseWheelMove();
            if (wheel != 0) ui.scroll = max(0, ui.scroll - (int)wheel);

            if (ui.confirmDelete) {
                BeginDrawing(); ClearBackground(BG); BeginMode2D(UI_CAMERA);
                drawDeleteDialog(app, ui);
                EndMode2D(); EndDrawing();
                continue;
            }

            BeginDrawing();
            ClearBackground(BG);
            BeginMode2D(UI_CAMERA);
            drawSidebar(ui, app);
            drawHeader(ui, app);
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
                case PAGE_PROFIT: drawProfitReport(app); break;
            }
            drawActions(app, ui);
            if (ui.messageTimer > 0 && !ui.message.empty()) {
                const Color color = feedbackColor(ui.messageKind);
                DrawRectangleRounded({ (float)SIDEBAR_WIDTH + 30, (float)WINDOW_HEIGHT - 58, (float)WINDOW_WIDTH - SIDEBAR_WIDTH - 180, 40 }, 0.16f, 6, Color{color.r, color.g, color.b, 32});
                label(fitText(ui.message, WINDOW_WIDTH - SIDEBAR_WIDTH - 220, UITheme::FontCaption), SIDEBAR_WIDTH + 46, WINDOW_HEIGHT - 46, (int)UITheme::FontCaption, color, FONT_MEDIUM);
            }
            EndMode2D();
            EndDrawing();
        }
        unloadUIFont();
        CloseWindow();
    } catch (const exception& e) {
        cerr << "PBL2 GUI startup error: " << e.what() << endl;
        InitWindow(900, 300, "PBL2 - Loi khoi dong");
        loadUIFont();
        SetTargetFPS(30);
        while (!WindowShouldClose()) {
            BeginDrawing(); ClearBackground(Color{250,244,244,255});
            drawText("Không thể khởi động dữ liệu", 30, 30, 24, UI_RED);
            drawText(e.what(), 30, 80, 16, INK);
            drawText("Kiểm tra dữ liệu trong thư mục data/", 30, 130, 16, MUTED);
            EndDrawing();
        }
        unloadUIFont();
        CloseWindow();
        return 1;
    }
    return 0;
}
