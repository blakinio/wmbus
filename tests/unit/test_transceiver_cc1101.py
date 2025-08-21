import pytest


class MockCC1101:
    def __init__(self):
        self.registers = {}
        self.strobes = []

    def write_register(self, address, value):
        self.registers[address] = value

    def read_register(self, address):
        return self.registers.get(address, 0)

    def strobe(self, command):
        self.strobes.append(command)

    def flush_rx_fifo_(self):
        self.strobe(0x3A)  # SFRX

    def setup_rf_settings_(self):
        self.write_register(0x00, 0x2E)  # IOCFG2
        self.write_register(0x02, 0x00)  # IOCFG0
        self.write_register(0x03, 0x00)  # FIFOTHR
        self.write_register(0x04, 0x54)  # SYNC1
        self.write_register(0x05, 0x3D)  # SYNC0
        self.write_register(0x06, 0x00)  # PKTLEN
        self.write_register(0x07, 0x00)  # PKTCTRL1
        self.write_register(0x08, 0x02)  # PKTCTRL0
        self.write_register(0x09, 0x00)  # ADDR
        self.write_register(0x0A, 0x00)  # CHANNR
        self.write_register(0x0B, 0x06)  # FSCTRL1
        self.write_register(0x0C, 0x00)  # FSCTRL0
        self.write_register(0x10, 0x8B)  # MDMCFG4
        self.write_register(0x11, 0xF8)  # MDMCFG3
        self.write_register(0x12, 0x13)  # MDMCFG2
        self.write_register(0x13, 0x22)  # MDMCFG1
        self.write_register(0x14, 0xF8)  # MDMCFG0
        self.write_register(0x15, 0x50)  # DEVIATN
        self.write_register(0x16, 0x07)  # MCSM2
        self.write_register(0x17, 0x30)  # MCSM1
        self.write_register(0x18, 0x18)  # MCSM0
        self.write_register(0x19, 0x16)  # FOCCFG
        self.write_register(0x1A, 0x6C)  # BSCFG
        self.write_register(0x1B, 0x43)  # AGCCTRL2
        self.write_register(0x1C, 0x40)  # AGCCTRL1
        self.write_register(0x1D, 0x91)  # AGCCTRL0
        self.write_register(0x21, 0x56)  # FREND1
        self.write_register(0x22, 0x10)  # FREND0
        self.write_register(0x23, 0xE9)  # FSCAL3
        self.write_register(0x24, 0x2A)  # FSCAL2
        self.write_register(0x25, 0x00)  # FSCAL1
        self.write_register(0x26, 0x1F)  # FSCAL0

    def restart_rx(self):
        self.strobe(0x36)  # SIDLE
        self.flush_rx_fifo_()
        self.strobe(0x34)  # SRX


@pytest.fixture
def radio():
    return MockCC1101()


def test_setup_rf_settings(radio):
    radio.setup_rf_settings_()
    expected = {
        0x00: 0x2E,
        0x02: 0x00,
        0x03: 0x00,
        0x04: 0x54,
        0x05: 0x3D,
        0x06: 0x00,
        0x07: 0x00,
        0x08: 0x02,
        0x09: 0x00,
        0x0A: 0x00,
        0x0B: 0x06,
        0x0C: 0x00,
        0x10: 0x8B,
        0x11: 0xF8,
        0x12: 0x13,
        0x13: 0x22,
        0x14: 0xF8,
        0x15: 0x50,
        0x16: 0x07,
        0x17: 0x30,
        0x18: 0x18,
        0x19: 0x16,
        0x1A: 0x6C,
        0x1B: 0x43,
        0x1C: 0x40,
        0x1D: 0x91,
        0x21: 0x56,
        0x22: 0x10,
        0x23: 0xE9,
        0x24: 0x2A,
        0x25: 0x00,
        0x26: 0x1F,
    }
    assert radio.registers == expected


def test_restart_rx_sequence(radio):
    radio.restart_rx()
    assert radio.strobes == [0x36, 0x3A, 0x34]

