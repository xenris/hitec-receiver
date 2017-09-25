#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "eeprom.hpp"

template <class Clock, class Usart, uint32_t CpuFreq, uint32_t Baud>
struct Serial : nbavr::Task<Clock> {
    static constexpr uint8_t NumChannels = 9;
    static constexpr uint8_t StartCode = 0xFF;
    static constexpr uint8_t PositionCode = 0xFF;
    static constexpr uint8_t FailsafeCode = 0xDD;
    static constexpr uint8_t EndCode = 0xEE;

    uint16_t (&positions)[NumChannels];
    uint16_t (&positionsFailsafe)[NumChannels];
    bool& failsafeEnabled;
    bool& positionsUpdated;
    nbavr::Task<Clock>* servoTask;
    nbavr::Queue<uint8_t, 21> in;

    Serial(uint16_t (&positions)[NumChannels], uint16_t (&positionsFailsafe)[NumChannels], bool& failsafeEnabled, bool& positionsUpdated, nbavr::Task<Clock>* servoTask)
    : positions(positions), positionsFailsafe(positionsFailsafe), failsafeEnabled(failsafeEnabled), positionsUpdated(positionsUpdated), servoTask(servoTask) {
        const uint16_t ubrr = (CpuFreq / (16 * Baud)) - 1;

        block {
            Usart::baud(ubrr);
            Usart::use2X(false);
            Usart::characterSize(Usart::CharacterSize::Size8);
            Usart::receiverEnable(true);
            Usart::rxCompleteIntEnable(true);
            Usart::rxCompleteCallback(usartRxComplete, this);
        }
    }

    void loop() override {
        static uint8_t i = 200;
        static uint8_t p = 0;
        static bool f = false;

        uint8_t b = 0;

        if(in.pop(&b)) {
            if((b == PositionCode) && (p == StartCode)) {
                i = 0;

                if(f) {
                    saveFailsafeData(positionsFailsafe, failsafeEnabled);

                    f = false;
                }
            } else if((b == FailsafeCode) && (p == StartCode)) {
                i = 0;
                f = true;
            } else if(i < (2 * NumChannels)) {
                if(odd(i)) {
                    uint16_t w = (uint16_t(p) << 8) | b;
                    uint8_t v = i / 2;

                    if(v < NumChannels) {
                        atomic {
                            positions[v] = w;
                        }

                        if(f) {
                            positionsFailsafe[v] = w;
                        }
                    }
                }

                i++;
            } else if(i == (2 * NumChannels)) {
                if(b == EndCode) {
                    positionsUpdated = true;

                    servoTask->wake();
                }

                i = 200;
            }

            p = b;
        }
    }

    static force_inline bool odd(int8_t n) {
        return n & 1;
    }

    static void usartRxComplete(void* data) {
        Serial* self = (Serial*)data;

        uint8_t c = Usart::pop();

        self->in.push_(c);
    }

    // static void debug(uint8_t b) {
    //     atomic {
    //         for(int8_t j = 0; j < 8; j++) {
    //             block nbavr::PinC1::output(nbavr::Value::High);
    //
    //             if(b & 0x80) {
    //                 Clock::template delay<4000>();
    //             } else {
    //                 Clock::template delay<500>();
    //             }
    //
    //             block nbavr::PinC1::output(nbavr::Value::Low);
    //
    //             Clock::template delay<1000>();
    //
    //             b <<= 1;
    //         }
    //     }
    // }
};

#endif
