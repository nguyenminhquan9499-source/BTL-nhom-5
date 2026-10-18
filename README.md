# BTL Nhóm 5 — OS_SCH_01

Ứng dụng desktop mô phỏng và so sánh hai thuật toán lập lịch CPU: FCFS và SJF không độc quyền.

## Phạm vi

Mô phỏng các tiến trình có arrival time và burst time. Chương trình hiển thị Gantt chart, waiting time, turnaround time và response time.

## Quy ước mô phỏng

- FCFS: tiến trình đến trước chạy trước.
- SJF không độc quyền: khi CPU rảnh, chọn tiến trình đã đến có burst time ngắn nhất.
- Nếu hòa, dùng thứ tự dòng trong CSV để phân xử cuối cùng.
- Nếu chưa có tiến trình sẵn sàng, Gantt chart thể hiện CPU idle.

## CSV đầu vào

Các cột: `pid,arrival_time,burst_time`

```csv
pid,arrival_time,burst_time
P1,0,5
P2,1,3
P3,2,1
