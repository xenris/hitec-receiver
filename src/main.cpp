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
    const uint32_t BaudRate = 115200;

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

    static uint16_t values[9] = {};

    static Servos<Clock, Channels, ServoTimer> servos(values);
    static Serial<Clock, SerialUsart, CpuFreq, BaudRate> serial(values, &servos);

    static nbavr::Task<Clock>* tasks[] = {&servos, &serial};

    nbavr::TaskManager<Clock> tm(tasks);
}
