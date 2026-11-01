// sjf_stub.h - BẢN TẠM của SJF để GUI chạy được. Khi TV3 xong:
//   xóa file này, thay bằng sjf.h của TV3 (hàm run_sjf trả về ScheduleResult).
#pragma once
#include "contract.h"
#include <algorithm>
#include <climits>

inline ScheduleResult run_sjf(const std::vector<Process>& ps) {
    ScheduleResult r; r.algorithm = "SJF";
    int n = (int)ps.size(), t = 0, done = 0;
    std::vector<bool> fin(n, false); std::vector<int> st(n), ct(n);
    while (done < n) {
        int pick = -1;
        for (int i = 0; i < n; ++i) {
            if (fin[i] || ps[i].arrival_time > t) continue;
            if (pick < 0) { pick = i; continue; }
            bool better = ps[i].burst_time != ps[pick].burst_time ? ps[i].burst_time < ps[pick].burst_time
                        : ps[i].arrival_time != ps[pick].arrival_time ? ps[i].arrival_time < ps[pick].arrival_time : false;
            if (better) pick = i;
        }
        if (pick < 0) {
            int nxt = INT_MAX;
            for (int i = 0; i < n; ++i) if (!fin[i]) nxt = std::min(nxt, ps[i].arrival_time);
            r.segments.push_back({Segment::IDLE, "", t, nxt}); t = nxt; continue;
        }
        r.segments.push_back({Segment::PROCESS, ps[pick].pid, t, t + ps[pick].burst_time});
        st[pick] = t; t += ps[pick].burst_time; ct[pick] = t; fin[pick] = true; ++done;
    }
    double sw = 0, sT = 0, sr = 0;
    for (int i = 0; i < n; ++i) {
        ProcessResult p{ps[i].pid, ps[i].arrival_time, ps[i].burst_time, st[i], ct[i],
                        st[i] - ps[i].arrival_time, ct[i] - ps[i].arrival_time, st[i] - ps[i].arrival_time};
        sw += p.waiting_time; sT += p.turnaround_time; sr += p.response_time; r.processes.push_back(p);
    }
    if (n) r.averages = {sw / n, sT / n, sr / n};
    return r;
}
