
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <algorithm>
#include <climits>
#include <filesystem>
#include <fstream>
#include <functional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <initializer_list>

// =====================================================================
// 1. HỢP ĐỒNG DỮ LIỆU 
// =====================================================================
struct Process { std::string pid; int arrival_time; int burst_time; };
struct Segment { enum Type { PROCESS, IDLE } type; std::string pid; int start_time, end_time; };
struct ProcessResult {
    std::string pid;
    int arrival_time, burst_time, start_time, completion_time, waiting_time, turnaround_time, response_time;
};
struct Averages { double average_waiting_time = 0, average_turnaround_time = 0, average_response_time = 0; };
struct ScheduleResult {
    std::string algorithm;
    std::vector<Segment> segments;
    std::vector<ProcessResult> processes;
    Averages averages;
};

// =====================================================================
// 2. THUẬT TOÁN FCFS VÀ SJF (cùng trả về ScheduleResult cho GUI).
// =====================================================================
static ScheduleResult run_fcfs(const std::vector<Process>& ps) {
    if (ps.empty()) throw std::invalid_argument("Chưa có tiến trình để lập lịch.");
    ScheduleResult result;
    result.algorithm = "FCFS";

    // Sắp theo arrival; stable_sort giữ thứ tự CSV khi arrival bằng nhau.
    std::vector<size_t> order(ps.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](size_t left, size_t right) {
        return ps[left].arrival_time < ps[right].arrival_time;
    });

    std::vector<ProcessResult> byInputOrder(ps.size());
    long long currentTime = 0;
    long double totalWaiting = 0, totalTurnaround = 0, totalResponse = 0;
    for (size_t index : order) {
        const Process& p = ps[index];
        if (currentTime < p.arrival_time) {
            result.segments.push_back({Segment::IDLE, "", (int)currentTime, p.arrival_time});
            currentTime = p.arrival_time;
        }
        const long long start = currentTime;
        const long long completion = start + p.burst_time;
        if (start > INT_MAX || completion > INT_MAX)
            throw std::overflow_error("Tổng thời gian mô phỏng vượt giới hạn; hãy dùng dữ liệu nhỏ hơn.");

        const int waiting = (int)(start - p.arrival_time);
        const int turnaround = (int)(completion - p.arrival_time);
        const int response = waiting; // CPU burst không bị ngắt nên response = waiting.
        result.segments.push_back({Segment::PROCESS, p.pid, (int)start, (int)completion});
        byInputOrder[index] = {p.pid, p.arrival_time, p.burst_time, (int)start,
                               (int)completion, waiting, turnaround, response};
        totalWaiting += waiting;
        totalTurnaround += turnaround;
        totalResponse += response;
        currentTime = completion;
    }

    // Bảng kết quả giữ thứ tự dòng CSV để dễ đối chiếu với dữ liệu nhập.
    result.processes = std::move(byInputOrder);
    const long double count = static_cast<long double>(result.processes.size());
    result.averages = {(double)(totalWaiting / count),
                       (double)(totalTurnaround / count),
                       (double)(totalResponse / count)};
    return result;
}

