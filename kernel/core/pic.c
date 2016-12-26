#include "pic.h"
#include "hal.h"
#include "stdint.h"

#define PIC1_REG_COMMAND      0x20
#define PIC1_REG_STATUS       0x20
#define PIC1_REG_DATA         0x21
#define PIC1_REG_IMR          0x21

#define PIC2_REG_COMMAND      0xA0
#define PIC2_REG_STATUS       0xA0
#define PIC2_REG_DATA         0xA1
#define PIC2_REG_IMR          0xA1

#define PIC_ICW1_MASK_IC4     0x1   // Expect ICW 4 bit
#define PIC_ICW1_MASK_SNGL    0x2   // single or cascaded
#define PIC_ICW1_MASK_ADI     0x4   // Call address interval
#define PIC_ICW1_MASK_LTIM    0x8   // operation mode
#define PIC_ICW1_MASK_INIT    0x10  // initialization

#define PIC_ICW1_IC4_EXPECT   1
#define PIC_ICW1_IC4_NO       0
#define PIC_ICW1_SNGL_YES     2
#define PIC_ICW1_SNGL_NO      0
#define PIC_ICW1_ADI_CALLINTERVAL4    4
#define PIC_ICW1_ADI_CALLINTERVAL8    0
#define PIC_ICW1_LTIM_LEVELTRIGGERED  8
#define PIC_ICW1_LTIM_EDGETRIGGERED   0
#define PIC_ICW1_INIT_YES     0x10
#define PIC_ICW1_INIT_NO      0


#define PIC_ICW4_MASK_UPM     0x1   // Mode
#define PIC_ICW4_MASK_AEOI    0x2   // Automatic EOI
#define PIC_ICW4_MASK_MS      0x4   // Selects buffer type
#define PIC_ICW4_MASK_BUF     0x8   // Buffered mode
#define PIC_ICW4_MASK_SFNM    0x10  // Special fully-nested mode

#define PIC_ICW4_UPM_86MODE       1
#define PIC_ICW4_UPM_MCSMODE      0
#define PIC_ICW4_AEOI_AUTOEOI     2
#define PIC_ICW4_AEOI_NOAUTOEOI   0
#define PIC_ICW4_MS_BUFFERMASTER  4
#define PIC_ICW4_MS_BUFFERSLAVE   0
#define PIC_ICW4_BUF_MODEYES      8
#define PIC_ICW4_BUF_MODENO       0
#define PIC_ICW4_SFNM_NESTEDMODE  0x10
#define PIC_ICW4_SFNM_NOTNESTED   0

inline void PicSendCommand(uint8_t command, uint8_t pic_number) {
  if (pic_number > 1) return;
  uint8_t reg = (pic_number == 1) ? PIC2_REG_COMMAND : PIC1_REG_COMMAND;
  WriteToIoPort(reg, command);
}

inline void PicSendData(uint8_t data, uint8_t pic_number) {
  if (pic_number > 1) return;
  uint8_t reg = (pic_number == 1) ? PIC2_REG_DATA : PIC1_REG_DATA;
  WriteToIoPort(reg, data);
}

inline uint8_t PicReadData(uint8_t pic_number) {
  if (pic_number > 1) return 0;
  uint8_t reg = (pic_number == 1) ? PIC2_REG_DATA : PIC1_REG_DATA;
  return ReadFromIoPort(reg);
}

void InitializePic(uint8_t base0, uint8_t base1) {
  uint8_t icw = 0;
  icw = (icw & ~PIC_ICW1_MASK_INIT) | PIC_ICW1_INIT_YES;
  icw = (icw & ~PIC_ICW1_MASK_IC4) | PIC_ICW1_IC4_EXPECT;

  PicSendCommand(icw, 0);
  PicSendCommand(icw, 1);

  PicSendData(base0, 0);
  PicSendData(base1, 1);

  PicSendData(0x04, 0);
  PicSendData(0x02, 1);

  icw = (icw & ~PIC_ICW4_MASK_UPM) | PIC_ICW4_UPM_86MODE;

  PicSendData(icw, 0);
  PicSendData(icw, 1);
}