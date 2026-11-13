# KẾ HOẠCH TRIỂN KHAI MODULE HÓA ĐƠN (NGƯỜI A)

> **Quy tắc giao tiếp dành cho Antigravity:** Từ thời điểm này, hãy **trả lời, giải thích, lập kế hoạch, nhận xét code và báo cáo kiểm thử hoàn toàn bằng tiếng Việt**. Giữ nguyên tên file, tên lớp, tên hàm, biến, cú pháp C++ và các giá trị quy ước (`Paid`, `Unpaid`, `Household`, `Business`, ...). Không tự ý đổi tên API hoặc dịch mã nguồn sang tiếng Việt. Nếu cần dùng thuật ngữ tiếng Anh, hãy kèm giải thích ngắn bằng tiếng Việt.

> **Lưu ý phạm vi:** Đây là kế hoạch của **Người A**, phụ trách `Invoice`, `InvoiceManager`, `FeePolicy`, `TemplateUtils`. **Chưa yêu cầu chỉnh sửa source ngay**. Đầu tiên hãy khảo sát repository và xác nhận kế hoạch trước khi bắt đầu code. Không chỉnh sửa module `Customer`/`CustomerManager` của Người B, `FileManager` dùng chung, hoặc `main.cpp` nếu chưa có yêu cầu rõ ràng.

> **Cơ sở đánh giá:** Kế hoạch được Việt hóa và điều chỉnh theo source ZIP PBL2 đã cung cấp trong cuộc trò chuyện. Antigravity phải kiểm tra lại các file **trong workspace hiện tại** vì source có thể đã thay đổi.

---

## 1. Tổng quan cấu trúc hiện tại

| Thành phần | Đường dẫn trong source | Hiện trạng trong bản ZIP được kiểm tra | Việc cần làm |
|---|---|---|---|
| Lớp `Invoice` | `Objects/Invoice.h` | Có khai báo `invoice_id`, `customer_id`, `amount`, `issue_date`, `payment_status`; thiếu `billing_month` | Cập nhật giao diện theo bản chốt |
| Cài đặt `Invoice` | `Objects/Invoice.cpp` | File có sẵn nhưng rỗng | Viết constructor, getter/setter, `Show()` và kiểm tra dữ liệu |
| `InvoiceManager` | `Managers/InvoiceManager.h` | Có `Invoice* arr`, `size`, `capacity`; các hàm CRUD, tìm kiếm, nạp chồng toán tử, `calculateTotalRevenue()` | Giữ mảng động, bổ sung nghiệp vụ tháng thu và doanh thu thực thu |
| Cài đặt `InvoiceManager` | `Managers/InvoiceManager.cpp` | File có sẵn nhưng rỗng | Triển khai các hàm đã khai báo và các API mới |
| `Customer` | `Objects/Customer.h` | Chưa có `service_area`; còn `calculate_fee()` thuần ảo | **Người B phụ trách; không tự sửa** |
| `CustomerManager` | `Managers/CustomerManager.h` | Sử dụng `Customer**`; có tìm kiếm theo ID | **Người B phụ trách; chỉ đọc để thống nhất giao diện** |
| `FileManager` | `FileIO/FileManager.h` và `FileIO/FileManager.cpp` | **Cả hai file đã tồn tại**, đã dùng Singleton | Tái sử dụng API `getInstance()`, `readFile()`, `writeFile()`, `appendLine()`, `fileExists()` |
| TXT | `data/*.txt` | Có thư mục `data/`; `invoice.txt` đang rỗng trong ZIP | Cập nhật định dạng mới và bổ sung `fee.txt` |
| Chương trình chính | `main.cpp` | Rỗng trong ZIP | Không sửa; dùng demo/test riêng |

**Chỉnh sửa so với kế hoạch Antigravity cũ:** Không tạo lại `FileIO/FileManager.cpp` vì file này đã có trong source ZIP. Không đưa `Customer.cpp` hay `CustomerManager.cpp` vào danh sách file Người A được chỉnh sửa.

---

## 2. Những file Người A cần chỉnh sửa và tạo mới

### 2.1. File cần chỉnh sửa

| File | Công việc cụ thể |
|---|---|
| `Objects/Invoice.h` | Bổ sung `billing_month`; thống nhất tên/kiểu thuộc tính, constructor, getter/setter; giữ tương thích API hợp lý |
| `Objects/Invoice.cpp` | Cài đặt các phương thức, chuẩn hóa `Paid`/`Unpaid`, hiển thị thông tin |
| `Managers/InvoiceManager.h` | Bổ sung kiểm tra trùng tháng, kiểm tra hóa đơn tham chiếu khách hàng và hai hàm thống kê |
| `Managers/InvoiceManager.cpp` | Cài đặt quản lý mảng động, CRUD, tìm kiếm, thanh toán, thống kê, nạp/ghi TXT và nạp chồng toán tử |
| `data/invoice.txt` | Dùng định dạng TXT mới có `billing_month`; tránh ghi đè dữ liệu hiện có khi chưa kiểm tra |

### 2.2. File cần tạo mới

