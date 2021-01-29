#pragma once

#include <cstdint>

namespace mod {

/// Checks if there is a USB Gecko on EXI channel `chan`
bool ugProbe(int chan);

/// Try to send `len` bytes from `data` via the USB Gecko on channel `chan`.
/// Returns number of bytes sent or negative on error.
int ugSend(int chan, const void *data, int len);

/// Try to receive `len` bytes into `data` via the USB Gecko on channel `chan`.
/// Returns number of bytes received or negative on error.
int ugRecv(int chan, void *data, int len);

}