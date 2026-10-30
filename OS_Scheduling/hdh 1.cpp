#include <iostream>
#include <string> // Thư viện để sử dụng các đối tượng và hàm liên quan đến chuỗi
#include <vector>// Thư viện vector để sử dụng kiểu dữ liệu vector
#include <algorithm>// Thư viện để sử dụng hàm sort
#include <iomanip> // Thư viện để in ra màn hình đẹp hơn
using namespace std;
// 1. Cấu trúc lưu thông tin đầu vào của một tiến trình
struct Process 
{
    string pid;           // Mã định danh tiến trình (Ví dụ: "P1", "P2"...)
    int arrival_time;     // Thời điểm tiến trình đến hệ thống
    int burst_time;       // Thời gian cần thiết để tiến trình chạy xong trên CPU
    int original_index;   // Lưu thứ tự dòng ban đầu trong file CSV để xử lý khi bị Tie
};
// 2. Cấu trúc lưu thông tin từng đoạn trên biểu đồ Gantt Chart
struct Segment 
{
    string type;          // Loại đoạn: "PROCESS" hoặc "IDLE"
    string pid;           // Tên tiến trình hoặc rỗng nếu là IDLE
    int start_time;       // Thời điểm bắt đầu
    int end_time;         // Thời điểm kết thúc
};
// 3. Cấu trúc lưu kết quả chi tiết của từng tiến trình sau khi chạy xong
struct ProcessResult 
{
    string pid; 
	int waiting_time; // Thời gian chờ của tiến trình
	int turnaround_time;// Thời gian hoàn thành của tiến trình
	int original_index;// Lưu thứ tự dòng ban đầu trong file CSV để xử lý khi bị Tie
	int arrival_time;// Thời điểm tiến trình đến hệ thống
	int burst_time;//	Thời gian cần thiết để tiến trình chạy xong trên CPU
	int start_time;// Thời điểm bắt đầu chạy của tiến trình
	int completion_time;// Thời điểm hoàn thành của tiến trình
	int response_time;// Thời gian phản hồi của tiến trình
};
// 4. Cấu trúc tổng hợp kết quả trả về chung cho toàn bộ chương trình
struct ScheduleResult 
{
	string algorithm = "SJF";// Tên thuật toán
	vector<Segment> segments;// Danh sách các đoạn trên biểu đồ Gantt Chart
	vector<ProcessResult> processes;// Danh sách kết quả chi tiết của từng tiến trình
    struct 
    {
		double average_waiting_time;// Thời gian chờ trung bình của tất cả tiến trình
		double average_turnaround_time;// Thời gian hoàn thành trung bình của tất cả tiến trình
		double average_response_time;// Thời gian phản hồi trung bình của tất cả tiến trình
    } 
    averages;
};
// hàm thực thi toán tử SJF không ưu tiên (Non-Preemptive SJF)
ScheduleResult runNonPreemptiveSJF(vector<Process> input_processes)  
{
    // Bước 1: Gắn chỉ số ban đầu phục vụ Tie-breaking theo thứ tự CSV
    for (size_t i = 0; i < input_processes.size(); ++i) // gắn chỉ số ban đầu cho từng tiến trình
    {
		input_processes[i].original_index = (int)i; // gán chỉ số ban đầu cho từng tiến trình
    }
	vector<Process> unvisited = input_processes; // Danh sách tiến trình chưa được xử lý
	vector<ProcessResult> completed_processes; // Danh sách tiến trình đã được xử lý xong
	vector<Segment> segments; // Danh sách các đoạn trên biểu đồ Gantt Chart
	int current_time = 0; // Thời điểm hiện tại trong quá trình mô phỏng
    // Bước 2: Vòng lặp giả lập tiến trình chạy đến khi hết
    while (!unvisited.empty()) 
    { 
		vector<Process> ready_queue;// Danh sách tiến trình đã đến hệ thống và sẵn sàng chạy
        for (const auto& p : unvisited) // duyệt qua d.sách tiến trình chưa xử lý
        {
            if (p.arrival_time <= current_time) // nếu t,trinh đến hệ thống
            {
				ready_queue.push_back(p);// thêm vào danh sách sẵn sàng chạy
            }
        }
        // Nếu hàng đợi trống -> CPU ở trạng thái IDLE
        if (ready_queue.empty())
        {
			int next_arrival = unvisited[0].arrival_time;// tìm thời điểm tiến trình tiếp theo đến hệ thống
            for (const auto& p : unvisited) // duyệt qua danh sách chưa xử lý 
            {
				if (p.arrival_time < next_arrival)// tìm thời điểm tiến trình tiếp theo đến hệ thống
                {
					next_arrival = p.arrival_time;// cập nhật thời điểm tiến trình tiếp theo đến hệ thống
                }
            }
			segments.push_back({ "IDLE", "", current_time, next_arrival // thêm đoạn IDLE vào biểu đồ Gantt Chart
                });
			current_time = next_arrival;// cập nhật thời điểm hiện tại
			continue;// tiếp tục vòng lặp
        }
        // Chọn tiến trình theo quy tắc SJF và Tie-breaking
		auto best_it = ready_queue.begin();// iterator trỏ đến tiến trình có thời gian chạy ngắn nhất
        for (auto it = ready_queue.begin(); it != ready_queue.end(); ++it)//duyệt qua d.sách sẵn sàng chạy 
        {
			if (it->burst_time < best_it->burst_time) // nếu thời gian chạy ngắn hơn
            {
				best_it = it; // cập nhật tiến trình có thời gian chạy ngắn nhất
            }
			else if (it->burst_time == best_it->burst_time) // nếu thời gian chạy bằng nhau
            {
				if (it->arrival_time < best_it->arrival_time) // nếu thời điểm đến hệ thống sớm hơn
                {
					best_it = it;// cập nhật tiến trình có thời điểm đến hệ thống sớm hơn
                }
				else if (it->arrival_time == best_it->arrival_time) // nếu thời điểm đến hệ thống bằng nhau
                {
					if (it->original_index < best_it->original_index) // nếu chỉ số ban đầu nhỏ hơn
					{
						best_it = it; // cập nhật tiến trình có chỉ số ban đầu nhỏ hơn
					}
                    {
						best_it = it; // cập nhật tiến trình có chỉ số ban đầu nhỏ hơn
                    }
                }
            }
        }
		Process best_p = *best_it; // tiến trình được chọn để chạy
        // Bước 3: Tính toán các mốc thời gian và chỉ số
		int start_time = current_time;// thời điểm bắt đầu chạy tiến trình
		int completion_time = start_time + best_p.burst_time;// thời điểm kết thúc chạy tiến trình
		int waiting_time = start_time - best_p.arrival_time;// thời gian chờ đợi của tiến trình
		int turnaround_time = completion_time - best_p.arrival_time;// thời gian hoàn thành của tiến trình
		int response_time = waiting_time;// thời gian phản hồi của tiến trình (bằng thời gian chờ đợi trong SJF không ưu tiên)
		segments.push_back({ "PROCESS", best_p.pid, start_time, completion_time });// thêm đoạn tiến trình vào biểu đồ Gantt Chart
		completed_processes.push_back({// lưu kết quả tiến trình đã hoàn thành
			best_p.pid, waiting_time, turnaround_time, best_p.original_index,// lưu chỉ số ban đầu
			best_p.arrival_time, best_p.burst_time, start_time,// lưu thời điểm bắt đầu
			completion_time, response_time// lưu thời điểm kết thúc và thời gian phản hồi
            });
		current_time = completion_time;// cập nhật thời điểm hiện tại
		unvisited.erase(// xóa tiến trình đã hoàn thành khỏi danh sách chưa xử lý
			remove_if(unvisited.begin(), unvisited.end(), [&](const Process& p) // tìm tiến trình đã hoàn thành
                {
					return p.pid == best_p.pid;// so sánh mã định danh tiến trình
                }),
			unvisited.end()// xóa tiến trình đã hoàn thành khỏi danh sách chưa xử lý
        );
    }
    // Bước 4: Sắp xếp lại danh sách kết quả theo thứ tự CSV ban đầu
	std::sort(completed_processes.begin(), completed_processes.end(), [](const ProcessResult& a, const ProcessResult& b)// sắp xếp theo chỉ số ban đầu
		{
			return a.original_index < b.original_index;// so sánh chỉ số ban đầu
		});
    // Bước 5: Tính các giá trị trung bình
		double total_waiting = 0, total_turnaround = 0, total_response = 0;// biến lưu tổng thời gian chờ đợi, tổng thời gian hoàn thành, tổng thời gian phản hồi
		int n = (int)completed_processes.size();// số lượng tiến trình đã hoàn thành
		for (const auto& cp : completed_processes) // duyệt qua danh sách tiến trình đã hoàn thành
    {
			total_waiting += cp.waiting_time;// cộng dồn thời gian chờ đợi
			total_turnaround += cp.turnaround_time;// cộng dồn thời gian hoàn thành
			total_response += cp.response_time;// cộng dồn thời gian phản hồi
    }
		ScheduleResult result;// tạo kết quả trả về
		result.segments = segments;// lưu danh sách các đoạn trên biểu đồ Gantt Chart
		result.processes = completed_processes;// lưu danh sách tiến trình đã hoàn thành
		result.averages.average_waiting_time = n > 0 ? total_waiting / n : 0.0;// tính thời gian chờ đợi trung bình
		result.averages.average_turnaround_time = n > 0 ? total_turnaround / n : 0.0;// tính thời gian hoàn thành trung bình
		result.averages.average_response_time = n > 0 ? total_response / n : 0.0;// tính thời gian phản hồi trung bình
		return result;// trả về kết quả
}
// hàm main kiểm thử
int main() 
{
    vector<Process> test_processes = // d.sách tiến trình kiểm thử
    {
        {"P1", 0, 5, 0},
        {"P2", 1, 3, 0},
        {"P3", 2, 1, 0}
    };
	ScheduleResult res = runNonPreemptiveSJF(test_processes);// chạy thuật toán SJF không ưu tiên
	cout << "=== KET QUA THUAT TOAN: " << res.algorithm << " ===" << endl;// in ra tên thuật toán
	cout << "\n1. GANTT CHART:" << endl;// in ra biểu đồ Gantt Chart
    for (const auto& seg : res.segments) //duyệt qua danh sách các đoạn trên gantt chart
    {
        if (seg.type == "IDLE") // nếu là idle
        {
			cout << "[IDLE: " << seg.start_time << " -> " << seg.end_time << "] ";// in ra đoạn idle
        }
        else 
        {
			cout << "[" << seg.pid << ": " << seg.start_time << " -> " << seg.end_time << "] ";// in ra đoạn tiến trình
        }
    }
    cout << endl;
    // In bảng thông số chi tiết đã được sửa chuẩn cú pháp
	cout << "\n2. BANG CHI SO CHI TIET:" << endl;// in ra bảng thông số chi tiết
	cout << left << setw(8) << "PID"// in ra tiêu đề cột PID
		<< setw(8) << "Arrival"// in ra tiêu đề cột Arrival
		<< setw(8) << "Burst"// in ra tiêu đề cột Burst
		<< setw(8) << "Start"// in ra tiêu đề cột Start
		<< setw(12) << "Completion"// in ra tiêu đề cột Completion
		<< setw(10) << "Waiting"// in ra tiêu đề cột Waiting
		<< setw(12) << "Turnaround"// in ra tiêu đề cột Turnaround
		<< setw(10) << "Response" << endl;// in ra tiêu đề cột Response
    for (const auto& p : res.processes) // duyệt d.sách tiến trình đã hoàn thành
    {
		cout << left << setw(8) << p.pid;// in ra mã định danh tiến trình
    {
			cout << left << setw(8) << p.pid// in ra mã định danh tiến trình
				<< setw(8) << p.arrival_time// in ra thời điểm đến hệ thống
				<< setw(8) << p.burst_time// in ra thời gian chạy
				<< setw(8) << p.start_time// in ra thời điểm bắt đầu chạy
				<< setw(12) << p.completion_time// in ra thời điểm kết thúc chạy
				<< setw(10) << p.waiting_time// in ra thời gian chờ đợi
				<< setw(12) << p.turnaround_time// in ra thời gian hoàn thành
				<< setw(10) << p.response_time << endl;// in ra thời gian phản hồi
    }
	cout << "\n3. TRUNG BINH (AVERAGES):" << endl;// in ra tiêu đề phần trung bình
	cout << fixed << setprecision(2);// định dạng số thập phân với 2 chữ số sau dấu phẩy
	cout << "- Avg Waiting Time: " << res.averages.average_waiting_time << endl;// in ra thời gian chờ đợi trung bình
	cout << "- Avg Turnaround Time: " << res.averages.average_turnaround_time << endl;// in ra thời gian hoàn thành trung bình
	cout << "- Avg Response Time: " << res.averages.average_response_time << endl;// in ra thời gian phản hồi trung bình
	return 0;// trả về 0 để kết thúc chương trình
}