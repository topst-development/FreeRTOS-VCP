#ifndef VCP_TYPES_HEADER
#define VCP_TYPES_HEADER

typedef struct {
    uint32 mId;
    uint8  data[2];
} VcpMessage_t;

enum VCP_IO_TYPE {
    VCP_IO_BREAK_LIGHT  = 0x101,
    VCP_IO_MOTOR_SPEED  = 0x102,
    VCP_IO_MOTOR_WHEEL  = 0x103,
    VCP_IO_EMER_SIGNAL  = 0x104,
    VCP_IO_FUEL_LEVEL   = 0x105,
    VCP_IO_TURN_SIGNAL  = 0x106,
    VCP_IO_HEAD_LIGHT   = 0x107
};

enum VCP_IO_ACTION {
    VCP_IO_ACTION_ON    = 0x01,
    VCP_IO_ACTION_OFF   = 0x02
};

enum VCP_IO_SUBTYPE {
    VCP_IO_SUB_LEFT     = 0x01,
    VCP_IO_SUB_RIGHT    = 0x02
};

#endif
