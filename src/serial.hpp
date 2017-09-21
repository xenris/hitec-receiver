#ifndef SERIAL_HPP
#define SERIAL_HPP

template <class Clock, class Usart, uint32_t CpuFreq, uint32_t Baud>
struct Serial : nbavr::Task<Clock> {
    static constexpr uint8_t MaxChannels = 9;

    uint16_t (&values)[9];
    nbavr::Task<Clock>* servos;
    nbavr::Queue<uint8_t, 10> out;
    nbavr::Queue<uint8_t, 21> in;

    Serial(uint16_t (&values)[9], nbavr::Task<Clock>* servos) : values(values), servos(servos) {
        const uint16_t ubrr = (CpuFreq / (16 * Baud)) - 1;

        block {
            Usart::baud(ubrr);
            Usart::use2X(false);
            Usart::characterSize(Usart::CharacterSize::Size8);
            // Usart::transmitterEnable(true);
            Usart::receiverEnable(true);
            Usart::rxCompleteIntEnable(true);
            Usart::rxCompleteCallback(usartRxComplete, this);
            // Usart::dataRegisterEmptyCallback(usartDataRegisterEmpty, this);
        }
    }

    void loop() override {
        static uint8_t i = 200;
        static uint8_t p = 0;

        uint8_t b = 0;

        if(in.pop(&b)) {
            if((b == 0xFF) && (p == 0xFF)) {
                i = 0;
            } else if(i < (2 * MaxChannels)) {
                if((i % 2) == 1) {
                    uint16_t w = (uint16_t(p) << 8) | b;
                    uint8_t v = i / 2;

                    if(v < 9) {
                        atomic {
                            values[v] = w;
                        }
                    }
                } else if(b == 0xEE) {
                    servos->wake();

                    i = 200;
                }

                i++;
            } else if(i == (2 * MaxChannels)) {
                servos->wake();

                i = 200;
            }

            p = b;
        }
    }

    static void usartRxComplete(void* data) {
        Serial* self = (Serial*)data;

        uint8_t c = Usart::pop();

        self->in.push_(c);
    }

    // static void usartDataRegisterEmpty(void* data) {
    //     Serial* self = (Serial*)data;
    //
    //     uint8_t d;
    //
    //     if(self->out.pop_(&d)) {
    //         Usart::push(d);
    //     } else {
    //         Usart::dataRegisterEmptyIntEnable(false);
    //     }
    // }

    // static void outNotify(void* data) {
    //     Usart::dataRegisterEmptyIntEnable(true);
    // }
};

#endif
