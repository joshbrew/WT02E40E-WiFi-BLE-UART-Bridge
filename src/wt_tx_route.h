#ifndef WT_TX_ROUTE_H
#define WT_TX_ROUTE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum wt_tx_origin { WT_TX_UART, WT_TX_BLE, WT_TX_WIFI };
enum wt_tx_state { WT_TX_SOURCE, WT_TX_UNAVAILABLE, WT_TX_NO_NOTIFY,
                   WT_TX_NO_TARGET, WT_TX_QUEUED, WT_TX_FAILED };
struct wt_tx_destination { enum wt_tx_state state; int error; };
struct wt_tx_result {
	struct wt_tx_destination uart, ble, wifi;
	unsigned accepted;
};
struct wt_tx_ops {
	bool ble_connected, ble_notify, wifi_ready;
	void *context;
	int (*uart_send)(void *context, const uint8_t *data, size_t len);
	int (*ble_send)(void *context, const uint8_t *data, size_t len);
	int (*wifi_send)(void *context, const char *ip, const char *port,
			 const uint8_t *data, size_t len);
};

int wt_tx_route_other(enum wt_tx_origin origin, const char *ip, const char *port,
		      const uint8_t *data, size_t len, const struct wt_tx_ops *ops,
		      struct wt_tx_result *result);
const char *wt_tx_state_text(enum wt_tx_state state);

#endif
