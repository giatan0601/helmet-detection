#include <gtanoc-project-1_inferencing.h>
#include <ctype.h>
#include <string.h>

// ========================================================
// 1. CẤU HÌNH ĐÈN VÀ KHUNG HÌNH (DÙNG RGB)
// ========================================================
#define RED_LED_PIN   4
#define GREEN_LED_PIN 5

#define FRAME_WIDTH   EI_CLASSIFIER_INPUT_WIDTH
#define FRAME_HEIGHT  EI_CLASSIFIER_INPUT_HEIGHT

// RGB cố định 3 kênh màu (Red, Green, Blue)
#define FRAME_CHANNELS 3
#define FRAME_SIZE     (FRAME_WIDTH * FRAME_HEIGHT * FRAME_CHANNELS)

#define CONF_THRESHOLD 0.45f

// CẤP PHÁT TĨNH BỘ ĐỆM RGB888
static uint8_t rgb888_buf[FRAME_SIZE];

// ========================================================
// 2. HÀM PHỤ TRỢ: KIỂM TRA NHÃN KHÔNG ĐỘI MŨ
// ========================================================
bool is_no_helmet_label(const char *label) {
    char temp[70];
    size_t i = 0;
    
    // Chuẩn hóa chuỗi về chữ thường
    for (; label[i] != '\0' && i < sizeof(temp) - 1; i++) {
        temp[i] = tolower((unsigned char)label[i]);
    }
    temp[i] = '\0';

    // Tìm kiếm từ khóa không đội mũ
    if (strstr(temp, "without") != NULL || strstr(temp, "no") != NULL ||
        strstr(temp, "not") != NULL || strstr(temp, "khong") != NULL || strstr(temp, "ko") != NULL) {
        return true;
    }
    return false;
}

// ========================================================
// 3. HÀM CHUYỂN ĐỔI DỮ LIỆU PIXEL RGB SANG EDGE IMPULSE
// ========================================================
static int raw_feature_get_data(size_t offset, size_t out_len, float *signal_ptr) {
    size_t pixel_ix = offset * FRAME_CHANNELS;
    size_t out_ptr_ix = 0;

    while (out_len != 0) {
        // Lấy 3 byte màu R, G, B và đóng gói thành số 24-bit (0xRRGGBB)
        uint32_t r = rgb888_buf[pixel_ix];
        uint32_t g = rgb888_buf[pixel_ix + 1];
        uint32_t b = rgb888_buf[pixel_ix + 2];

        // Edge Impulse yêu cầu dữ liệu ảnh RGB dạng float chứa packed integer 0xRRGGBB
        signal_ptr[out_ptr_ix] = (float)((r << 16) | (g << 8) | b);

        pixel_ix += 3;
        out_ptr_ix++;
        out_len--;
    }
    return 0;
}

// ========================================================
// 4. KHỞI TẠO (SETUP)
// ========================================================
void setup() {
    Serial.setRxBufferSize(4096); // Mở rộng bộ đệm Serial nhận dữ liệu ảnh
    Serial.begin(460800);        // Tốc độ truyền dữ liệu cao
    delay(1000);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);

    Serial.println("\n=== HE THONG NHAN DIEN (EON COMPILER - RGB) ===");
    Serial.printf("Kich thuoc anh: %dx%d | Kenh mau: %d | Frame size: %d bytes\n", 
                  FRAME_WIDTH, FRAME_HEIGHT, FRAME_CHANNELS, FRAME_SIZE);
    Serial.printf("Free Heap: %u bytes\n", (unsigned)ESP.getFreeHeap());
}

// ========================================================
// 5. VÒNG LẶP CHÍNH (LOOP)
// ========================================================
void loop() {
    // Đọc bỏ dữ liệu dư thừa còn sót trên đường truyền
    while (Serial.available()) {
        Serial.read();
    }

    // Gửi tín hiệu báo cho máy tính/camera sẵn sàng nhận khung hình RGB mới
    Serial.printf("READY:%d:%d:%d\n", FRAME_WIDTH, FRAME_HEIGHT, FRAME_CHANNELS);

    size_t bytes_read = 0;
    unsigned long start_wait = millis();

    // Nhận đủ toàn bộ số bytes của khung hình (Width * Height * 3)
    while (bytes_read < FRAME_SIZE) {
        if (Serial.available()) {
            int c = Serial.read();
            if (c >= 0) {
                rgb888_buf[bytes_read++] = (uint8_t)c;
                start_wait = millis();
            }
        } else if (millis() - start_wait > 3000) { // Lỗi quá thời gian chờ (3 giây)
            if (bytes_read > 0) {
                Serial.printf("[LOI] Timeout! Nhan %u/%d bytes.\n", (unsigned)bytes_read, FRAME_SIZE);
            }
            return;
        }
    }

    // Cấu hình cấu trúc Signal để nạp vào Edge Impulse
    signal_t signal;
    signal.total_length = FRAME_WIDTH * FRAME_HEIGHT;
    signal.get_data = &raw_feature_get_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        Serial.printf("[LOI] Suy luan that bai (Loi RAM/Arena): %d\n", (int)res);
        delay(500);
        return;
    }

    int count_no_helmet = 0;
    int count_helmet = 0;

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
        auto bb = result.bounding_boxes[ix];
        if (bb.value < CONF_THRESHOLD) continue;

        if (is_no_helmet_label(bb.label)) {
            count_no_helmet++;
        } else {
            count_helmet++;
        }
        
        Serial.printf("Phat hien: %s (%.2f)\n", bb.label, bb.value);
    }
#endif

    // Điều khiển đèn tín hiệu dựa trên kết quả phát hiện
    if (count_no_helmet > 0) {
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
    } else if (count_helmet > 0) {
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
    } else {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
    }
}