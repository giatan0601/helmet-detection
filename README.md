# helmet-detection

Hệ thống **nhận diện mũ bảo hiểm** bằng học máy trên thiết bị nhúng (TinyML). Mô hình được huấn luyện trên [Edge Impulse](https://edgeimpulse.com/), xuất ra thư viện Arduino để chạy trực tiếp trên vi điều khiển **ESP32-S3**, đồng thời có script Python để thử nghiệm nhanh với webcam trên máy tính.

## Mục lục

- [Giới thiệu](#giới-thiệu)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Nguyên lý hoạt động](#nguyên-lý-hoạt-động)
- [Yêu cầu](#yêu-cầu)
- [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
- [Kết quả](#kết-quả)
- [Báo cáo](#báo-cáo)
- [Tác giả](#tác-giả)
- [Giấy phép](#giấy-phép)

## Giới thiệu

Đội mũ bảo hiểm khi tham gia giao thông giúp giảm đáng kể nguy cơ chấn thương nghiêm trọng. Dự án này xây dựng một mô hình nhận diện ảnh có thể phân biệt người **có đội** và **không đội** mũ bảo hiểm, với mục tiêu:

- Huấn luyện mô hình nhẹ, phù hợp để chạy trên thiết bị nhúng có tài nguyên hạn chế.
- Triển khai mô hình lên board **ESP32-S3** thông qua thư viện Arduino xuất từ Edge Impulse.
- Cung cấp script thử nghiệm với webcam để kiểm tra mô hình trên máy tính.

## Cấu trúc thư mục

```
helmet-detection/
├── ei-gtanoc-project-1-arduino-1.0.18-impulse-#1.zip   # Thư viện Arduino xuất từ Edge Impulse
├── helmet_detect.ino                                    # Sketch Arduino chạy mô hình nhận diện
├── webcam.py                                            # Script thử nghiệm nhận diện bằng webcam
├── model_info.json                                      # Thông tin mô hình (cấu hình, nhãn, ...)
├── FinalReport.docx                                     # Báo cáo cuối kỳ (bản Word)
├── FinalReport.pdf                                      # Báo cáo cuối kỳ (bản PDF)
└── README.md
```

| Tệp | Mô tả |
| --- | --- |
| `ei-gtanoc-project-1-arduino-*.zip` | Thư viện Arduino chứa mô hình đã huấn luyện (impulse #1) |
| `helmet_detect.ino` | Mã nguồn Arduino: đọc dữ liệu hình ảnh, chạy suy luận và xuất kết quả |
| `webcam.py` | Chạy nhận diện thời gian thực với webcam trên máy tính |
| `model_info.json` | Thông tin mô tả mô hình |
| `FinalReport.docx` / `.pdf` | Báo cáo chi tiết của dự án |

## Nguyên lý hoạt động

```
Thu thập ảnh → Huấn luyện (Edge Impulse) → Xuất thư viện Arduino → Triển khai lên board → Nhận diện
```

1. **Thu thập và gán nhãn dữ liệu** ảnh người có và không đội mũ bảo hiểm.
2. **Huấn luyện mô hình** trên Edge Impulse (impulse #1) và tối ưu để chạy trên thiết bị nhúng.
3. **Xuất thư viện Arduino** và nạp cùng sketch `helmet_detect.ino`.
4. **Suy luận (inference):** board nhận ảnh đầu vào, chạy mô hình và cho ra nhãn kèm độ tin cậy.

## Yêu cầu

**Phần cứng**

- Board **ESP32-S3** kèm camera: `[ghi rõ module, ví dụ ESP32-S3-EYE, XIAO ESP32S3 Sense, ESP32-S3-CAM, ...]`
- Cáp USB để cấp nguồn và nạp chương trình
- Webcam (nếu chạy `webcam.py` trên máy tính)

**Phần mềm**

- [Arduino IDE](https://www.arduino.cc/en/software)
- Python 3.8 trở lên (để chạy `webcam.py`)
- Các thư viện Python mà `webcam.py` sử dụng, ví dụ: `[opencv-python, numpy, edge_impulse_linux, ...]`

> Cập nhật lại hai mục có dấu `[...]` cho đúng với thiết bị và thư viện bạn đã dùng.

## Hướng dẫn sử dụng

### 1. Tải mã nguồn

```bash
git clone https://github.com/giatan0601/helmet-detection.git
cd helmet-detection
```

### 2. Nạp lên ESP32-S3

1. Mở **Arduino IDE**.
2. Cài gói board ESP32 (nếu chưa có):
   - Vào **File → Preferences**, thêm đường dẫn sau vào ô *Additional boards manager URLs*:
     ```
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```
   - Vào **Tools → Board → Boards Manager**, tìm `esp32` của *Espressif Systems* và nhấn **Install**.
3. Vào **Sketch → Include Library → Add .ZIP Library...** và chọn tệp `ei-gtanoc-project-1-arduino-1.0.18-impulse-#1.zip`.
4. Mở tệp `helmet_detect.ino`.
5. Cấu hình board trong menu **Tools**:
   - **Board:** `ESP32S3 Dev Module` (hoặc đúng tên board nếu bạn dùng board có sẵn trong danh sách, ví dụ `XIAO_ESP32S3`)
   - **PSRAM:** bật `OPI PSRAM` hoặc `QSPI PSRAM` tùy board (cần khi dùng camera)
   - **USB CDC On Boot:** `Enabled` nếu muốn xem Serial qua cổng USB native
   - **Port:** chọn cổng COM của board
6. Nhấn **Upload**. Nếu không nạp được, giữ nút **BOOT**, nhấn **RESET** rồi thả ra để vào chế độ nạp.
7. Mở **Serial Monitor** (đặt baud rate theo cấu hình trong sketch) để xem kết quả nhận diện.

> Các thiết lập ở bước 5 có thể khác nhau tùy board, bạn đối chiếu lại với `helmet_detect.ino` và tài liệu của board bạn dùng.

### 3. Chạy thử với webcam

Cài thư viện cần thiết:

```bash
pip install -r requirements.txt   # hoặc cài thủ công các thư viện mà webcam.py import
```

Chạy script:

```bash
python webcam.py
```

Nhấn `q` (hoặc `Esc`) để thoát, nếu script có hỗ trợ.

## Kết quả

| Chỉ số | Giá trị |
| --- | --- |
| Độ chính xác (accuracy) | `[?]%` |
| Số nhãn | `[?]` (ví dụ: `helmet`, `no_helmet`) |
| Kích thước mô hình | `[?]` KB |
| Thời gian suy luận | `[?]` ms |

<!-- Thêm ảnh demo hoặc GIF vào đây -->
<!-- ![Demo](images/demo.png) -->

## Báo cáo

Chi tiết về dữ liệu, phương pháp huấn luyện, đánh giá và hạn chế của hệ thống xem tại:

- [FinalReport.pdf](FinalReport.pdf)
- [FinalReport.docx](FinalReport.docx)

## Tác giả

- **giatan0601** – [GitHub](https://github.com/giatan0601)

## Giấy phép

`[Chọn giấy phép, ví dụ MIT]` – xem tệp `LICENSE` nếu có.
