#include <iostream>
#include <stdexcept>
#include "Objects/Invoice.h"

using namespace std;

struct TestResult {
    string name;
    bool passed;
};

void print(const TestResult& r) {
    cout << (r.passed ? "PASS" : "FAIL") << " : " << r.name << endl;
}

int main() {
    int total = 0, passed = 0;
    vector<TestResult> results;

    // 1. Khởi tạo hợp lệ (Unpaid mặc định)
    ++total;
    try {
        Invoice inv("HD001", "KH001", 25000, "2026-11-06", "Unpaid", "2026-11");
        results.push_back({"Create valid invoice (Unpaid)", true});
        ++passed;
    } catch (...) { results.push_back({"Create valid invoice (Unpaid)", false}); }

    // 2. Khởi tạo hợp lệ (Paid khi khôi phục)
    ++total;
    try {
        Invoice inv("HD002", "KH002", 30000, "2026-10-01", "Paid", "2026-10");
        results.push_back({"Create valid invoice (Paid)", true});
        ++passed;
    } catch (...) { results.push_back({"Create valid invoice (Paid)", false}); }

    // 3. Getter hoạt động
    ++total;
    try {
        Invoice inv("HD003", "KH003", 40000, "2026-09-15", "Unpaid", "2026-09");
        bool ok = inv.getInvoice_id() == "HD003" && inv.getCustomer_id() == "KH003" &&
                  inv.getTotal_amount() == 40000 && inv.getInvoice_date() == "2026-09-15" &&
                  inv.getInvoice_status() == "Unpaid" && inv.getBilling_month() == "2026-09";
        results.push_back({"Getters return correct values", ok});
        if (ok) ++passed;
    } catch (...) { results.push_back({"Getters return correct values", false}); }

    // 4. Setter cập nhật hợp lệ
    ++total;
    try {
        Invoice inv;
        inv.setInvoice_id("HD004");
        inv.setCustomer_id("KH004");
        inv.setTotal_amount(50000);
        inv.setInvoice_date("2026-08-20");
        inv.setInvoice_status("Unpaid");
        inv.setBilling_month("2026-08");
        bool ok = inv.getInvoice_id() == "HD004" && inv.getCustomer_id() == "KH004" &&
                  inv.getTotal_amount() == 50000 && inv.getInvoice_date() == "2026-08-20" &&
                  inv.getInvoice_status() == "Unpaid" && inv.getBilling_month() == "2026-08";
        results.push_back({"Setters accept valid data", ok});
        if (ok) ++passed;
    } catch (...) { results.push_back({"Setters accept valid data", false}); }

    // 5. Trạng thái khác Paid/Unpaid bị từ chối
    ++total;
    try { Invoice inv; inv.setInvoice_status("Done"); results.push_back({"Invalid status rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Invalid status rejected", true}); ++passed; }
    catch (...) { results.push_back({"Invalid status rejected", false}); }

    // 6. Ngày sai định dạng
    ++total;
    try { Invoice inv; inv.setInvoice_date("202611-06"); results.push_back({"Invalid date format rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Invalid date format rejected", true}); ++passed; }
    catch (...) { results.push_back({"Invalid date format rejected", false}); }

    // 7. Ngày không tồn tại (2026-02-30)
    ++total;
    try { Invoice inv; inv.setInvoice_date("2026-02-30"); results.push_back({"Non‑existent date rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Non‑existent date rejected", true}); ++passed; }
    catch (...) { results.push_back({"Non‑existent date rejected", false}); }

    // 8. Ngày nhuận hợp lệ (2028-02-29)
    ++total;
    try { Invoice inv; inv.setInvoice_date("2028-02-29"); results.push_back({"Leap year date accepted", true}); ++passed; }
    catch (...) { results.push_back({"Leap year date accepted", false}); }

    // 9. Billing month sai định dạng
    ++total;
    try { Invoice inv; inv.setBilling_month("202611"); results.push_back({"Invalid billing month format rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Invalid billing month format rejected", true}); ++passed; }
    catch (...) { results.push_back({"Invalid billing month format rejected", false}); }

    // 10. Tổng tiền bằng 0 bị từ chối
    ++total;
    try { Invoice inv; inv.setTotal_amount(0); results.push_back({"Zero amount rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Zero amount rejected", true}); ++passed; }
    catch (...) { results.push_back({"Zero amount rejected", false}); }

    // 11. ID rỗng bị từ chối
    ++total;
    try { Invoice inv; inv.setInvoice_id(""); results.push_back({"Empty invoice_id rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Empty invoice_id rejected", true}); ++passed; }
    catch (...) { results.push_back({"Empty invoice_id rejected", false}); }

    // 12. Ký tự phân cách | trong dữ liệu bị từ chối
    ++total;
    try { Invoice inv; inv.setCustomer_id("KH|001"); results.push_back({"Invalid char in customer_id rejected", false}); }
    catch (const std::invalid_argument&) { results.push_back({"Invalid char in customer_id rejected", true}); ++passed; }
    catch (...) { results.push_back({"Invalid char in customer_id rejected", false}); }

    // 13. Đối tượng mặc định có thể gán từ đối tượng hợp lệ (mô phỏng mảng động)
    ++total;
    try {
        Invoice src("HD005", "KH005", 60000, "2026-07-01", "Unpaid", "2026-07");
        Invoice dest; // default
        dest = src;   // copy assign
        bool ok = dest.getInvoice_id() == "HD005" && dest.getCustomer_id() == "KH005" && dest.getTotal_amount() == 60000;
        results.push_back({"Default object assignable", ok});
        if (ok) ++passed;
    } catch (...) { results.push_back({"Default object assignable", false}); }

    // 14. Show() không làm thay đổi dữ liệu
    ++total;
    try {
        Invoice inv("HD006", "KH006", 70000, "2026-06-15", "Paid", "2026-06");
        string before = inv.getInvoice_id();
        inv.Show(); // output to console
        string after = inv.getInvoice_id();
        bool ok = before == after;
        results.push_back({"Show() does not modify", ok});
        if (ok) ++passed;
    } catch (...) { results.push_back({"Show() does not modify", false}); }

    // Tổng kết
    for (auto& r : results) print(r);
    cout << "\n=== SUMMARY ===\n";
    cout << "Passed " << passed << " / " << total << " tests." << endl;
    return (passed == total) ? 0 : 1;
}
