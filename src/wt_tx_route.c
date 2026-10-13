#include "wt_tx_route.h"
#include <errno.h>
#include <string.h>

static void completed(struct wt_tx_result *result,
		      struct wt_tx_destination *destination, int error)
{
	destination->error = error;
	destination->state = error ? WT_TX_FAILED : WT_TX_QUEUED;
	if (!error) {
		result->accepted++;
	}
}

int wt_tx_route_other(enum wt_tx_origin origin, const char *ip, const char *port,
		      const uint8_t *data, size_t len, const struct wt_tx_ops *ops,
		      struct wt_tx_result *result)
{
	if (!data || !len || !ops || !result || origin > WT_TX_WIFI ||
	    origin < WT_TX_UART) {
		return -EINVAL;
	}
	memset(result, 0, sizeof(*result));
	result->uart.state = result->ble.state = result->wifi.state = WT_TX_SOURCE;

	if (origin != WT_TX_UART) {
		completed(result, &result->uart,
			  ops->uart_send ? ops->uart_send(ops->context, data, len) : -ENOSYS);
	}
	if (origin != WT_TX_BLE) {
		if (!ops->ble_connected) {
			result->ble.state = WT_TX_UNAVAILABLE;
		} else if (!ops->ble_notify) {
			result->ble.state = WT_TX_NO_NOTIFY;
		} else {
			completed(result, &result->ble,
				  ops->ble_send ? ops->ble_send(ops->context, data, len) : -ENOSYS);
		}
	}
	if (origin != WT_TX_WIFI) {
		if (!ops->wifi_ready) {
			result->wifi.state = WT_TX_UNAVAILABLE;
		} else if (!ip || !*ip || !port || !*port ||
			   !strcmp(ip, "0.0.0.0") || !strcmp(port, "0")) {
			result->wifi.state = WT_TX_NO_TARGET;
		} else {
			completed(result, &result->wifi, ops->wifi_send ?
				  ops->wifi_send(ops->context, ip, port, data, len) : -ENOSYS);
		}
	}
	/* Each destination is independent, including races after readiness checks. */
	return result->accepted ? 0 : -ENOTCONN;
}

const char *wt_tx_state_text(enum wt_tx_state state)
{
	switch (state) {
	case WT_TX_SOURCE: return "source";
	case WT_TX_UNAVAILABLE: return "unavailable";
	case WT_TX_NO_NOTIFY: return "notify_off";
	case WT_TX_NO_TARGET: return "no_target";
	case WT_TX_QUEUED: return "queued";
	case WT_TX_FAILED: return "failed";
	default: return "invalid";
	}
}
