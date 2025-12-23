
#define F_CPU 1000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>

#include "RFID.h"


/*
 * H�m truy?n SPI
 * G?i m?t byte d? li?u qua SPI v� tr? v? byte nh?n ???c
 * @param cData: D? li?u c?n g?i
 * @return: D? li?u nh?n ???c
 */
uint8_t SPI_MasterTransmit(uint8_t cData)
{
    SPDR = cData;                       // ??a d? li?u v�o thanh ghi d? li?u SPI
    while(!(SPSR & (1<<SPIF)));         // ??i cho ??n khi truy?n xong
    return SPDR;                        // Tr? v? d? li?u nh?n ???c
}

/*
 * Kh?i t?o thi?t b? MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param sad: Ch�n Slave Address (SAD)
 * @param reset: Ch�n Reset
 */
void MFRC522_Init(MFRC522_Dev* dev, int sad, int reset) {
    dev->sad = sad;
    sbi(DDRB, dev->sad);       // Thi?t l?p ch�n digital l� OUTPUT ?? k?t n?i v?i ch�n RFID /ENABLE
    sbi(PORTB, dev->sad);      // ??t ch�n SAD l� HIGH (kh�ng k�ch ho?t)

    dev->reset = reset;
    sbi(DDRB, dev->reset);     // Thi?t l?p ch�n digital l� OUTPUT ?? k?t n?i v?i ch�n RFID /RESET
    sbi(PORTB, dev->reset);    // ??t ch�n RESET l� HIGH (kh�ng reset)
}

/*
 * L?y phi�n b?n firmware c?a MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @return: Phi�n b?n firmware
 */
uint8_t MFRC522_GetFirmwareVersion(MFRC522_Dev* dev) {
    uint8_t response;
    response = MFRC522_ReadRegister(dev, 0x37);  // ??c thanh ghi VersionReg
    return response;
}

/*
 * Ghi d? li?u v�o thanh ghi c?a MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param addr: ??a ch? thanh ghi
 * @param val: Gi� tr? c?n ghi
 */
void MFRC522_WriteRegister(MFRC522_Dev* dev, uint8_t addr, uint8_t val) {
    cbi(PORTB, dev->sad);                // K�ch ho?t giao ti?p SPI (SAD = LOW)

    // ??nh d?ng ??a ch?: 0XXXXXX0
    SPI_MasterTransmit((addr<<1)&0x7E); // G?i ??a ch? thanh ghi
    SPI_MasterTransmit(val);            // G?i gi� tr? c?n ghi
    
    sbi(PORTB, dev->sad);                // K?t th�c giao ti?p SPI (SAD = HIGH)
}

/*
 * ??c d? li?u t? thanh ghi c?a MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param addr: ??a ch? thanh ghi
 * @return: Gi� tr? ??c ???c
 */
uint8_t MFRC522_ReadRegister(MFRC522_Dev* dev, uint8_t addr) {
    uint8_t val;
    cbi(PORTB, dev->sad);                     // K�ch ho?t giao ti?p SPI (SAD = LOW)
    
    SPI_MasterTransmit(((addr<<1)&0x7E) | 0x80); // G?i ??a ch? thanh ghi v?i bit ??c (MSB = 1)
    val = SPI_MasterTransmit(0x00);           // G?i byte gi? ?? ??c d? li?u
    
    sbi(PORTB, dev->sad);                     // K?t th�c giao ti?p SPI (SAD = HIGH)
    return val;
}

/*
 * Reset module MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 */
void MFRC522_Reset(MFRC522_Dev* dev) {
    MFRC522_WriteRegister(dev, CommandReg, MFRC522_SOFTRESET);  // G?i l?nh reset m?m
}

/*
 * Thi?t l?p c�c bit ???c ch? ??nh b?i mask
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param addr: ??a ch? thanh ghi
 * @param mask: M?t n? bit c?n thi?t l?p
 */
void MFRC522_SetBitMask(MFRC522_Dev* dev, uint8_t addr, uint8_t mask) {
    uint8_t current;
    current = MFRC522_ReadRegister(dev, addr);  // ??c gi� tr? hi?n t?i
    MFRC522_WriteRegister(dev, addr, current | mask);  // Thi?t l?p c�c bit
}

/*
 * X�a c�c bit ???c ch? ??nh b?i mask
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param addr: ??a ch? thanh ghi
 * @param mask: M?t n? bit c?n x�a
 */
