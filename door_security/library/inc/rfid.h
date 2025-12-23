/*
 * RFID.h
 *
 * Created: 7/31/2018 12:05:17 AM
 * Tác gi? g?c: Xiu
 * Chuy?n ??i sang C và b? sung chú thích ti?ng Vi?t
 */ 


#ifndef RFID_H_
#define RFID_H_



// Các macro ?? thi?t l?p và xóa bit
#define sbi(PORT,bit) {PORT |=(1<<bit);} // ??t m?t bit lên 1
#define cbi(PORT,bit) {PORT &=~(1<<bit);} // ??t m?t bit xu?ng 0

// ??nh ngh?a chân SPI
#define SCK_PIN  7  // Chân ??ng h? SPI
#define MOSI_PIN 5  // Chân Master Out Slave In
#define MISO_PIN 6  // Chân Master In Slave Out
#define SS       4  // Chân Slave Select

#define MAX_LEN 16  // ?? dài t?i ?a c?a m?t m?ng

// Các mã l?i c?a MF522 MFRC522
#define MI_OK       0  // Thành công
#define MI_NOTAGERR 1  // Không tìm th?y th?
#define MI_ERR      2  // L?i chung

// Các l?nh c?a MF522 (tham kh?o trang 70 trong datasheet)
#define MFRC522_IDLE       0x00  // Không hành ??ng; H?y l?nh hi?n t?i
#define MFRC522_MEM        0x01  // L?u 25 byte vào b? ??m n?i b?
#define MFRC522_GENID      0x02  // T?o ID ng?u nhiên 10 byte
#define MFRC522_CALCCRC    0x03  // Tính CRC ho?c t? ki?m tra
#define MFRC522_TRANSMIT   0x04  // Truy?n d? li?u
#define MFRC522_NOCMDCH    0x07  // Không thay ??i l?nh
#define MFRC522_RECEIVE    0x08  // Nh?n d? li?u
#define MFRC522_TRANSCEIVE 0x0C  // Truy?n và nh?n d? li?u
#define MFRC522_AUTHENT    0x0E  // Xác th?c khóa
#define MFRC522_SOFTRESET  0x0F  // Reset ph?n m?m

// Các l?nh c?a th? Mifare_One
#define MF1_REQIDL    0x26  // Tìm th? trong vùng ?ng-ten không vào tr?ng thái ngh?
#define MF1_REQALL    0x52  // Tìm t?t c? th? trong vùng ?ng-ten
#define MF1_ANTICOLL  0x93  // Ch?ng va ch?m
#define MF1_SELECTTAG 0x93  // Ch?n th?
#define MF1_AUTHENT1A 0x60  // Xác th?c khóa A
#define MF1_AUTHENT1B 0x61  // Xác th?c khóa B
#define MF1_READ      0x30  // ??c kh?i
#define MF1_WRITE     0xA0  // Ghi kh?i
#define MF1_DECREMENT 0xC0  // Gi?m giá tr?
#define MF1_INCREMENT 0xC1  // T?ng giá tr?
#define MF1_RESTORE   0xC2  // Chuy?n d? li?u kh?i vào b? ??m
#define MF1_TRANSFER  0xB0  // L?u d? li?u trong b? ??m
#define MF1_HALT      0x50  // Cho th? vào tr?ng thái ngh?


