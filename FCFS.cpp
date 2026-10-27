#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>

using namespace std;

struct Process {
    int id;                 // Process ID (P1, P2, ...)
    int arrival_time;       // Thời gian đến (arrival time)
    int burst_time;         // Thời gian thực thi (burst time)
    int original_index;     // Thứ tự trong file CSV (để xử lý tie-break)

    // Các chỉ số kết quả
    int start_time = -1;    // Thời điểm bắt đầu thực thi
    int completion_time = -1; // Thời điểm hoàn thành
    int waiting_time = 0;   // Thời gian chờ
    int turnaround_time = 0; // Thời gian hoàn thành tổng cộng
    int response_time = 0;  // Thời gian phản hồi (bằng waiting_time ở FCFS)
};

void trim(string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) {
        s = "";
        return;
    }
    s = s.substr(start, end - start + 1);
}

vector<Process> readCSV(const string &filename) {
    vector<Process> processes;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Loi: Khong the mo file " << filename << endl;
        return processes;
    }

    string line;
    bool firstLine = true;
    int index = 0;

    while (getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        if (firstLine) {
            // Bỏ qua dòng header
            firstLine = false;
            continue;
        }

        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            trim(token);
            tokens.push_back(token);
        }

        if (tokens.size() < 2) {
            cerr << "Canh bao: Dong khong hop le, bo qua: " << line << endl;
            continue;
        }

        Process p;
        p.id = index + 1;
        p.original_index = index;

        try {
            p.arrival_time = stoi(tokens[0]);
            p.burst_time = stoi(tokens[1]);
        } catch (const exception &e) {
            cerr << "Loi parse so: " << line << endl;
            continue;
        }

        if (p.burst_time <= 0) {
            cerr << "Canh bao: Burst time phai > 0, bo qua process " << p.id << endl;
            continue;
        }

        processes.push_back(p);
        index++;
    }

    file.close();
    return processes;
}

