# Hitec Receiver PPM firmware

Firmware for Hitec receivers to add an 8 channel PPM mode.

Caution: Use at your own risk. If you don't know what you are doing you could
potentially ruin your receiver.

## Notes

### Fuses

* E:FF, H:DD, L:FD
* 1111 1111
* 1101 1101
* 1111 1101

* SPIEN enabled
* BODLEVEL1 enabled
* CKSEL1 enabled

Running at 7.3728MHz.

### Serial in

#### 21 bytes in for standard data

* 2x  0xFF
* 18x 0x__ (9x uint16_t values)
* 1x  0xEE

#### 21 bytes in for fail-safe

* 1x  0xFF
* 1x  0xBB
* 18x 0x__ (9x uint16_t values)
* 1x  0xEE

### PWM - PPM

To enable PPM mode, connect the output pin on channel 1 to V+ before turning on.

### Channels

In PWM mode channels 1 to 6 are active. In PPM mode channels 1 to 8 are sent on channel 2.
