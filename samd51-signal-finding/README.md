# Spot Finding on SAMD51 MCU

Port of spot finding / signal pixel calculation from DIALS to MCU: typically this requires many 100s of MB of RAM, so porting to ~200kB is an interesting challenge. This will not be fast.

Preconditions:

- data unpacked from HDF5 files
- data rearranged to "stack" the modules i.e. as an array of `uint16_t` of width NX but height `NZ * NY`

With these in place we _may_ be able to make it work.
