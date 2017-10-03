#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <nbavr.hpp>

void loadFailsafeData(uint16_t (&positions)[9], bool& enabled) {
    while(nbavr::hw::Eeprom::writeEnabled());

    block nbavr::hw::Eeprom::address(0x00);
    block nbavr::hw::Eeprom::readEnable();
    uint8_t l = nbavr::hw::Eeprom::data();
    block nbavr::hw::Eeprom::address(0x01);
    block nbavr::hw::Eeprom::readEnable();
    uint8_t h = nbavr::hw::Eeprom::data();

    if((h == 0xF0) && (l == 0x0D)) {
        for(int8_t i = 0; i < 9; i++) {
            block nbavr::hw::Eeprom::address(0x02 + i * 2);
            block nbavr::hw::Eeprom::readEnable();
            l = nbavr::hw::Eeprom::data();
            block nbavr::hw::Eeprom::address(0x02 + i * 2 + 1);
            block nbavr::hw::Eeprom::readEnable();
            h = nbavr::hw::Eeprom::data();

            positions[i] = (uint16_t(h) << 8) | l;
        }

        enabled = true;
    } else {
        enabled = false;
    }
}

void saveFailsafeData(uint16_t (&positions)[9], bool& enabled) {
    while(nbavr::hw::Eeprom::writeEnabled());

    block {
        nbavr::hw::Eeprom::address(0x00);
        nbavr::hw::Eeprom::data(0x0D);
    }

    atomic {
        block nbavr::hw::Eeprom::masterWriteEnable();
        block nbavr::hw::Eeprom::writeEnable();
    }

    while(nbavr::hw::Eeprom::writeEnabled());

    block {
        nbavr::hw::Eeprom::address(0x01);
        nbavr::hw::Eeprom::data(0xF0);
    }

    atomic {
        block nbavr::hw::Eeprom::masterWriteEnable();
        block nbavr::hw::Eeprom::writeEnable();
    }

    for(int8_t i = 0; i < 9; i++) {
        uint8_t h = positions[i] >> 8;
        uint8_t l = positions[i];

        while(nbavr::hw::Eeprom::writeEnabled());

        block {
            nbavr::hw::Eeprom::address(0x02 + i * 2);
            nbavr::hw::Eeprom::data(l);
        }

        block nbavr::hw::Eeprom::masterWriteEnable();
        block nbavr::hw::Eeprom::writeEnable();

        while(nbavr::hw::Eeprom::writeEnabled());

        block {
            nbavr::hw::Eeprom::address(0x02 + i * 2 + 1);
            nbavr::hw::Eeprom::data(h);
        }

        block nbavr::hw::Eeprom::masterWriteEnable();
        block nbavr::hw::Eeprom::writeEnable();
    }

    enabled = true;
}

#endif