// SJF không độc quyền: mỗi lần CPU rảnh, chọn tiến trình đã đến
// có burst nhỏ nhất; tie theo arrival time rồi thứ tự dòng input.
static ScheduleResult run_sjf(const std::vector<Process>& ps) {
    if (ps.empty()) throw std::invalid_argument("Chưa có tiến trình để lập lịch.");

    ScheduleResult result;
    result.algorithm = "SJF";
    std::vector<bool> finished(ps.size(), false);
    std::vector<ProcessResult> byInputOrder(ps.size());
    size_t completedCount = 0;
    long long currentTime = 0;
    long double totalWaiting = 0, totalTurnaround = 0, totalResponse = 0;

    while (completedCount < ps.size()) {
        int selected = -1;

        // Chỉ xét tiến trình đã đến; chỉ số vector là thứ tự dòng CSV.
        for (size_t i = 0; i < ps.size(); ++i) {
            if (finished[i] || ps[i].arrival_time > currentTime) continue;
            if (selected < 0) {
                selected = static_cast<int>(i);
                continue;
            }

            const Process& candidate = ps[i];
            const Process& currentBest = ps[static_cast<size_t>(selected)];
            const bool better =
                candidate.burst_time < currentBest.burst_time ||
                (candidate.burst_time == currentBest.burst_time &&
                 candidate.arrival_time < currentBest.arrival_time) ||
                (candidate.burst_time == currentBest.burst_time &&
                 candidate.arrival_time == currentBest.arrival_time &&
                 i < static_cast<size_t>(selected));
            if (better) selected = static_cast<int>(i);
        }

        if (selected < 0) {
            // Không có tiến trình sẵn sàng: CPU idle đến arrival gần nhất còn lại.
            bool foundArrival = false;
            int nextArrival = INT_MAX;
            for (size_t i = 0; i < ps.size(); ++i) {
                if (!finished[i] && (!foundArrival || ps[i].arrival_time < nextArrival)) {
                    nextArrival = ps[i].arrival_time;
                    foundArrival = true;
                }
            }
            if (!foundArrival)
                throw std::logic_error("Không tìm được tiến trình tiếp theo.");
            if (currentTime > INT_MAX)
                throw std::overflow_error("Tổng thời gian mô phỏng vượt giới hạn kiểu int.");
            if (currentTime < nextArrival) {
                result.segments.push_back({Segment::IDLE, "", static_cast<int>(currentTime), nextArrival});
                currentTime = nextArrival;
            }
            continue;
        }

        const size_t index = static_cast<size_t>(selected);
        const Process& p = ps[index];
        const long long start = currentTime;
        const long long completion = start + p.burst_time;
        if (start > INT_MAX || completion > INT_MAX)
            throw std::overflow_error("Tổng thời gian mô phỏng vượt giới hạn; hãy dùng dữ liệu nhỏ hơn.");

        const int waiting = static_cast<int>(start - p.arrival_time);
        const int turnaround = static_cast<int>(completion - p.arrival_time);
        const int response = waiting; // Một CPU burst, không bị ngắt.
        result.segments.push_back({Segment::PROCESS, p.pid,
                                   static_cast<int>(start), static_cast<int>(completion)});
        byInputOrder[index] = {p.pid, p.arrival_time, p.burst_time,
                               static_cast<int>(start), static_cast<int>(completion),
                               waiting, turnaround, response};
        totalWaiting += waiting;
        totalTurnaround += turnaround;
        totalResponse += response;
        currentTime = completion; // Chạy đến hết rồi mới chọn tiến trình kế tiếp.
        finished[index] = true;
        ++completedCount;
    }

    result.processes = std::move(byInputOrder); // Trả bảng theo thứ tự input/CSV.
    const long double count = static_cast<long double>(result.processes.size());
    result.averages = {static_cast<double>(totalWaiting / count),
                       static_cast<double>(totalTurnaround / count),
                       static_cast<double>(totalResponse / count)};
    return result;
}

// =====================================================================
// 3. TIỆN ÍCH + KIỂM TRA DỮ LIỆU NHẬP
// =====================================================================
static std::wstring W(const std::string& s) {           // UTF-8 -> wstring
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n - 1, L'\0'); MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n); return w;
}
static std::string U(const std::wstring& w) {           // wstring -> UTF-8
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0'); WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr); return s;
}
static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n"), e = s.find_last_not_of(" \t\r\n");
    return b == std::string::npos ? "" : s.substr(b, e - b + 1);
}
static bool parseInt(const std::string& s, int& out) {
    std::string t = trim(s); if (t.empty()) return false;
    try { size_t pos; long v = std::stol(t, &pos); if (pos != t.size()) return false; out = (int)v; return true; }
    catch (...) { return false; }
}
struct RawRow { std::string pid, arrival, burst; };
static std::vector<std::string> validate(const std::vector<RawRow>& rows, std::vector<Process>& procs) {
    std::vector<std::string> err; procs.clear();
    if (rows.empty()) { err.push_back("Chưa có tiến trình nào."); return err; }
    std::set<std::string> seen;
    for (size_t i = 0; i < rows.size(); ++i) {
        std::string n = "Dòng " + std::to_string(i + 1), pid = trim(rows[i].pid);
        if (pid.empty()) { err.push_back(n + ": pid rỗng."); continue; }
        if (!seen.insert(pid).second) err.push_back(n + ": pid '" + pid + "' bị trùng.");
        int a, b;
        if (!parseInt(rows[i].arrival, a) || a < 0) { err.push_back(n + " (" + pid + "): arrival_time phải là số nguyên >= 0."); continue; }
        if (!parseInt(rows[i].burst, b) || b <= 0) { err.push_back(n + " (" + pid + "): burst_time phải là số nguyên > 0."); continue; }
        procs.push_back({pid, a, b});
    }
    return err;
}