| File | Mục đích |
|---|---|
| `Policies/FeePolicy.h` và `Policies/FeePolicy.cpp` | Tra cứu phí hằng tháng dựa trên `customer_type` và `service_area` |
| `Utils/TemplateUtils.h` | Định nghĩa **Function Template `findIf`** dùng chung giữa các Manager. **Không tạo `TemplateUtils.cpp`** vì định nghĩa template nằm trong header |
| `data/fee.txt` | Bảng giá dịch vụ; đọc qua `FileManager` |
| `demoInvoice.cpp` hoặc thư mục test riêng | Biên dịch và kiểm thử module hóa đơn độc lập, không động đến `main.cpp` |

Có thể đề xuất vị trí thư mục khác nếu cấu trúc workspace hiện tại khác, nhưng phải trình bày trước và giữ cách tổ chức nhất quán.

---

## 3. Quy tắc nghiệp vụ đã thống nhất

1. **Quan hệ dữ liệu:** Một `Customer` có nhiều `Invoice`; `Invoice` chỉ giữ `customer_id`, không lưu con trỏ `Customer*`.
2. **Tách trách nhiệm:** `Controller/Menu` kiểm tra khách hàng tồn tại thông qua `CustomerManager`, đọc loại/phân loại dịch vụ, gọi `FeePolicy` để lấy giá rồi chuyển dữ liệu đã xác minh cho `InvoiceManager`. `InvoiceManager` **không tự truy cập `CustomerManager` hoặc tự tính phí**.
3. **Cấu trúc dữ liệu:** `InvoiceManager` giữ `Invoice* arr`, `size`, `capacity`, có quản lý bộ nhớ an toàn. Không tự đổi sang `std::vector` trong Manager nếu chưa thống nhất với nhóm.
4. **Chu kỳ:** Mỗi hóa đơn tương ứng một tháng thu phí `billing_month` định dạng `YYYY-MM`; ngày lập `invoice_date` định dạng `YYYY-MM-DD`.
5. **Chống trùng:** Một khách hàng không thể có hai hóa đơn trong cùng một `billing_month`.
6. **Trạng thái:** Chỉ dùng `Unpaid` (mặc định) và `Paid`; thanh toán chỉ chuyển `Unpaid` sang `Paid`. Không có công nợ hoặc thanh toán một phần.
7. **Bảo toàn giá:** `Invoice` lưu `total_amount` tại thời điểm lập; thay đổi `fee.txt` về sau không làm thay đổi hóa đơn đã phát hành.
8. **Thống kê:** `calculateTotalInvoiced()` cộng mọi hóa đơn; `calculateCollectedRevenue()` chỉ cộng hóa đơn `Paid`.
9. **Kiểu tiền:** Ưu tiên `long long` biểu diễn VND.
10. **Xóa khách hàng:** Controller hỏi `InvoiceManager::hasInvoicesForCustomer(customerId)`; nếu có hóa đơn liên quan thì không cho xóa khách hàng.

### 3.1. Bảng giá phí hằng tháng

`data/fee.txt` dùng dấu `|` làm dấu phân cách (không phải CSV phân cách dấu phẩy):

```text
Household|Area1|25000
Household|Area2|30000
Business|Type1|165000
Business|Type2|265000
```

- `Household/Area1`: hộ gia đình trong hẻm, kiệt — 25.000đ/tháng.
- `Household/Area2`: hộ gia đình mặt tiền đường phố — 30.000đ/tháng.
- `Business/Type1`: văn phòng, trường học, cơ quan hành chính — 165.000đ/tháng.
- `Business/Type2`: sản xuất, kinh doanh, chợ… — 265.000đ/tháng.

`FeePolicy` phải báo lỗi rõ khi thiếu bảng giá, khóa bị trùng, dòng không hợp lệ hoặc không tra được phí. Không hard-code giá làm fallback trong `InvoiceManager`.

### 3.2. Định dạng hóa đơn

`data/invoice.txt` lưu các trường theo **đúng thứ tự**:

```text
invoice_id|customer_id|invoice_date|invoice_status|total_amount|billing_month
```

Ví dụ bản ghi (không mặc định rằng parser phải có dòng header):

```text
HD001|KH001|2026-11-06|Unpaid|25000|2026-11
HD002|KH002|2026-11-06|Paid|165000|2026-11
```

Phải chốt cách có/không có dòng tiêu đề và nhất quán giữa dữ liệu mẫu, đọc và ghi. Không được âm thầm chuyển dữ liệu 5 trường cũ sang 6 trường khi chưa thống nhất cách bổ sung `billing_month`.

---

## 4. Những điểm kỹ thuật và tích hợp cần lưu ý

