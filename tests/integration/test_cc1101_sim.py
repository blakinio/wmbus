import pytest

try:
    import cc1101
except Exception:  # pragma: no cover - library may not be present
    cc1101 = None


@pytest.mark.skipif(cc1101 is None, reason="CC1101 simulation not available")
def test_cc1101_receives_sample_frame():
    sim = cc1101.Simulator()
    sample = bytes.fromhex("AABBCCDD")
    sim.inject(sample)
    assert sim.receive() == sample