void MFRC522_ClearBitMask(MFRC522_Dev* dev, uint8_t addr, uint8_t mask) {
    uint8_t current;
    current = MFRC522_ReadRegister(dev, addr);  // ??c gi� tr? hi?n t?i
    MFRC522_WriteRegister(dev, addr, current & (~mask));  // X�a c�c bit
}

/*
 * Kh?i t?o module MFRC522
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 */
void MFRC522_Begin(MFRC522_Dev* dev) {
    sbi(PORTB, dev->sad);  // ??m b?o ch�n SAD l� HIGH ban ??u

    MFRC522_Reset(dev);    // Reset module

    // B? ??nh th?i: TPrescaler*TreloadVal/6.78MHz = 24ms == 3390 * 48 / 6.78
    MFRC522_WriteRegister(dev, TModeReg, 0x8D);       // Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    MFRC522_WriteRegister(dev, TPrescalerReg, 0x3E);  // TModeReg[3..0] + TPrescalerReg
    MFRC522_WriteRegister(dev, TReloadRegL, 30);      // 48
    MFRC522_WriteRegister(dev, TReloadRegH, 0);

    MFRC522_WriteRegister(dev, TxAutoReg, 0x40);      // 100%ASK
    MFRC522_WriteRegister(dev, ModeReg, 0x3D);        // Gi� tr? kh?i t?o CRC l� 0x6363

    MFRC522_SetBitMask(dev, TxControlReg, 0x03);      // B?t ?ng-ten
}

/*
 * G?i l?nh ??n th? RFID
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param cmd: L?nh c?n g?i
 * @param data: D? li?u c?n g?i
 * @param dlen: ?? d�i d? li?u g?i
 * @param result: N?i l?u k?t qu? nh?n ???c
 * @param rlen: Con tr? ??n bi?n l?u ?? d�i d? li?u nh?n
 * @return: Tr?ng th�i (MI_OK, MI_ERR, MI_NOTAGERR)
 */
uint8_t MFRC522_CommandTag(MFRC522_Dev* dev, uint8_t cmd, uint8_t *data, int dlen, uint8_t *result, int *rlen) {
    int status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits, n;
    int i;

    // Thi?t l?p c�c bit ng?t v� ng?t ch? t�y thu?c v�o l?nh
    switch (cmd) {
        case MFRC522_AUTHENT:       // L?nh x�c th?c
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        case MFRC522_TRANSCEIVE:    // L?nh truy?n v� nh?n
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        default:
            break;
    }

    MFRC522_WriteRegister(dev, CommIEnReg, irqEn|0x80);  // K�ch ho?t ng?t y�u c?u
    MFRC522_ClearBitMask(dev, CommIrqReg, 0x80);         // X�a t?t c? bit y�u c?u ng?t
    MFRC522_SetBitMask(dev, FIFOLevelReg, 0x80);         // FlushBuffer=1, kh?i t?o FIFO

    MFRC522_WriteRegister(dev, CommandReg, MFRC522_IDLE); // Kh�ng h�nh ??ng, h?y l?nh hi?n t?i

    // Ghi d? li?u v�o FIFO
    for (i=0; i < dlen; i++) {
        MFRC522_WriteRegister(dev, FIFODataReg, data[i]);
    }

    // Th?c thi l?nh
    MFRC522_WriteRegister(dev, CommandReg, cmd);
    if (cmd == MFRC522_TRANSCEIVE) {
        MFRC522_SetBitMask(dev, BitFramingReg, 0x80);  // StartSend=1, b?t ??u truy?n d? li?u
    }

    // ??i l?nh ho�n th�nh ?? nh?n d? li?u
    i = 25;  // Th?i gian ??i t?i ?a l� 25ms
    do {
        _delay_ms(5);
        // CommIRqReg[7..0]
        // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = MFRC522_ReadRegister(dev, CommIrqReg);
        i--;
    } while ((i!=0) && !(n&0x01) && !(n&waitIRq));  // Ki?m tra bit TimerIRq ho?c bit ng?t ch?

    MFRC522_ClearBitMask(dev, BitFramingReg, 0x80);  // StartSend=0

    if (i != 0) {  // Y�u c?u kh�ng h?t th?i gian ch?
        if(!(MFRC522_ReadRegister(dev, ErrorReg) & 0x1D)) {  // BufferOvfl Collerr CRCErr ProtocolErr
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;  // Kh�ng t�m th?y th?
            }

            if (cmd == MFRC522_TRANSCEIVE) {
                n = MFRC522_ReadRegister(dev, FIFOLevelReg);
                lastBits = MFRC522_ReadRegister(dev, ControlReg) & 0x07;
                if (lastBits) {
                    *rlen = (n-1)*8 + lastBits;  // T�nh s? bit nh?n ???c
                } else {
                    *rlen = n*8;  // T�nh s? bit nh?n ???c
                }

                if (n == 0) {
                    n = 1;
                }

                if (n > MAX_LEN) {
                    n = MAX_LEN;  // Gi?i h?n s? byte ??c
                }

                // ??c d? li?u nh?n ???c t? FIFO
                for (i=0; i<n; i++) {
                    result[i] = MFRC522_ReadRegister(dev, FIFODataReg);
                }
            }
        } else {
            status = MI_ERR;  // C� l?i
        }
    }
    return status;
}