1. **Mảng động và bộ nhớ:** `InvoiceManager` hiện dùng `Invoice* arr` (mảng đối tượng, không phải mảng con trỏ `Invoice**`). Cần cấp phát, mở rộng dung lượng, copy constructor, toán tử gán và destructor đúng để tránh rò rỉ bộ nhớ.
2. **API hiện có:** Rà soát tên cũ `amount`, `issue_date`, `payment_status`, `calculateTotalRevenue()`, `generateInvoiceFromCustomer(...)`; nếu đổi tên hoặc chữ ký hàm, phải liệt kê tương thích và tác động rõ ràng.
3. **Function Template:** `TemplateUtils.h` chứa `findIf`, được `InvoiceManager::findInvoiceById()` sử dụng; có giao diện tái sử dụng được cho `CustomerManager` sau này. Không dùng template vào mục đích định dạng hóa đơn.
4. **FileManager dùng chung:** Chỉ dùng các hàm đã được định nghĩa thật trong source; không tạo Singleton thứ hai. Trước khi ghi đè file, phải phân biệt trường hợp dữ liệu trống hợp lệ với load thất bại; không tự ghi đè dữ liệu gốc khi có lỗi parse/đọc file.
5. **Exception:** `bool` hoặc `nullptr` cho trường hợp nghiệp vụ thông thường; dùng exception cho lỗi file nghiêm trọng, dữ liệu hỏng, truy cập chỉ số sai. Tầng Controller/Menu chịu trách nhiệm bắt và thông báo lỗi khi tích hợp.
6. **Giới hạn sửa file:** Không thêm stub hay thay đổi trong `Customer.cpp`, `CustomerManager.cpp` chỉ để dự án biên dịch. Nếu thiếu giao diện liên quan, ghi rõ việc Người B cần bổ sung.
7. **Build:** Kiểm tra compiler/build system có thật; nếu chưa có CMake hoặc project file, không giả định chúng tồn tại. Chạy riêng demo/test bằng `g++ -std=c++17` và chỉ rõ các file được biên dịch.
8. **Git:** Kiểm tra `git status`/`git diff` trước và sau khi chỉnh sửa; không commit, push, đổi nhánh hoặc xóa thay đổi của thành viên khác khi chưa được yêu cầu.

---

## 5. Thứ tự triển khai đề xuất

1. **Khảo sát workspace và thống nhất API:** Đọc `Invoice.h`, `InvoiceManager.h`, `Customer.h`, `CustomerManager.h`, `FileManager.h/.cpp`, xác định những khác biệt với bản chốt.
2. **Tạo `FeePolicy.h/.cpp` và `data/fee.txt`:** Xây dựng tra cứu phí độc lập, kiểm tra đủ bốn trường hợp và lỗi bảng giá.
3. **Cập nhật `Objects/Invoice.h/.cpp`:** Hoàn thành lớp thực thể, `billing_month`, các getter/setter và `Show()`.
4. **Tạo `Utils/TemplateUtils.h`:** Viết `findIf` và sử dụng cho tìm kiếm hóa đơn.
5. **Cập nhật `Managers/InvoiceManager.h/.cpp`:** CRUD, kiểm tra trùng theo tháng, thanh toán, thống kê, copy/destructor và TXT.
6. **Tạo demo/test riêng:** Chạy các trường hợp hợp lệ, không hợp lệ, lưu/nạp lại dữ liệu và lỗi file mà không sửa `main.cpp`.
7. **Đề xuất điểm tích hợp với Người B:** Liệt kê tên hàm, tham số và nhiệm vụ của Controller, nhưng không sửa code Customer của Người B.

### Bộ kiểm thử tối thiểu

- Tra đúng bốn mức phí; thử thiếu phí/trùng khóa/dữ liệu sai.
- Thêm hóa đơn hợp lệ và từ chối `invoice_id` trùng.
- Từ chối hai hóa đơn cùng `customer_id` + `billing_month`; cho phép khác tháng.
- Chuyển `Unpaid` sang `Paid`; không tính tiền lần hai khi thanh toán lặp.
- Thống kê tổng giá trị hóa đơn và doanh thu đã thu chính xác.
- Lưu TXT, mở lại chương trình và giữ nguyên dữ liệu.
- Không ghi đè dữ liệu cũ nếu load hoặc parse thất bại.
- Kiểm thử copy constructor, toán tử gán, toán tử `[]` và quản lý bộ nhớ.
- Biên dịch demo với C++17 trên source hiện có.

---

## 6. Cách Antigravity phải báo cáo kết quả

**Toàn bộ báo cáo bằng tiếng Việt**, theo cấu trúc sau:

1. **Khảo sát:** Đã đọc những file nào, hiện trạng thực tế ra sao, những điểm khác so với kế hoạch.
2. **Kế hoạch:** Các file sẽ sửa/tạo và mục đích của từng file.
3. **Triển khai:** Khi đã được phép, nêu ngắn gọn thay đổi đã thực hiện, không làm sang phần Người B.
4. **Kiểm thử:** Lệnh biên dịch, kết quả chạy, trường hợp đạt/chưa đạt, lỗi còn tồn tại.
5. **Tích hợp:** Những API cần nhóm/Người B thống nhất để kết nối Customer–Invoice.
6. **Git:** Danh sách file thay đổi; xác nhận chưa commit/push nếu chưa được yêu cầu.

**Hãy trả lời bằng tiếng Việt ngay từ tin nhắn tiếp theo.**