// =====================================================================
// 4. GUI
// =====================================================================
enum { ID_PID = 101, ID_AT, ID_BT, ID_ADD, ID_DEL, ID_CLR, ID_OPEN, ID_ALGO, ID_RUN, ID_EXPORT, ID_INLIST, ID_RESLIST };

static HWND hPid, hAt, hBt, hInList, hResList, hAlgo;
static ScheduleResult g_result; static bool g_has = false;
static const RECT kGantt = {10, 215, 975, 335};   // vùng vẽ Gantt

static HWND mk(HWND p, const wchar_t* cls, const wchar_t* txt, DWORD style, int x, int y, int w, int h, int id = 0, DWORD ex = 0) {
    HWND c = CreateWindowExW(ex, cls, txt, WS_CHILD | WS_VISIBLE | style, x, y, w, h, p, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
    SendMessageW(c, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return c;
}
static void addCols(HWND lv, const std::vector<const wchar_t*>& names, int width) {
    for (int i = 0; i < (int)names.size(); ++i) {
        LVCOLUMNW c{}; c.mask = LVCF_TEXT | LVCF_WIDTH; c.pszText = (LPWSTR)names[i]; c.cx = width;
        SendMessageW(lv, LVM_INSERTCOLUMNW, i, (LPARAM)&c);
    }
}
static void addRow(HWND lv, const std::vector<std::wstring>& cells) {
    LVITEMW it{}; it.mask = LVIF_TEXT; it.iItem = ListView_GetItemCount(lv); it.pszText = (LPWSTR)cells[0].c_str();
    int r = (int)SendMessageW(lv, LVM_INSERTITEMW, 0, (LPARAM)&it);
    for (int c = 1; c < (int)cells.size(); ++c) {
        LVITEMW s{}; s.iSubItem = c; s.pszText = (LPWSTR)cells[c].c_str();
        SendMessageW(lv, LVM_SETITEMTEXTW, r, (LPARAM)&s);
    }
}
static std::string cellText(HWND lv, int row, int col) {
    wchar_t buf[256] = L""; ListView_GetItemText(lv, row, col, buf, 256); return U(buf);
}
static std::string editText(HWND e) { wchar_t b[256]; GetWindowTextW(e, b, 256); return U(b); }
static void msg(HWND h, const std::string& t, const wchar_t* title, UINT icon) { MessageBoxW(h, W(t).c_str(), title, MB_OK | icon); }

static void resetOutput(HWND hwnd) {
    g_has = false; ListView_DeleteAllItems(hResList);
    InvalidateRect(hwnd, &kGantt, TRUE);
}


    // Hàm đọc CSV .
static void openCsv(HWND hwnd) {
    // Mở hộp thoại chọn file
    wchar_t path[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"CSV (*.csv)\0*.csv\0";
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameW(&ofn)) return;

    std::ifstream file{std::filesystem::path(path)};
    std::string line;

    if (!file || !std::getline(file, line)) {
        msg(hwnd, "Không đọc được file hoặc file rỗng.",
            L"Lỗi CSV", MB_ICONERROR);
        return;
    }

    // Tách một dòng theo dấu phẩy
    auto split = [](const std::string& text) {
        // Đọc CSV có dấu ngoặc kép và dấu phẩy nằm bên trong trường.
        std::vector<std::string> cells;
        std::string cell;
        bool quoted = false;
        for (size_t i = 0; i < text.size(); ++i) {
            const char ch = text[i];
            if (ch == '"') {
                if (quoted && i + 1 < text.size() && text[i + 1] == '"') {
                    cell += '"';
                    ++i;
                } else {
                    quoted = !quoted;
                }
            } else if (ch == ',' && !quoted) {
                cells.push_back(trim(cell));
                cell.clear();
            } else {
                cell += ch;
            }
        }
        cells.push_back(trim(cell));
        return cells;
    };

    // Bỏ BOM nếu file có
    if (line.size() >= 3 &&
        (unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF) {
        line.erase(0, 3);
    }

    // Tìm vị trí các cột trong dòng tiêu đề
    auto headers = split(line);
    auto findColumn = [&](const std::string& name) {
        for (int i = 0; i < (int)headers.size(); ++i) {
            if (headers[i] == name) return i;
        }
        return -1;
    };

    int pidCol = findColumn("pid");
    int arrivalCol = findColumn("arrival_time");
    int burstCol = findColumn("burst_time");

    if (pidCol < 0 || arrivalCol < 0 || burstCol < 0) {
        msg(hwnd, "CSV cần có cột pid, arrival_time, burst_time.",
            L"Lỗi CSV", MB_ICONERROR);
        return;
    }

    // Đọc từng dòng thành RawRow
    std::vector<RawRow> rows;

    while (std::getline(file, line)) {
        if (trim(line).empty()) continue;

        auto cells = split(line);
        auto getCell = [&](int column) {
            return column < (int)cells.size() ? cells[column] : "";
        };

        rows.push_back({
            getCell(pidCol),
            getCell(arrivalCol),
            getCell(burstCol)
        });
    }

    // Dùng hàm validate() đã có trong code GUI
    std::vector<Process> processes;
    auto errors = validate(rows, processes);

    if (!errors.empty()) {
        std::string message;
        for (const auto& error : errors)
            message += error + "\n";

        msg(hwnd, message, L"Dữ liệu không hợp lệ", MB_ICONERROR);
        return;
    }

    // Đưa dữ liệu hợp lệ lên bảng GUI
    ListView_DeleteAllItems(hInList);

    for (const auto& p : processes) {
        addRow(hInList, {
            W(p.pid),
            std::to_wstring(p.arrival_time),
            std::to_wstring(p.burst_time)
        });
    }

    resetOutput(hwnd);
}

static void fillTable() {
    ListView_DeleteAllItems(hResList);
    for (auto& p : g_result.processes)
        addRow(hResList, {W(p.pid), std::to_wstring(p.arrival_time), std::to_wstring(p.burst_time), std::to_wstring(p.start_time),
                          std::to_wstring(p.completion_time), std::to_wstring(p.waiting_time),
                          std::to_wstring(p.turnaround_time), std::to_wstring(p.response_time)});
    wchar_t a[32], b[32], c[32];
    swprintf(a, 32, L"%.2f", g_result.averages.average_waiting_time);
    swprintf(b, 32, L"%.2f", g_result.averages.average_turnaround_time);
    swprintf(c, 32, L"%.2f", g_result.averages.average_response_time);
    addRow(hResList, {L"TRUNG BÌNH", L"", L"", L"", L"", a, b, c});
}

static void run(HWND hwnd) {
    std::vector<RawRow> rows;
    for (int r = 0; r < ListView_GetItemCount(hInList); ++r)
        rows.push_back({cellText(hInList, r, 0), cellText(hInList, r, 1), cellText(hInList, r, 2)});
    std::vector<Process> procs; auto errs = validate(rows, procs);
    if (!errs.empty()) { std::string m; for (auto& e : errs) m += e + "\n"; msg(hwnd, m, L"Dữ liệu không hợp lệ", MB_ICONERROR); return; }
    try {
        g_result = (SendMessageW(hAlgo, CB_GETCURSEL, 0, 0) == 0) ? run_fcfs(procs) : run_sjf(procs);
        g_has = true;
        fillTable();
        InvalidateRect(hwnd, &kGantt, TRUE);
    } catch (const std::exception& e) {
        resetOutput(hwnd);
        msg(hwnd, e.what(), L"Chưa chạy được thuật toán", MB_ICONINFORMATION);
    }
}
static std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char ch : value) {
        if (ch == '\"') escaped += "\"\"";
        else escaped += ch;
    }
    escaped += "\"";
    return escaped;
}

static void writeCsvRow(
    std::ofstream& file,
    std::initializer_list<std::string> cells
) {
    bool firstCell = true;
    for (const auto& cell : cells) {
        if (!firstCell) file << ",";
        file << csvEscape(cell);
        firstCell = false;
    }
    file << "\n";
}

static void writeScheduleResult(
    std::ofstream& file,
    const ScheduleResult& result
) {
    // Tên các cột
    writeCsvRow(file, {
        "record_type", "algorithm", "pid", "segment_type",
        "start_time", "end_time", "arrival_time", "burst_time",
        "completion_time", "waiting_time", "turnaround_time",
        "response_time", "average_waiting_time",
        "average_turnaround_time", "average_response_time"
    });

    // Các đoạn Gantt
    for (const auto& s : result.segments) {
        writeCsvRow(file, {
            "SEGMENT",
            result.algorithm,
            s.pid,
            s.type == Segment::PROCESS ? "PROCESS" : "IDLE",
            std::to_string(s.start_time),
            std::to_string(s.end_time),
            "", "", "", "", "", "", "", "", ""
        });
    }

    // Kết quả từng tiến trình
    for (const auto& p : result.processes) {
        writeCsvRow(file, {
            "PROCESS", result.algorithm, p.pid,
            "", std::to_string(p.start_time), "",
            std::to_string(p.arrival_time),
            std::to_string(p.burst_time),
            std::to_string(p.completion_time),
            std::to_string(p.waiting_time),
            std::to_string(p.turnaround_time),
            std::to_string(p.response_time),
            "", "", ""
        });
    }

    // Các giá trị trung bình
    writeCsvRow(file, {
        "AVERAGES", result.algorithm,
        "", "", "", "", "", "", "", "", "", "",
        std::to_string(result.averages.average_waiting_time),
        std::to_string(result.averages.average_turnaround_time),
        std::to_string(result.averages.average_response_time)
    });
}
    //  hàm ghi CSV .
    static void exportCsv(HWND hwnd) {
    if (!g_has) {
        msg(hwnd, "Hãy bấm Chạy trước.",
            L"Chưa có kết quả", MB_ICONINFORMATION);
        return;
    }

    wchar_t path[MAX_PATH] = L"ketqua.csv";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"CSV (*.csv)\0*.csv\0";
    ofn.lpstrDefExt = L"csv";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (!GetSaveFileNameW(&ofn)) return;

    std::ofstream file{std::filesystem::path(path)};
    if (!file) {
        msg(hwnd, "Không ghi được file.",
            L"Lỗi CSV", MB_ICONERROR);
        return;
    }

    writeScheduleResult(file, g_result);

    if (!file) {
        msg(hwnd, "Có lỗi khi ghi CSV.",
            L"Lỗi CSV", MB_ICONERROR);
        return;
    }

    msg(hwnd, "Đã xuất kết quả CSV.",
        L"Hoàn tất", MB_ICONINFORMATION);
}

// ---- Vẽ Gantt chart
static void drawGantt(HDC dc) {
    HGDIOBJ oldFont = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, 10, 195, L"3. Gantt chart", 14);
    if (g_has && !g_result.segments.empty()) {
        int total = 0; for (auto& s : g_result.segments) total = std::max(total, s.end_time);
        const int x0 = kGantt.left + 10, y0 = kGantt.top + 25, h = 45;
        double unit = double(kGantt.right - kGantt.left - 20) / total;       // px mỗi đơn vị thời gian
        static const COLORREF pal[] = {RGB(76,120,168), RGB(245,133,24), RGB(84,162,75), RGB(228,87,86),
                                       RGB(114,183,178), RGB(178,121,162), RGB(238,202,59), RGB(157,117,93)};
        std::vector<std::string> order;                                      // mỗi PID một màu cố định
        for (auto& s : g_result.segments) {
            int xs = x0 + (int)(s.start_time * unit), xe = x0 + (int)(s.end_time * unit);
            RECT rc = {xs, y0, xe, y0 + h};
            COLORREF col = RGB(189,189,189); std::wstring label = L"IDLE"; COLORREF txt = RGB(0,0,0);
            if (s.type == Segment::PROCESS) {
                size_t k = std::find(order.begin(), order.end(), s.pid) - order.begin();
                if (k == order.size()) order.push_back(s.pid);
                col = pal[k % 8]; label = W(s.pid); txt = RGB(255,255,255);
            }
            HBRUSH br = CreateSolidBrush(col); FillRect(dc, &rc, br); DeleteObject(br);
            FrameRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            SetTextColor(dc, txt); DrawTextW(dc, label.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(dc, RGB(0,0,0)); std::wstring t = std::to_wstring(s.start_time);
            TextOutW(dc, xs - 4, y0 + h + 4, t.c_str(), (int)t.size());       // mốc bắt đầu
        }
        std::wstring t = std::to_wstring(total);
        TextOutW(dc, x0 + (int)(total * unit) - 4, y0 + h + 4, t.c_str(), (int)t.size());   // mốc kết thúc
    }
    SelectObject(dc, oldFont);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX ic{sizeof(ic), ICC_LISTVIEW_CLASSES}; InitCommonControlsEx(&ic);
        mk(hwnd, L"STATIC", L"pid", 0, 10, 10, 80, 18);
        mk(hwnd, L"STATIC", L"arrival_time", 0, 100, 10, 80, 18);
        mk(hwnd, L"STATIC", L"burst_time", 0, 190, 10, 80, 18);
        hPid = mk(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 10, 30, 80, 24, ID_PID);
        hAt  = mk(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 100, 30, 80, 24, ID_AT);
        hBt  = mk(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 190, 30, 80, 24, ID_BT);
        mk(hwnd, L"BUTTON", L"Thêm", BS_PUSHBUTTON, 280, 30, 60, 24, ID_ADD);
        mk(hwnd, L"BUTTON", L"Xóa dòng chọn", BS_PUSHBUTTON, 345, 30, 100, 24, ID_DEL);
        mk(hwnd, L"BUTTON", L"Xóa tất cả", BS_PUSHBUTTON, 450, 30, 80, 24, ID_CLR);
        mk(hwnd, L"BUTTON", L"Mở CSV...", BS_PUSHBUTTON, 535, 30, 90, 24, ID_OPEN);
        hInList = mk(hwnd, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 10, 62, 420, 120, ID_INLIST, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(hInList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        addCols(hInList, {L"pid", L"arrival_time", L"burst_time"}, 130);
        mk(hwnd, L"STATIC", L"2. Thuật toán:", 0, 450, 66, 100, 18);
        hAlgo = mk(hwnd, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, 450, 86, 220, 100, ID_ALGO);
        SendMessageW(hAlgo, CB_ADDSTRING, 0, (LPARAM)L"FCFS");
        SendMessageW(hAlgo, CB_ADDSTRING, 0, (LPARAM)L"SJF (không độc quyền)");
        SendMessageW(hAlgo, CB_SETCURSEL, 0, 0);
        mk(hwnd, L"BUTTON", L"▶ Chạy", BS_PUSHBUTTON, 450, 120, 100, 30, ID_RUN);
        mk(hwnd, L"BUTTON", L"Xuất CSV...", BS_PUSHBUTTON, 560, 120, 110, 30, ID_EXPORT);
        mk(hwnd, L"STATIC", L"4. Kết quả", 0, 10, 345, 100, 18);
        hResList = mk(hwnd, WC_LISTVIEW, L"", LVS_REPORT | LVS_SHOWSELALWAYS, 10, 365, 965, 290, ID_RESLIST, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(hResList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        addCols(hResList, {L"pid", L"arrival_time", L"burst_time", L"start_time", L"completion_time",
                           L"waiting_time", L"turnaround_time", L"response_time"}, 118);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_ADD: {
            std::string a = editText(hPid), b = editText(hAt), c = editText(hBt);
            addRow(hInList, {W(a), W(b), W(c)});
            SetWindowTextW(hPid, L""); SetWindowTextW(hAt, L""); SetWindowTextW(hBt, L""); SetFocus(hPid);
            resetOutput(hwnd); break; }
        case ID_DEL: {
            int i = ListView_GetNextItem(hInList, -1, LVNI_SELECTED);
            if (i >= 0) { ListView_DeleteItem(hInList, i); resetOutput(hwnd); }
            break; }
        case ID_CLR: ListView_DeleteAllItems(hInList); resetOutput(hwnd); break;
        case ID_OPEN: openCsv(hwnd); break;
        case ID_RUN: run(hwnd); break;
        case ID_EXPORT: exportCsv(hwnd); break;
        case ID_ALGO: if (HIWORD(wp) == CBN_SELCHANGE) resetOutput(hwnd); break;   // đổi thuật toán -> xóa kết quả cũ
        }
        return 0;
    case WM_PAINT: { PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps); drawGantt(dc); EndPaint(hwnd, &ps); return 0; }
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, m, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int show) {
    WNDCLASSW wc{}; wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"OS_SCH_TV4";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassW(&wc);
    HWND h = CreateWindowExW(0, wc.lpszClassName, L"OS_SCH_01 - Mô phỏng lập lịch CPU",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 1005, 710, nullptr, nullptr, hi, nullptr);
    ShowWindow(h, show);
    MSG msg; while (GetMessageW(&msg, nullptr, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return 0;
}