/*
 * Y�u c?u th? RFID
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param mode: Ch? ?? y�u c?u (MF1_REQIDL ho?c MF1_REQALL)
 * @param data: N?i l?u d? li?u nh?n ???c
 * @return: Tr?ng th�i (MI_OK, MI_ERR, MI_NOTAGERR)
 */
uint8_t MFRC522_RequestTag(MFRC522_Dev* dev, uint8_t mode, uint8_t *data) {
    int status;
	int len = 0; // reset ?? d�i ph?n h?i
	
    MFRC522_WriteRegister(dev, BitFramingReg, 0x07);  // TxLastBists = BitFramingReg[2..0] = 7 bit

    data[0] = mode;
    status = MFRC522_CommandTag(dev, MFRC522_TRANSCEIVE, data, 1, data, &len);

    if ((status != MI_OK) || (len != 0x10)) {  // Ki?m tra k?t qu? (ph?i nh?n ???c 16 bit)
        status = MI_ERR;
    }

    return status;
}

/*
 * Ch?ng va ch?m - L?y ID c?a th?
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param serial: N?i l?u d? li?u ID th?
 * @return: Tr?ng th�i (MI_OK, MI_ERR, MI_NOTAGERR)
 */
uint8_t MFRC522_AntiCollision(MFRC522_Dev* dev, uint8_t *serial) {
    int status, i, len;
    uint8_t check = 0x00;

    MFRC522_WriteRegister(dev, BitFramingReg, 0x00);  // TxLastBits = BitFramingReg[2..0] = 0

    serial[0] = MF1_ANTICOLL;  // L?nh ch?ng va ch?m
    serial[1] = 0x20;          // ?�nh d?u bit
    status = MFRC522_CommandTag(dev, MFRC522_TRANSCEIVE, serial, 2, serial, &len);
    len = len / 8;  // Chuy?n ??i t? bit sang byte
    
    if (status == MI_OK) {
        // T?ng ki?m tra (checksum) c?a th? l� ph�p XOR c?a t?t c? c�c gi� tr?
        for (i = 0; i < len-1; i++) {
            check ^= serial[i];
        }
        // Checksum n�n gi?ng v?i gi� tr? cung c?p t? th? (serial[4])
        if (check != serial[i]) {
            status = MI_ERR;  // Checksum kh�ng kh?p
        }
    }

    return status;
}

/*
 * T�nh CRC
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param data: D? li?u ??u v�o
 * @param len: ?? d�i d? li?u ??u v�o
 * @param result: N?i l?u k?t qu? CRC (2 byte)
 */
void MFRC522_CalculateCRC(MFRC522_Dev* dev, uint8_t *data, int len, uint8_t *result) {
    int i;
    uint8_t n;

    MFRC522_ClearBitMask(dev, DivIrqReg, 0x04);   // CRCIrq = 0
    MFRC522_SetBitMask(dev, FIFOLevelReg, 0x80);  // X�a con tr? FIFO

    // Ghi d? li?u v�o FIFO
    for (i = 0; i < len; i++) {
        MFRC522_WriteRegister(dev, FIFODataReg, data[i]);
    }
    MFRC522_WriteRegister(dev, CommandReg, MFRC522_CALCCRC);  // G?i l?nh t�nh CRC

    // ??i cho vi?c t�nh to�n CRC ho�n th�nh
    i = 0xFF;
    do {
        n = MFRC522_ReadRegister(dev, DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));  // CRCIrq = 1 khi ho�n th�nh

    // ??c k?t qu? t? ph�p t�nh CRC
    result[0] = MFRC522_ReadRegister(dev, CRCResultRegL);  // Byte th?p
    result[1] = MFRC522_ReadRegister(dev, CRCResultRegM);  // Byte cao
}

