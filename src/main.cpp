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
        using Ch1Pin = nbavr::hw::PinC1;
        using Ch2Pin = nbavr::hw::PinC0;
        using Ch3Pin = nbavr::hw::PinD4;
        using Ch4Pin = nbavr::hw::PinD3;
        using Ch5Pin = nbavr::hw::PinD2;
        using Ch6Pin = nbavr::hw::PinC2;
        // using Ch7Pin = nbavr::hw::PinC3;
        // using Ch8Pin = nbavr::hw::PinC4;
        // using Ch9Pin = nbavr::hw::PinC5;
    };

    using SerialUsart = nbavr::hw::Usart0;
    using SystemTimer = nbavr::hw::TimerCounter0;
    using ServoTimer = nbavr::hw::TimerCounter1;

    using Clock = nbavr::Clock<SystemTimer, CpuFreq>;

    static uint16_t positions[NumChannels];
    static uint16_t positionsFailsafe[NumChannels];
    static bool positionsUpdated = false;
    static bool failsafeEnabled = false;

    loadFailsafeData(positionsFailsafe, failsafeEnabled);

    static Servo<Clock, Channels, ServoTimer> servo(positions, positionsFailsafe, failsafeEnabled, positionsUpdated);
    static Serial<Clock, SerialUsart, CpuFreq, Baud> serial(positions, positionsFailsafe, failsafeEnabled, positionsUpdated, &servo);

    static nbavr::Task<Clock>* tasks[] = {&servo, &serial};

    nbavr::TaskManager<Clock> tm(tasks);
}
