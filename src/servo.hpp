#ifndef SERVO_HPP
#define SERVO_HPP

#include <nbavr.hpp>

template <class Clock, class Channels, class Timer>
struct Servo : nbavr::Task<Clock> {
    static constexpr uint8_t NumChannels = 9;
    static constexpr uint16_t PositionMin = 6635;
    static constexpr uint16_t PositionMax = 15482;
    static constexpr uint16_t PulsePreempt = 200;

    uint16_t (&positions)[NumChannels];
    uint16_t (&positionsFailsafe)[NumChannels];
    bool& failsafeEnabled;
    bool& positionsUpdated;
    bool ppmMode = false;
    int8_t currentChannel = 1;

    Servo(uint16_t (&positions)[NumChannels], uint16_t (&positionsFailsafe)[NumChannels], bool& failsafeEnabled, bool& positionsUpdated)
    : positions(positions), positionsFailsafe(positionsFailsafe), failsafeEnabled(failsafeEnabled), positionsUpdated(positionsUpdated) {
        // Check for pullup plug on channel 1.
        // Servos' signal leads are grounded.

        block Channels::Ch1Pin::direction(nbavr::hw::Direction::Output);
        block Channels::Ch1Pin::output(nbavr::hw::Value::Low);
        block Channels::Ch1Pin::direction(nbavr::hw::Direction::Input);

        Clock::template delay<1000>();

        ppmMode = Channels::Ch1Pin::input() == nbavr::hw::Value::High;

        // Set channels to ppm or pwm.

        if(ppmMode) {
            Channels::Ch1Pin::direction(nbavr::hw::Direction::Input);
            Channels::Ch2Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch3Pin::direction(nbavr::hw::Direction::Input);
            Channels::Ch4Pin::direction(nbavr::hw::Direction::Input);
            Channels::Ch5Pin::direction(nbavr::hw::Direction::Input);
            Channels::Ch6Pin::direction(nbavr::hw::Direction::Input);
            // Channels::Ch7Pin::direction(nbavr::hw::Direction::Input);
            // Channels::Ch8Pin::direction(nbavr::hw::Direction::Input);
            // Channels::Ch9Pin::direction(nbavr::hw::Direction::Input);
        } else {
            Channels::Ch1Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch2Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch3Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch4Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch5Pin::direction(nbavr::hw::Direction::Output);
            Channels::Ch6Pin::direction(nbavr::hw::Direction::Output);
            // Channels::Ch7Pin::direction(nbavr::hw::Direction::Output);
            // Channels::Ch8Pin::direction(nbavr::hw::Direction::Output);
            // Channels::Ch9Pin::direction(nbavr::hw::Direction::Output);
        }

        Timer::clock(Timer::Clock::None);
        Timer::waveform(Timer::Waveform::Normal);
        Timer::OutputCompareA::callback(nearPulseCallback, this);
        Timer::OutputCompareA::intEnable(true);
        Timer::OutputCompareB::intEnable(false);
        Timer::OutputCompareA::mode(Timer::OutputCompareA::Mode::Disconnected);
        Timer::OutputCompareB::mode(Timer::OutputCompareB::Mode::Disconnected);

        this->sleep();
    }

    // Serial will wake this task when there is new data, once every 20ms.
    // OutputCompareA is the indicator that the end of pulse is near.
    // OutputCompareB is the time when the pulse needs to end.
    void loop() override {
        if(!positionsUpdated && !failsafeEnabled) {
            this->sleep(Clock::millisToTicks(500));

            return;
        }

        if(!positionsUpdated) {
            nbavr::copy(positionsFailsafe, positions, NumChannels);
        }

        atomic {
            currentChannel = 1;

            int16_t pulseTime = nbavr::clip(positions[0], PositionMin, PositionMax);

            block Timer::OutputCompareA::value(pulseTime - PulsePreempt);
            block Timer::OutputCompareB::value(pulseTime);

            block Timer::counter(0);

            block Timer::OutputCompareA::intFlagClear();
            block Timer::OutputCompareB::intFlagClear();

            block Timer::clock(Timer::Clock::Div1);

            if(ppmMode) {
                ppmPulse();
            } else {
                setChannelPin(1, nbavr::hw::Value::High);
            }
        }

        if(positionsUpdated) {
            this->sleep(Clock::millisToTicks(500));

            positionsUpdated = false;
        } else {
            this->sleep(Clock::millisToTicks(20));
        }
    }

    static void nearPulseCallback(void* data) {
        auto* self = (Servo*)data;

        while(!Timer::OutputCompareB::intFlag());

        block Timer::counter(0); // TODO Get hardware to do this.

        if(self->ppmMode) {
            ppmPulse();
        } else {
            setChannelPin(self->currentChannel, nbavr::hw::Value::Low);
            setChannelPin(self->currentChannel + 1, nbavr::hw::Value::High);
        }

        self->currentChannel++;

        // Limited to NumChannels - 1 (8) Channels.
        if(self->currentChannel < NumChannels) {
            int16_t nextPulseTime = nbavr::clip(self->positions[self->currentChannel - 1], PositionMin, PositionMax);

            block Timer::OutputCompareA::value(nextPulseTime - PulsePreempt);
            block Timer::OutputCompareB::value(nextPulseTime);
        } else {
            block Timer::clock(Timer::Clock::None);

            self->currentChannel = 1;
        }

        block Timer::OutputCompareA::intFlagClear();
        block Timer::OutputCompareB::intFlagClear();
    }

    static force_inline void ppmPulse() {
        block setChannelPin(2, nbavr::hw::Value::High);

        Clock::template delay<10000>();

        block setChannelPin(2, nbavr::hw::Value::Low);
    }

    static force_inline void setChannelPin(int8_t channel, nbavr::hw::Value value) {
        switch(channel) {
        case 1:
            Channels::Ch1Pin::output(value);
            break;
        case 2:
            Channels::Ch2Pin::output(value);
            break;
        case 3:
            Channels::Ch3Pin::output(value);
            break;
        case 4:
            Channels::Ch4Pin::output(value);
            break;
        case 5:
            Channels::Ch5Pin::output(value);
            break;
        case 6:
            Channels::Ch6Pin::output(value);
            break;
        // case 7:
        //     Channels::Ch7Pin::output(value);
        //     break;
        // case 8:
        //     Channels::Ch8Pin::output(value);
        //     break;
        // case 9:
        //     Channels::Ch9Pin::output(value);
        //     break;
        }
    }
};

#endif
