#include "../gui.cpp"

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

int soLoi = 0;

void kiemTra(bool dung, const std::string& ten) {
    if (dung) {
        std::cout << "PASS: " << ten << "\n";
    } else {
        std::cout << "FAIL: " << ten << "\n";
        ++soLoi;
    }
}

int main() {
    std::vector<Process> processes;
    std::vector<std::string> errors;
    std::vector<std::string> cells;

    // Kiểm tra tách ô CSV có dấu phẩy trong PID.
    bool tachDuoc = parseCsvRow("\"P,1\",0,5", cells);
    kiemTra(
        tachDuoc && cells.size() == 3 && cells[0] == "P,1",
        "Tach CSV co dau phay trong PID"
    );

    // Đọc file CSV hợp lệ.
    bool docDuoc = readCsvProcesses(
        "testdata/valid.csv", processes, errors
    );
    kiemTra(
        docDuoc && processes.size() == 3 && processes[1].pid == "P2",
        "Doc file CSV hop le"
    );

    // PID bị trùng phải bị từ chối.
    bool docPIDTrung = readCsvProcesses(
        "testdata/duplicate.csv", processes, errors
    );
    kiemTra(
        !docPIDTrung && !errors.empty(),
        "Tu choi PID bi trung"
    );

    // Arrival âm và burst bằng 0 phải bị từ chối.
    bool docThoiGianSai = readCsvProcesses(
        "testdata/invalid_times.csv", processes, errors
    );
    kiemTra(
        !docThoiGianSai && !errors.empty(),
        "Tu choi thoi gian khong hop le"
    );

    // Đọc PID có dấu phẩy và dấu ngoặc kép.
    bool docPIDCoNgoac = readCsvProcesses(
        "testdata/quoted_pid.csv", processes, errors
    );
    kiemTra(
        docPIDCoNgoac &&
        processes.size() == 2 &&
        processes[0].pid == "P,1" &&
        processes[1].pid == "P\"2",
        "Doc PID co dau phay va ngoac kep"
    );

    // Ngoặc kép mở nhưng không đóng phải bị từ chối.
    bool docNgoacLoi = readCsvProcesses(
        "testdata/bad_quote.csv", processes, errors
    );
    kiemTra(
        !docNgoacLoi && !errors.empty(),
        "Tu choi CSV co ngoac kep sai"
    );

    // Thiếu cột bắt buộc phải bị từ chối.
    bool docThieuCot = readCsvProcesses(
        "testdata/missing_header.csv", processes, errors
    );
    kiemTra(
        !docThieuCot && !errors.empty(),
        "Tu choi CSV thieu cot"
    );

    // File rỗng phải bị từ chối.
    bool docFileRong = readCsvProcesses(
        "testdata/empty.csv", processes, errors
    );
    kiemTra(
        !docFileRong && !errors.empty(),
        "Tu choi file CSV rong"
    );

    // Tạo kết quả có khoảng CPU IDLE để thử ghi CSV.
    ScheduleResult result = run_fcfs({{"P,1", 3, 2}});

    std::ofstream output(
        "testdata/output.csv",
        std::ios::binary
    );

    kiemTra((bool)output, "Tao file CSV ket qua");

    if (output) {
        writeScheduleResult(output, result);
        output.close();
    }

    // Đọc lại file vừa ghi để kiểm tra nội dung.
    std::ifstream input(
        "testdata/output.csv",
        std::ios::binary
    );

    std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>()
    );

    kiemTra(
        content.find("record_type,algorithm") != std::string::npos,
        "File ket qua co dong tieu de"
    );

    kiemTra(
        content.find("SEGMENT") != std::string::npos &&
        content.find("PROCESS") != std::string::npos &&
        content.find("AVERAGES") != std::string::npos,
        "File ket qua co cac loai ban ghi"
    );

    kiemTra(
        content.find("IDLE") != std::string::npos,
        "File ket qua ghi doan CPU IDLE"
    );

    kiemTra(
        content.find("\"P,1\"") != std::string::npos,
        "File ket qua bao ve PID co dau phay"
    );

    if (soLoi == 0) {
        std::cout << "\nTat ca kiem tra deu PASS.\n";
        return 0;
    }

    std::cout << "\nCo " << soLoi << " kiem tra FAIL.\n";
    return 1;
}