// Các thanh ghi c?a MFRC522
// Trang 0: L?nh và Tr?ng thái
#define Reserved00            0x00
#define CommandReg            0x01  // B?t ??u và d?ng l?nh
#define CommIEnReg            0x02  // Kích ho?t và vô hi?u hóa ng?t yêu c?u
#define DivIEnReg             0x03  // Kích ho?t và vô hi?u hóa ng?t
#define CommIrqReg            0x04  // Ng?t yêu c?u
#define DivIrqReg             0x05  // Ng?t
#define ErrorReg              0x06  // Các bit l?i hi?n th? l?i giao ti?p
#define Status1Reg            0x07  // Tr?ng thái giao ti?p
#define Status2Reg            0x08  // Tr?ng thái b? thu và truy?n
#define FIFODataReg           0x09  // D? li?u vào ra 64 byte FIFO
#define FIFOLevelReg          0x0A  // S? byte l?u trong FIFO
#define WaterLevelReg         0x0B  // M?c n??c cho FIFO
#define ControlReg            0x0C  // Các thi?t l?p ?a d?ng
#define BitFramingReg         0x0D  // ?i?u ch?nh bit frame
#define CollReg               0x0E  // Bit cho vi?c x? lý va ch?m
#define Reserved01            0x0F
// Trang 1: L?nh
#define Reserved10            0x10
#define ModeReg               0x11  // ??nh ngh?a ch? ?? truy?n và nh?n
#define TxModeReg             0x12  // ??nh ngh?a truy?n d? li?u
#define RxModeReg             0x13  // ??nh ngh?a nh?n d? li?u
#define TxControlReg          0x14  // ?i?u khi?n ?ng-ten
#define TxAutoReg             0x15  // ?i?u khi?n ?ng-ten
#define TxSelReg              0x16  // L?a ch?n ch?c n?ng truy?n
#define RxSelReg              0x17  // L?a ch?n ch?c n?ng nh?n
#define RxThresholdReg        0x18  // L?a ch?n ng??ng
#define DemodReg              0x19  // Cài ??t gi?i ?i?u ch?
#define Reserved11            0x1A
#define Reserved12            0x1B
#define MifareReg             0x1C  // ?i?u khi?n giao ti?p th? MIFARE
#define Reserved13            0x1D
#define Reserved14            0x1E
#define SerialSpeedReg        0x1F  // ?i?u khi?n t?c ?? UART
// Trang 2: C?u hình
#define Reserved20            0x20
#define CRCResultRegM         0x21  // K?t qu? CRC - byte cao nh?t
#define CRCResultRegL         0x22  // K?t qu? CRC - byte th?p nh?t
#define Reserved21            0x23
#define ModWidthReg           0x24  // Cài ??t ?? r?ng ?i?u ch?
#define Reserved22            0x25
#define RFCfgReg              0x26  // Cài ??t t?n s? thu
#define GsNReg                0x27  // Ch?n d?n ?i?n cho ?ng-ten
#define CWGsPReg              0x28  // Ch?n d?n ?i?n cho p-driver
#define ModGsPReg             0x29  // Ch?n d?n ?i?n cho p-driver
#define TModeReg              0x2A  // Cài ??t cho b? ??m th?i gian n?i
#define TPrescalerReg         0x2B  // Cài ??t b? chia b? ??m th?i gian n?i
#define TReloadRegH           0x2C  // Giá tr? 16 bit cao ?? n?p l?i
#define TReloadRegL           0x2D  // Giá tr? 16 bit th?p ?? n?p l?i
#define TCounterValueRegH     0x2E  // Hi?n th? giá tr? 16 bit cao hi?n t?i
#define TCounterValueRegL     0x2F  // Hi?n th? giá tr? 16 bit th?p hi?n t?i
// Trang 3: Thanh ghi ki?m tra
#define Reserved30            0x30
#define TestSel1Reg           0x31
#define TestSel2Reg           0x32
#define TestPinEnReg          0x33
#define TestPinValueReg       0x34
#define TestBusReg            0x35
#define AutoTestReg           0x36
#define VersionReg            0x37  // Hi?n th? phiên b?n c?a MFRC522
#define AnalogTestReg         0x38
#define TestDAC1Reg           0x39
#define TestDAC2Reg           0x3A
#define TestADCReg            0x3B
#define Reserved31            0x3C
#define Reserved32            0x3D
#define Reserved33            0x3E
#define Reserved34            0x3F

// Khai báo các hàm c?a th? vi?n RFID
// L?u ý: Trong C, không có ph??ng th?c c?a l?p nh? C++, 
// chúng ta chuy?n sang các hàm thông th??ng v?i tham s? MFRC522

// Khai báo c?u trúc cho thi?t b? MFRC522
typedef struct {
    int sad;    // Pin Slave Address (SAD) - còn g?i là Chip Select (CS)
    int reset;  // Pin Reset
} MFRC522_Dev;

// Kh?i t?o thi?t b? MFRC522
void MFRC522_Init(MFRC522_Dev* dev, int sad, int reset);

// Ghi giá tr? vào thanh ghi
void MFRC522_WriteRegister(MFRC522_Dev* dev, uint8_t addr, uint8_t val);

// ??c giá tr? t? thanh ghi
uint8_t MFRC522_ReadRegister(MFRC522_Dev* dev, uint8_t addr);

// Thi?t l?p bit mask
void MFRC522_SetBitMask(MFRC522_Dev* dev, uint8_t addr, uint8_t mask);

// Xóa bit mask
void MFRC522_ClearBitMask(MFRC522_Dev* dev, uint8_t addr, uint8_t mask);

// Kh?i t?o module MFRC522
void MFRC522_Begin(MFRC522_Dev* dev);

// Reset module
void MFRC522_Reset(MFRC522_Dev* dev);

// L?y phiên b?n firmware
uint8_t MFRC522_GetFirmwareVersion(MFRC522_Dev* dev);

// G?i l?nh t?i th?
uint8_t MFRC522_CommandTag(MFRC522_Dev* dev, uint8_t command, uint8_t *data, 
                     int dlen, uint8_t *result, int *rlen);

// Yêu c?u th?
uint8_t MFRC522_RequestTag(MFRC522_Dev* dev, uint8_t mode, uint8_t *data);

// Ch?ng va ch?m
uint8_t MFRC522_AntiCollision(MFRC522_Dev* dev, uint8_t *serial);

// Tính CRC
void MFRC522_CalculateCRC(MFRC522_Dev* dev, uint8_t *data, int len, uint8_t *result);

// Ch?n th?
uint8_t MFRC522_SelectTag(MFRC522_Dev* dev, uint8_t *serial);

// D?ng th?
uint8_t MFRC522_HaltTag(MFRC522_Dev* dev);

// Truy?n nh?n SPI
uint8_t SPI_MasterTransmit(uint8_t cData);

// ?? dài UID và s? l??ng UID h?p l?
#define UID_LEN           4
#define VALID_UID_COUNT   3

// M?ng các UID h?p l? (extern ?? dùng toàn c?c)
extern const uint8_t validUIDs[VALID_UID_COUNT][UID_LEN];
extern char current_uid[UID_LEN*2 +1];

// Prototype hàm ki?m tra UID
uint8_t RFID_IsValidUID(MFRC522_Dev *dev);

//Khoi tao SPI
void SPI_Init(void);


#endif /* RFID_H_ */