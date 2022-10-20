#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mbusparser.h"
#include "esp_log.h"

static const char *TAG = "ParseMbus";

extern struct MeterData parseMbusFrame(unsigned char *frame, int fLen);
extern void mbus_data_tm_decode(struct tm *t, unsigned char *t_data, size_t t_data_size);

int mbusParseFrame(unsigned char *vframe, int vframeLen){
    char buf[25] = {"\000"};
    unsigned char frame_vec[vframeLen];
    memcpy(frame_vec, vframe, vframeLen);

    struct MeterData md = parseMbusFrame(frame_vec, vframeLen);

    ESP_LOGI(TAG, "activeImportWh: %d Valid: %d", md.activeImportWh, md.activeImportWhValid);
    strftime(buf, 25, "%Y-%m-%d %H:%M:%S", &md.DateTime);
    ESP_LOGI(TAG, "Timestamp: %s", buf);
    return 0;
}