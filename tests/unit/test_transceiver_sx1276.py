import pytest

class MockSX1276:
    def __init__(self):
        self.registers = {}
        self.writes = []

    def spi_write(self, address, value):
        self.registers[address] = value
        self.writes.append((address, value))

    def spi_read(self, address):
        return self.registers.get(address, 0)

    def restart_rx(self):
        # Standby mode
        self.spi_write(0x01, 0b001)
        # Clear FIFO
        self.spi_write(0x3F, 1 << 4)
        # Enable RX
        self.spi_write(0x01, 0b101)

    def get_rssi(self):
        rssi = self.spi_read(0x11)
        return -rssi // 2


def test_rssi_conversion():
    radio = MockSX1276()
    radio.spi_write(0x11, 80)
    assert radio.get_rssi() == -40


def test_restart_rx_sequence():
    radio = MockSX1276()
    radio.restart_rx()
    assert radio.writes == [
        (0x01, 0b001),
        (0x3F, 1 << 4),
        (0x01, 0b101),
    ]
