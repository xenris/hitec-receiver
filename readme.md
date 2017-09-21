# Hitec Receiver PPM firmware

## Notes

### Fuses

* E:FF, H:DD, L:FD
* 1111 1111
* 1101 1101
* 1111 1101

* SPIEN enabled
* BODLEVEL1 enabled
* CKSEL1 enabled

Running at either 4MHz or 8MHz.

### Serial in

#### 21 bytes in

* 2x  0xFF
* 18x 0x__ (9x uint16_t values)
* 1x  0xEE

### PWM - PPM

To enable PPM mode, ground the output pin on channel 1 before turning on.

### Channels

In PWM mode channels 1 to 6 are active. In PPM mode channels 1 to 8 are sent on channel 2.
