# Quy ước mô phỏng

- Mô phỏng một CPU và mỗi tiến trình có một CPU burst.
- CSV gồm các cột `pid,arrival_time,burst_time`.
- `pid` phải khác nhau; arrival time là số nguyên từ 0 trở lên; burst time là số nguyên lớn hơn 0.
- FCFS: tiến trình đến trước chạy trước; nếu cùng thời điểm đến, ưu tiên dòng đứng trước trong CSV.
- SJF không độc quyền: khi CPU rảnh, chọn tiến trình đã đến có burst time ngắn nhất.
- Nếu SJF hòa burst time, ưu tiên arrival time nhỏ hơn, sau đó đến thứ tự dòng CSV.
- Tiến trình đã chạy thì chạy đến hết, không bị ngắt.
- Nếu chưa có tiến trình sẵn sàng, Gantt chart thể hiện khoảng CPU IDLE.
- Waiting time = start time − arrival time.
- Turnaround time = completion time − arrival time.
- Response time = start time − arrival time.