void fcfsScheduling(vector<Process> &processes) {
    // Sắp xếp theo arrival_time, nếu bằng nhau thì theo original_index (thứ tự trong file)
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b) {
        if (a.arrival_time != b.arrival_time)
            return a.arrival_time < b.arrival_time;
        return a.original_index < b.original_index;
    });

    int current_time = 0;
    int total_idle_time = 0;

    cout << "\n=== QUY TRINH LAP LICH FCFS ===\n";
    cout << left << setw(8) << "PID" << setw(12) << "Arrival" << setw(10) << "Burst"
         << setw(12) << "Start" << setw(14) << "Complete" << setw(10) << "Wait"
         << setw(14) << "Turnaround" << setw(12) << "Response" << endl;
    cout << string(92, '-') << endl;

    for (auto &p : processes) {
        // Xử lý CPU Idle: nếu chưa có process nào đến, CPU chờ
        if (current_time < p.arrival_time) {
            int idle_duration = p.arrival_time - current_time;
            cout << "[IDLE] CPU nghỉ từ " << current_time << " đến " << p.arrival_time
                 << " (khoảng " << idle_duration << " đơn vị thời gian)\n";
            total_idle_time += idle_duration;
            current_time = p.arrival_time;
        }

        // Process bắt đầu thực thi
        p.start_time = current_time;
        p.waiting_time = p.start_time - p.arrival_time;
        p.response_time = p.waiting_time; // FCFS: response time = waiting time

        // Thực thi liên tục cho đến khi xong
        current_time += p.burst_time;
        p.completion_time = current_time;
        p.turnaround_time = p.completion_time - p.arrival_time;

        // In kết quả process này
        cout << left << setw(8) << ("P" + to_string(p.id))
             << setw(12) << p.arrival_time
             << setw(10) << p.burst_time
             << setw(12) << p.start_time
             << setw(14) << p.completion_time
             << setw(10) << p.waiting_time
             << setw(14) << p.turnaround_time
             << setw(12) << p.response_time << endl;
    }

    // Tính các chỉ số trung bình
    double avg_waiting = 0, avg_turnaround = 0, avg_response = 0;
    int n = processes.size();

    for (const auto &p : processes) {
        avg_waiting += p.waiting_time;
        avg_turnaround += p.turnaround_time;
        avg_response += p.response_time;
    }

    if (n > 0) {
        avg_waiting /= n;
        avg_turnaround /= n;
        avg_response /= n;
    }

    cout << string(92, '-') << endl;
    cout << "\n=== CHI SO THONG KE ===\n";
    cout << "Tong so tien trinh: " << n << endl;
    cout << "Tong thoi gian CPU Idle: " << total_idle_time << endl;
    cout << fixed << setprecision(2);
    cout << "Thoi gian cho trung binh (Avg Waiting Time):     " << avg_waiting << endl;
    cout << "Thoi gian hoan thanh trung binh (Avg Turnaround): " << avg_turnaround << endl;
    cout << "Thoi gian phan hoi trung binh (Avg Response):     " << avg_response << endl;

    // In biểu đồ Gantt đơn giản
    cout << "\n=== BIEU DO GANTT ===\n";
    current_time = 0;
    cout << "|";
    for (const auto &p : processes) {
        if (current_time < p.arrival_time) {
            cout << " IDLE(" << p.arrival_time - current_time << ") |";
            current_time = p.arrival_time;
        }
        cout << " P" << p.id << "(" << p.burst_time << ") |";
        current_time += p.burst_time;
    }
    cout << "\n";

    // In timeline
    current_time = 0;
    cout << "0";
    for (const auto &p : processes) {
        if (current_time < p.arrival_time) {
            int idle = p.arrival_time - current_time;
            cout << string(idle * 3, '-') << p.arrival_time;
            current_time = p.arrival_time;
        }
        cout << string(p.burst_time * 3, '-') << current_time + p.burst_time;
        current_time += p.burst_time;
    }
    cout << endl;
}

void writeResultsCSV(const string &filename, const vector<Process> &processes) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Loi: Khong the ghi file " << filename << endl;
        return;
    }

    file << "PID,Arrival_Time,Burst_Time,Start_Time,Completion_Time,Waiting_Time,Turnaround_Time,Response_Time\n";

    for (const auto &p : processes) {
        file << "P" << p.id << ","
             << p.arrival_time << ","
             << p.burst_time << ","
             << p.start_time << ","
             << p.completion_time << ","
             << p.waiting_time << ","
             << p.turnaround_time << ","
             << p.response_time << "\n";
    }

    file.close();
    cout << "\nDa ghi ket qua vao file: " << filename << endl;
}

int main() {
    cout << "========================================\n";
    cout << "  THUAT TOAN FCFS (First-Come, First-Served)\n";
    cout << "  Mon: He Dieu Hanh\n";
    cout << "========================================\n";

    string input_file;
    cout << "\nNhap ten file CSV dau vao (VD: input.csv): ";
    getline(cin, input_file);
    trim(input_file);

    if (input_file.empty()) {
        input_file = "input.csv";
    }

    vector<Process> processes = readCSV(input_file);

    if (processes.empty()) {
        cout << "\nKhong co du lieu tien trinh hop le. Kiem tra file input.\n";
        cout << "Dinh dang file CSV mong doi:\n";
        cout << "  Arrival_Time,Burst_Time\n";
        cout << "  0,5\n";
        cout << "  1,3\n";
        cout << "  2,8\n";
        cout << "...\n";
        return 1;
    }

    cout << "\nDoc duoc " << processes.size() << " tien trinh tu file.\n";

    fcfsScheduling(processes);

    // Ghi file kết quả
    string output_file = "fcfs_output.csv";
    writeResultsCSV(output_file, processes);

    cout << "\nHoan tat!\n";
    return 0;
}