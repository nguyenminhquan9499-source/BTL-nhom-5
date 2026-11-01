// contract.h - CẤU TRÚC DỮ LIỆU CHUNG của nhóm (theo docs/decisions.md).
// TV2, TV3, TV4, TV5 đều dùng chung file này. Đừng tự định nghĩa lại ở nơi khác.
#pragma once
#include <string>
#include <vector>

struct Process {                 // một tiến trình ĐẦU VÀO
    std::string pid;             // ví dụ "P1"
    int arrival_time;
    int burst_time;
};

struct Segment {                 // một đoạn trên Gantt: tiến trình chạy hoặc CPU rảnh
    enum Type { PROCESS, IDLE } type;
    std::string pid;             // để trống nếu IDLE
    int start_time, end_time;
};

struct ProcessResult {           // kết quả của TỪNG tiến trình
    std::string pid;
    int arrival_time, burst_time;
    int start_time, completion_time;
    int waiting_time, turnaround_time, response_time;
};

struct Averages {
    double average_waiting_time = 0;
    double average_turnaround_time = 0;
    double average_response_time = 0;
};

struct ScheduleResult {          // KẾT QUẢ mà mọi thuật toán phải trả về
    std::string algorithm;
    std::vector<Segment> segments;          // các đoạn Gantt theo thời gian (có IDLE)
    std::vector<ProcessResult> processes;   // theo đúng thứ tự dòng trong CSV
    Averages averages;
};
