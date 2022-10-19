#include <stdio.h>
#include <string.h>
#include "mbusparser.h"
#include "esp_log.h"
#include "vec.h"

static const char *TAG = "ParseMbus";

extern struct MeterData parseMbusFrame(unsigned char *frame, int fLen);

int mbusParseFrame(unsigned char *vframe, int vframeLen){
    unsigned char frame_vec[vframeLen];
    memcpy(frame_vec, vframe, vframeLen);

    struct MeterData md = parseMbusFrame(frame_vec, vframeLen);

    ESP_LOGI(TAG, "activeImportWh: %d Valid: %d", md.activeImportWh, md.activeImportWhValid);
    return 0;
}