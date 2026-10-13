# Jolteon bridge update — 0.4.0-independent-routing

`tx both` / `tx all` now sends the payload to the two transports other than the
command's source. Each destination is independent: a Wi-Fi failure does not
prevent BLE or UART from receiving. BLE is eligible only with a connected client
and TX notifications enabled. Command responses still return to their caller.

| Command source | Payload destinations |
|---|---|
| UART | BLE and Wi-Fi UDP |
| BLE | UART and Wi-Fi UDP |
| Wi-Fi | UART and BLE |

The response reports each destination and the number accepted locally. It does
not acknowledge delivery at a remote peer. Wi-Fi UDP requires association, an
IPv4 address and a configured listening destination; UDP has no BLE-style client
connection indicator. `0.0.0.0 0` deliberately leaves that destination unset.

## Jolteon UART wiring

The supplied Nordic schematic connects SAMD PA22/TX1 to **nRF P1.05/RX**, and nRF
P1.04/TX to SAMD PA23/RX1. P1.06 is the separate PA15 IRQ net.
`boards/jolteon_uart.overlay` corrects RX for this board. The original default
`app.overlay` retains RX=P1.06 for the earlier bridge carrier.

## Build with the installed Windows Nordic SDK

```powershell
.\build_jolteon.ps1
```

The helper uses the installed NCS v3.3.1 and toolchain 936afb6332, builds both
application and Bluetooth network-core firmware through sysbuild, and adds the
Jolteon overlay after the normal overlay. Change those two SDK/toolchain paths
in the helper for another installation. `-Pristine` requests a clean rebuild.

Equivalent build from an initialized Nordic SDK terminal:

```text
west build --sysbuild -b nrf7002dk/nrf5340/cpuapp -d build/jolteon-routing . -- -DEXTRA_DTC_OVERLAY_FILE=boards/jolteon_uart.overlay
```

`build/jolteon-routing/merged.hex` contains both cores. The staged copy is
`firmware/jolteon-0.4.0/merged.hex`; consult its build manifest and SHA256SUMS.
Use the merged HEX with the existing nRF5340 flashing instructions. The separate
application `.bin` has no address metadata and does not include the network core.
The SAMD21 flashing batch/configuration is for the SAMD21, not this module.

## SAMD21 verification fallback

The verification sketch is in
`C:\Users\brews\Documents\Arduino\SAMD21J18_Blinky\Jolteon_Feature_Verification`.
It uses USB as the primary console and queues reports for BLE when USB is absent.
It sends `wt tx both 0.0.0.0 0 "..."` by default. Configure the sketch's UDP target
with `report wifi <ip> <port>` to allow reports to both BLE and Wi-Fi.

Connect a BLE client, subscribe to TX notifications, and use the BLE command
characteristic to send `tx uart "j18 all"` or `tx uart "j18 summary"`.
Enable response notifications as well to see command acknowledgements. The web
console understands the bridge's stream framing for smaller negotiated MTUs.

Automatic module power in the sketch is gated by `NRF_POWER_PATH_VERIFIED` because
the supplied schematic/BOM show R20 as a 10-ohm shunt on VDD_NRF. Correct/confirm
the power circuit and RX wiring, then set the flag true and rebuild. The sketch
README explains the explicit USB `nrf on` route and report limitations.

## Validation

- Built both cores successfully with NCS v3.3.1 on October 13, 2026.
- Application memory usage and image hashes are recorded in the build manifest.
- Checked the generated devicetree for TX=P1.04 and RX=P1.05 in both pin states.
- Host C tests cover source exclusion, independent destination failures,
  disconnected/unsubscribed BLE, missing Wi-Fi target, and invalid input.
  Run `tests/run_route_tests.bat` with the installed VS 2022 Build Tools, or
  compile `tests/test_tx_route.c` plus `src/wt_tx_route.c` using a C11 compiler.
- Reapplied and rebuilt against upstream `d522312` after reviewing the stash;
  the upstream queues, streaming and scan implementation are preserved.
- No module was flashed and no physical BLE/Wi-Fi/UART transfer was tested here.
  The build manifest records the upstream commit and hashes of the modified sources.

On hardware, test each source with both destinations ready, then disconnect
one destination and repeat. Check the remaining destination still receives, the
response identifies the unavailable/failed one, and the payload is not echoed to
its source. Repeat BLE tests with TX subscription disabled. Verify a unique
payload at each receiver; a local `queued` response alone is insufficient.
