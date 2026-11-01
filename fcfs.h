// fcfs.h - Thuật toán FCFS (TV2). Logic lấy từ FCFS.cpp của TV2, đổi thành HÀM THUẦN:
//   - nhận vector<Process> (theo contract.h), trả về ScheduleResult
//   - KHÔNG đọc file, KHÔNG cin/cout  => GUI và test gọi được trực tiếp
#pragma once
#include "contract.h"
#include <algorithm>
#include <numeric>

inline ScheduleResult run_fcfs(const std::vector<Process>& ps) {
    ScheduleResult r;
    r.algorithm = "FCFS";
    const int n = (int)ps.size();
    if (n == 0) return r;

    // Thứ tự chạy: arrival nhỏ trước; bằng nhau thì dòng CSV đứng trước (stable_sort giữ thứ tự).
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return ps[a].arrival_time < ps[b].arrival_time; });

    std::vector<ProcessResult> res(n);          // lưu theo vị trí dòng CSV
    int t = 0;
    for (int i : order) {
        if (t < ps[i].arrival_time) {           // CPU rảnh -> đoạn IDLE
            r.segments.push_back({Segment::IDLE, "", t, ps[i].arrival_time});
            t = ps[i].arrival_time;
        }
        ProcessResult p;
        p.pid = ps[i].pid;
        p.arrival_time = ps[i].arrival_time;
        p.burst_time = ps[i].burst_time;
        p.start_time = t;
        p.waiting_time = p.start_time - p.arrival_time;
        p.response_time = p.waiting_time;       // FCFS: response = waiting
        t += p.burst_time;                      // chạy liên tục đến hết
        p.completion_time = t;
        p.turnaround_time = p.completion_time - p.arrival_time;
        r.segments.push_back({Segment::PROCESS, p.pid, p.start_time, p.completion_time});
        res[i] = p;
    }
    double sw = 0, st = 0, sr = 0;
    for (auto& p : res) { sw += p.waiting_time; st += p.turnaround_time; sr += p.response_time; }
    r.processes = res;                          // giữ thứ tự dòng CSV
    r.averages = {sw / n, st / n, sr / n};
    return r;
}
