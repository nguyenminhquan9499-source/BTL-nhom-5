// Kiểm tra run_fcfs với 3 file input của TV2 (thêm cột pid cho đúng định dạng nhóm).
#include "fcfs.h"
#include <iostream>
static void show(const char* name, std::vector<Process> ps) {
    auto r = run_fcfs(ps);
    std::cout << "== " << name << "\nGantt: ";
    for (auto& s : r.segments) std::cout << (s.type == Segment::IDLE ? "IDLE" : s.pid) << "[" << s.start_time << "-" << s.end_time << "] ";
    std::cout << "\n";
    for (auto& p : r.processes) std::cout << p.pid << " start=" << p.start_time << " done=" << p.completion_time
        << " wait=" << p.waiting_time << " ta=" << p.turnaround_time << " resp=" << p.response_time << "\n";
    std::cout << "avg wait=" << r.averages.average_waiting_time << " ta=" << r.averages.average_turnaround_time
              << " resp=" << r.averages.average_response_time << "\n";
}
int main() {
    show("input.csv  (khong idle)", {{"P1",0,5},{"P2",1,3},{"P3",2,8},{"P4",3,6}});
    show("input2.csv (co idle)",    {{"P1",0,5},{"P2",10,3},{"P3",12,8},{"P4",15,6}});
    show("input3.csv (tie)",        {{"P1",0,5},{"P2",0,3},{"P3",2,8},{"P4",2,6}});
}
