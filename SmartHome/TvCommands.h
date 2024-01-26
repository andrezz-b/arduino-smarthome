
#ifndef TVCommands_h
#define TVCommands_h

// Panasonic Commands
// Power button
#define CMD_POWER 0x3D
#define ADDR_POWER 0x8
// Channel forward
#define CMD_CHANNEL_FORWARD 0x34
#define ADDR_CHANNEL_FORWARD 0x8
// Channel back
#define CMD_CHANNEL_BACK 0x35
#define ADDR_CHANNEL_BACK 0x8
// Volume up
#define CMD_VOLUME_UP 0x21
#define ADDR_VOLUME_UP 0x8
// Volume down
#define CMD_VOLUME_DOWN 0x20
#define ADDR_VOLUME_DOWN 0x8
// Mute
#define CMD_MUTE 0x32
#define ADDR_MUTE 0x8
// Return
#define CMD_RETURN 0xD4
#define ADDR_RETURN 0x8
// Numbers, all commands are in order after 1, 2 is 0x11, 3 is 0x12..., address is 0x8 for all of them
#define CMD_ONE 0x10
#define ADDR_NUMBER 0x8
// Number 0
#define CMD_ZERO 0x19


#endif  // TVCommands_h
