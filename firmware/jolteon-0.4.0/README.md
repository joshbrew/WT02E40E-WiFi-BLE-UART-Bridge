# Jolteon WT02E40E firmware 0.4.0

`merged.hex` contains both application and network-core firmware with address metadata. Use the nRF5340 flashing workflow in `../../JOLTEON_UPDATE.md`. The separate `.bin` files are not standalone replacements for the merged HEX.

Rebuilt from upstream d522312 with the reviewed Jolteon patch and UART RX=P1.05 overlay. No hardware was flashed. See BUILD_MANIFEST.json and SHA256SUMS.txt for provenance.
