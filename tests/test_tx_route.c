#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "wt_tx_route.h"
struct fixture { unsigned uart, ble, wifi; int ble_error, wifi_error; };
static int uart(void *ctx, const uint8_t *data, size_t len) {
    struct fixture *f=ctx; assert(len==5 && !memcmp(data,"hello",5)); ++f->uart; return 0;
}
static int ble(void *ctx, const uint8_t *data, size_t len) {
    struct fixture *f=ctx; assert(len==5 && data); ++f->ble; return f->ble_error;
}
static int wifi(void *ctx, const char *ip, const char *port, const uint8_t *data, size_t len) {
    struct fixture *f=ctx; assert(!strcmp(ip,"192.168.1.8") && !strcmp(port,"5000") && data && len==5);
    ++f->wifi; return f->wifi_error;
}
static void reset(struct fixture *f) { memset(f,0,sizeof(*f)); }
int main(void) {
    struct fixture f={0}; struct wt_tx_result r;
    struct wt_tx_ops ops={true,true,true,&f,uart,ble,wifi};
    const uint8_t *data=(const uint8_t *)"hello";
    f.wifi_error=-EIO;
    assert(!wt_tx_route_other(WT_TX_UART,"192.168.1.8","5000",data,5,&ops,&r));
    assert(f.uart==0 && f.ble==1 && f.wifi==1 && r.accepted==1 && r.wifi.state==WT_TX_FAILED);
    reset(&f); f.ble_error=-EIO;
    assert(!wt_tx_route_other(WT_TX_UART,"192.168.1.8","5000",data,5,&ops,&r));
    assert(f.wifi==1 && r.ble.state==WT_TX_FAILED && r.wifi.state==WT_TX_QUEUED);
    reset(&f); f.wifi_error=-EIO;
    assert(!wt_tx_route_other(WT_TX_BLE,"192.168.1.8","5000",data,5,&ops,&r));
    assert(f.uart==1 && f.ble==0 && f.wifi==1 && r.ble.state==WT_TX_SOURCE);
    reset(&f);
    assert(!wt_tx_route_other(WT_TX_WIFI,"0.0.0.0","0",data,5,&ops,&r));
    assert(f.uart==1 && f.ble==1 && f.wifi==0 && r.accepted==2 && r.wifi.state==WT_TX_SOURCE);
    reset(&f); ops.ble_notify=false; ops.wifi_ready=false;
    assert(wt_tx_route_other(WT_TX_UART,"192.168.1.8","5000",data,5,&ops,&r)==-ENOTCONN);
    assert(!f.uart && !f.ble && !f.wifi && r.ble.state==WT_TX_NO_NOTIFY && r.wifi.state==WT_TX_UNAVAILABLE);
    ops.ble_connected=false; ops.wifi_ready=true;
    assert(wt_tx_route_other(WT_TX_UART,"0.0.0.0","0",data,5,&ops,&r)==-ENOTCONN);
    assert(r.ble.state==WT_TX_UNAVAILABLE && r.wifi.state==WT_TX_NO_TARGET);
    assert(wt_tx_route_other(WT_TX_UART,NULL,NULL,NULL,0,&ops,&r)==-EINVAL);
    puts("PASS: source exclusion, independent failures, BLE subscription gate, Wi-Fi target gate, invalid input");
    return 0;
}
