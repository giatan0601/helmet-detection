import cv2
import serial
import time

# THAY DOI CONG COM PHU HOP VOI MAY TINH CUA BAN    
COM_PORT = 'COM3'
BAUD_RATE = 460800

CHUNK_SIZE = 256
CHUNK_DELAY = 0.001

ser = serial.Serial()
ser.port = COM_PORT
ser.baudrate = BAUD_RATE
ser.timeout = 0.05
ser.dtr = False
ser.rts = False

try:
    ser.open()
    print(f"[OK] Da ket noi voi ESP32 tai cong {COM_PORT}")
except Exception as e:
    print(f"[LOI] Khong the mo cong {COM_PORT}: {e}")
    print("Hay kiem tra xem Serial Monitor cua Arduino IDE da dong chua!")
    raise SystemExit(1)

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("[LOI] Khong the mo Webcam Laptop!")
    ser.close()
    raise SystemExit(1)

print("Dang cho tin hieu READY tu ESP32... (Neu lau qua, hay nhan nut RESET tren mach)")

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        h, w, _ = frame.shape
        min_dim = min(h, w)
        start_x = (w - min_dim) // 2
        start_y = (h - min_dim) // 2
        cropped = frame[start_y:start_y + min_dim, start_x:start_x + min_dim]

        cv2.imshow("Webcam Laptop (Nhan 'q' de thoat)", cropped)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        ready_dims = None
        channels = 3 # Mặc định là 3 kênh màu (RGB)

        while ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line:
                continue

            if line.startswith("READY"):
                parts = line.split(":")
                try:
                    ready_dims = (int(parts[1]), int(parts[2]))
                    # Đọc thêm thông số kênh màu nếu có
                    if len(parts) > 3:
                        channels = int(parts[3])
                except (IndexError, ValueError):
                    ready_dims = (96, 96)
            else:
                print(f"ESP32 -> {line}")

        if ready_dims is not None:
            target_w, target_h = ready_dims
            resized = cv2.resize(cropped, (target_w, target_h), interpolation=cv2.INTER_AREA)

            # Tự động chọn định dạng ảnh theo yêu cầu của mạch
            if channels == 1:
                # Đổi sang ảnh Xám (Grayscale) -> Dành cho model 1 kênh màu
                processed_frame = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
            else:
                # Đổi sang ảnh RGB -> Dành cho model 3 kênh màu
                processed_frame = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
                
            raw_bytes = processed_frame.tobytes()

            for i in range(0, len(raw_bytes), CHUNK_SIZE):
                ser.write(raw_bytes[i:i + CHUNK_SIZE])
                time.sleep(CHUNK_DELAY)
            ser.flush()

except KeyboardInterrupt:
    pass
finally:
    cap.release()
    cv2.destroyAllWindows()
    ser.close()