#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mbusparser.h"
#include "esp_log.h"

extern struct MeterData parseMbusFrame(unsigned char *frame, int fLen);
extern void mbus_data_tm_decode(struct tm *t, unsigned char *t_data, size_t t_data_size);
extern int PublishMqtt(struct MeterData md);

int mbusParseFrame(unsigned char *vframe, int vframeLen){
    unsigned char frame_vec[vframeLen];
    memcpy(frame_vec, vframe, vframeLen);

    struct MeterData md = parseMbusFrame(frame_vec, vframeLen);

    PublishMqtt(md);
    return 0;
}