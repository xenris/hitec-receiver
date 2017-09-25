#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <nbavr.hpp>

void loadFailsafeData(uint16_t (&positions)[9], bool& enabled) {
    while(nbavr::Eeprom::writeEnabled());

    block nbavr::Eeprom::address(0x00);
    block nbavr::Eeprom::readEnable();
    uint8_t l = nbavr::Eeprom::data();
    block nbavr::Eeprom::address(0x01);
    block nbavr::Eeprom::readEnable();
    uint8_t h = nbavr::Eeprom::data();

    if((h == 0xF0) && (l == 0x0D)) {
        for(int8_t i = 0; i < 9; i++) {
            block nbavr::Eeprom::address(0x02 + i * 2);
            block nbavr::Eeprom::readEnable();
            l = nbavr::Eeprom::data();
            block nbavr::Eeprom::address(0x02 + i * 2 + 1);
            block nbavr::Eeprom::readEnable();
            h = nbavr::Eeprom::data();

            positions[i] = (uint16_t(h) << 8) | l;
        }

        enabled = true;
    } else {
        enabled = false;
    }
}

void saveFailsafeData(uint16_t (&positions)[9], bool& enabled) {
    while(nbavr::Eeprom::writeEnabled());

    block {
        nbavr::Eeprom::address(0x00);
        nbavr::Eeprom::data(0x0D);
    }

    atomic {
        block nbavr::Eeprom::masterWriteEnable();
        block nbavr::Eeprom::writeEnable();
    }

    while(nbavr::Eeprom::writeEnabled());

    block {
        nbavr::Eeprom::address(0x01);
        nbavr::Eeprom::data(0xF0);
    }

    atomic {
        block nbavr::Eeprom::masterWriteEnable();
        block nbavr::Eeprom::writeEnable();
    }

    for(int8_t i = 0; i < 9; i++) {
        uint8_t h = positions[i] >> 8;
        uint8_t l = positions[i];

        while(nbavr::Eeprom::writeEnabled());

        block {
            nbavr::Eeprom::address(0x02 + i * 2);
            nbavr::Eeprom::data(l);
        }

        block nbavr::Eeprom::masterWriteEnable();
        block nbavr::Eeprom::writeEnable();

        while(nbavr::Eeprom::writeEnabled());

        block {
            nbavr::Eeprom::address(0x02 + i * 2 + 1);
            nbavr::Eeprom::data(h);
        }

        block nbavr::Eeprom::masterWriteEnable();
        block nbavr::Eeprom::writeEnable();
    }

    enabled = true;
}

#endif