/*
 * Ch?n th? RFID
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @param serial: M?ng ch?a ID th? (5 byte)
 * @return: SAK (Select Acknowledge) - byte x�c nh?n t? th?
 */
uint8_t MFRC522_SelectTag(MFRC522_Dev* dev, uint8_t *serial) {
    int i, status, len;
    uint8_t sak;
    uint8_t buffer[9];

    buffer[0] = MF1_SELECTTAG;  // L?nh ch?n th?
    buffer[1] = 0x70;           // ?�nh d?u bit
    
    // Sao ch�p ID th? v�o buffer
    for (i = 0; i < 5; i++) {
        buffer[i+2] = serial[i];
    }
    
    // Th�m 2 byte CRC v�o cu?i
    MFRC522_CalculateCRC(dev, buffer, 7, &buffer[7]);

    // G?i l?nh ch?n th?
    status = MFRC522_CommandTag(dev, MFRC522_TRANSCEIVE, buffer, 9, buffer, &len);

    // Ki?m tra k?t qu?
    if ((status == MI_OK) && (len == 0x18)) {
        sak = buffer[0];  // SAK l� byte ??u ti�n c?a ph?n h?i
    }
    else {
        sak = 0;  // C� l?i
    }

    return sak;
}

/*
 * D?ng th? RFID
 * @param dev: Con tr? ??n c?u tr�c thi?t b?
 * @return: Tr?ng th�i (MI_OK, MI_ERR, MI_NOTAGERR)
 */
uint8_t MFRC522_HaltTag(MFRC522_Dev* dev) {
    int status, len;
    uint8_t buffer[4];

    buffer[0] = MF1_HALT;  // L?nh d?ng th?
    buffer[1] = 0;         // ?�nh d?u bit
    
    // Th�m 2 byte CRC v�o cu?i
    MFRC522_CalculateCRC(dev, buffer, 2, &buffer[2]);
    
    // G?i l?nh d?ng th?
    status = MFRC522_CommandTag(dev, MFRC522_TRANSCEIVE, buffer, 4, buffer, &len);
    
    // T?t m� h�a
    MFRC522_ClearBitMask(dev, Status2Reg, 0x08);
    
    return status;
}

const uint8_t validUIDs[VALID_UID_COUNT][UID_LEN] = {
	{ 0x94, 0xE7, 0x34, 0x02 }, //ID user 1
	{ 0x33, 0x03, 0x89, 0x13 }, // ID user 2
	{ 0xC5, 0x57, 0xA5, 0xAC }	// ID user 3
};

char current_uid[UID_LEN*2 +1];

uint8_t RFID_IsValidUID(MFRC522_Dev *dev) {
	uint8_t status;
	uint8_t dataBuf[2];
	uint8_t serial[5];

	// 1. Kiểm tra xem có thẻ không
	status = MFRC522_RequestTag(dev, MF1_REQIDL, dataBuf);
	if (status != MI_OK) {
		return -1;   // Không có thẻ
	}

	// 2. Đọc UID
	status = MFRC522_AntiCollision(dev, serial);
	if (status != MI_OK) {
		return -1;
	}

	// 3. So sánh UID
	for (int i = 0; i < VALID_UID_COUNT; i++) {
		int match = 1;
		for (int j = 0; j < UID_LEN; j++) {
			if (serial[j] != validUIDs[i][j]) {
				match = 0;
				break;
			}
		}
		if (match) {
			MFRC522_HaltTag(dev);
			
			// Thuc hien luu hien luu ma the vao trong mang
			for (int k = 0; k < UID_LEN; k++) {
				sprintf(&current_uid[k * 2], "%02X", serial[k]);
			}
			current_uid[UID_LEN * 2] = '\0';  //them gia tri null vao cuoi chuoi
			return 1;
		}
	}

	// 4. Không khớp UID nào
	MFRC522_HaltTag(dev);
	return 0;
}

void SPI_Init(void) {
	// Thi?t l?p ch?n MOSI, SCK, SS l? output
	sbi(DDRB, MOSI_PIN);
	sbi(DDRB, SCK_PIN);
	sbi(DDRB, SS);           // ??m b?o SS l? OUTPUT ?? gi? ch? ?? Master
	// B?t SPI, Master mode, fosc/4
	SPCR = (1<<SPE) | (1<<MSTR);
}