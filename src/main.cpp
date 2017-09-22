#include "main.hpp"

INCLUDE_DEFAULT_CALLBACK();
INCLUDE_TIMERCOUNTER_OUTPUT_CALLBACK(0, A);
INCLUDE_TIMERCOUNTER_OUTPUT_CALLBACK(0, B);
INCLUDE_TIMERCOUNTER_OVERFLOW_CALLBACK(0);
INCLUDE_TIMERCOUNTER_OUTPUT_CALLBACK(1, A);
INCLUDE_USART_CALLBACK(0, RX);
INCLUDE_USART_CALLBACK(0, DE);

void main() {
    const uint32_t CpuFreq = 7372800;
    const uint32_t Baud = 115200;
    const uint8_t NumChannels = 9;

    struct Channels {
        using Ch1Pin = nbavr::PinC1;
        using Ch2Pin = nbavr::PinC0;
        using Ch3Pin = nbavr::PinD4;
        using Ch4Pin = nbavr::PinD3;
        using Ch5Pin = nbavr::PinD2;
        using Ch6Pin = nbavr::PinC2;
        // using Ch7Pin = nbavr::PinC3;
        // using Ch8Pin = nbavr::PinC4;
        // using Ch9Pin = nbavr::PinC5;
    };

    using SerialUsart = nbavr::Usart0;
    using SystemTimer = nbavr::TimerCounter0;
    using ServoTimer = nbavr::TimerCounter1;

    using Clock = nbavr::Clock<SystemTimer, CpuFreq>;

    static uint16_t positions[NumChannels];
    static uint16_t positionsFailsafe[NumChannels];
    static bool positionsUpdated = false;

    static Servo<Clock, Channels, ServoTimer> servo(positions, positionsFailsafe, positionsUpdated);
    static Serial<Clock, SerialUsart, CpuFreq, Baud> serial(positions, positionsFailsafe, positionsUpdated, &servo);

    static nbavr::Task<Clock>* tasks[] = {&servo, &serial};

    nbavr::TaskManager<Clock> tm(tasks);
}
