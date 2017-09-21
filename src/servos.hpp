#ifndef SERVOS_HPP
#define SERVOS_HPP

#include <nbavr.hpp>

template <class Clock, class Channels, class Timer>
struct Servos : nbavr::Task<Clock> {
    uint16_t (&values)[9];
    bool ppmMode = false;
    int8_t currentChannel = 1;

    Servos(uint16_t (&values)[9]) : values(values) {
        // Check for pulldown plug on channel 1.

        block Channels::Ch1Pin::direction(nbavr::Direction::Output);
        block Channels::Ch1Pin::output(nbavr::Value::High);
        block Channels::Ch1Pin::direction(nbavr::Direction::Input);

        Clock::template delay<10000>();

        ppmMode = Channels::Ch1Pin::input() == nbavr::Value::Low;

        block Channels::Ch1Pin::pullup(false);

        // Set channels to ppm or pwm.

        if(ppmMode) {
            Channels::Ch1Pin::direction(nbavr::Direction::Input);
            Channels::Ch2Pin::direction(nbavr::Direction::Output);
            Channels::Ch3Pin::direction(nbavr::Direction::Input);
            Channels::Ch4Pin::direction(nbavr::Direction::Input);
            Channels::Ch5Pin::direction(nbavr::Direction::Input);
            Channels::Ch6Pin::direction(nbavr::Direction::Input);
            // Channels::Ch7Pin::direction(nbavr::Direction::Input);
            // Channels::Ch8Pin::direction(nbavr::Direction::Input);
            // Channels::Ch9Pin::direction(nbavr::Direction::Input);
        } else {
            Channels::Ch1Pin::direction(nbavr::Direction::Output);
            Channels::Ch2Pin::direction(nbavr::Direction::Output);
            Channels::Ch3Pin::direction(nbavr::Direction::Output);
            Channels::Ch4Pin::direction(nbavr::Direction::Output);
            Channels::Ch5Pin::direction(nbavr::Direction::Output);
            Channels::Ch6Pin::direction(nbavr::Direction::Output);
            // Channels::Ch7Pin::direction(nbavr::Direction::Output);
            // Channels::Ch8Pin::direction(nbavr::Direction::Output);
            // Channels::Ch9Pin::direction(nbavr::Direction::Output);
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
        atomic {
            currentChannel = 1;

            int16_t pulseTime = nbavr::clip(int16_t(values[0]), 6635, 15482);

            block Timer::OutputCompareA::value(pulseTime - 200);
            block Timer::OutputCompareB::value(pulseTime);

            block Timer::counter(0);

            block Timer::OutputCompareA::intFlagClear();
            block Timer::OutputCompareB::intFlagClear();

            block Timer::clock(Timer::Clock::Div1);

            if(ppmMode) {
                ppmPulse();
            } else {
                setChannelPin(1, nbavr::Value::High);
            }
        }

        this->sleep();
    }

    static void nearPulseCallback(void* data) {
        auto* self = (Servos*)data;

        while(!Timer::OutputCompareB::intFlag());

        block Timer::counter(0); // TODO Get hardware to do this.

        if(self->ppmMode) {
            ppmPulse();
        } else {
            setChannelPin(self->currentChannel, nbavr::Value::Low);
            setChannelPin(self->currentChannel + 1, nbavr::Value::High);
        }

        self->currentChannel++;

        if(self->currentChannel <= 8) {
            int16_t nextPulseTime = nbavr::clip(int16_t(self->values[self->currentChannel - 1]), 6635, 15482);

            block Timer::OutputCompareA::value(nextPulseTime - 200);
            block Timer::OutputCompareB::value(nextPulseTime);
        } else {
            block Timer::clock(Timer::Clock::None);

            self->currentChannel = 1;
        }

        block Timer::OutputCompareA::intFlagClear();
        block Timer::OutputCompareB::intFlagClear();
    }

    static force_inline void ppmPulse() {
        block setChannelPin(2, nbavr::Value::High);

        Clock::template delay<10000>();

        block setChannelPin(2, nbavr::Value::Low);
    }

    static force_inline void setChannelPin(int8_t channel, nbavr::Value value) {
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
