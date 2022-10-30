#include "esp_log.h"
#include "mqtt_client.h"
#include "mbusparser.h"

static const char *TAG = "MQTT";

// YOUR MQTT URL e.g.:
#define CONFIG_BROKER_URL "192.168.0.71"

extern struct MeterData parseMbusFrame(unsigned char *frame, int fLen);

esp_mqtt_client_handle_t MqttClient = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    /*
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    */
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC= %.*s\r\n", event->topic_len, event->topic);
        printf("DATA= %.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

int PublishMqtt(struct MeterData md) {
    char buf[30] = "\000";
    int msg_id = 0;

    if (md.activePowerPlusValid)
    {
        sprintf(buf, "%d",  md.activePowerPlus);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerPlus", buf, 0, 1, 1);
    }
    if (md.activePowerMinusValid)
    {
        sprintf(buf, "%d",  md.activePowerMinus);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerMinus", buf, 0, 1, 1);
    }
    if (md.activePowerPlusValidL1)
    {
        sprintf(buf, "%d",  md.activePowerPlusL1);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerPlusL1", buf, 0, 1, 0);
    }
    if (md.activePowerMinusValidL1)
    {
        sprintf(buf, "%d",  md.activePowerMinusL1);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerMinusL1", buf, 0, 1, 0);
    }
    if (md.activePowerPlusValidL2)
    {
        sprintf(buf, "%d",  md.activePowerPlusL2);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerPlusL2", buf, 0, 1, 0);
    }
    if (md.activePowerMinusValidL2)
    {
        sprintf(buf, "%d",  md.activePowerMinusL2);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerMinusL2", buf, 0, 1, 0);
    }
    if (md.activePowerPlusValidL3)
    {
    sprintf(buf, "%d",  md.activePowerPlusL3);
    msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerPlusL3", buf, 0, 1, 0);
    }
    if (md.activePowerMinusValidL3)
    {        
    sprintf(buf, "%d",  md.activePowerMinusL3);
    msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/activePowerMinusL3", buf, 0, 1, 0);
    }
    if (md.reactivePowerPlusValid)
    {
    sprintf(buf, "%d",  md.reactivePowerPlus);
    msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/reactivePowerPlus", buf, 0, 1, 0);
    }
    if (md.reactivePowerMinusValid)
    {
    sprintf(buf, "%d",  md.reactivePowerMinus);
    msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/reactivePowerMinus", buf, 0, 1, 0);
    }

    if (md.powerFactorValidL1)
    sprintf(buf, "%d",  md.powerFactorL1);
    msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/powerFactorL1", buf, 0, 1, 0);
    if (md.powerFactorValidL2)
    {
        sprintf(buf, "%d",  md.powerFactorL2);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/powerFactorL2", buf, 0, 1, 0);
    }
    if (md.powerFactorValidL3)
    {
        sprintf(buf, "%d",  md.powerFactorL3);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/powerFactorL3", buf, 0, 1, 0);
    }
    if (md.powerFactorTotalValid)
    {
        sprintf(buf, "%d",  md.powerFactorTotal);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/power/powerFactorTotal", buf, 0, 1, 0);
    }

    if (md.voltageL1Valid)
    {
        sprintf(buf, "%d",  md.voltageL1);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/voltage/L1", buf, 0, 1, 0);
    }
    if (md.voltageL2Valid)
    {
        sprintf(buf, "%d",  md.voltageL2);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/voltage/L2", buf, 0, 1, 0);
    }
    if (md.voltageL3Valid)
    {
        sprintf(buf, "%d",  md.voltageL3);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/voltage/L3", buf, 0, 1, 0);
    }

    if (md.centiAmpereL1Valid)
    {
        sprintf(buf, "%d",  md.centiAmpereL1 / 100);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/current/L1", buf, 0, 1, 0);
    }
    if (md.centiAmpereL2Valid)
    {
        sprintf(buf, "%d",  md.centiAmpereL2 / 100);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/current/L2", buf, 0, 1, 0);
    }
    if (md.centiAmpereL3Valid)
    {
        sprintf(buf, "%d",  md.centiAmpereL3 / 100);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/current/L3", buf, 0, 1, 0);
    }

    if (md.activeImportWhValid)
    {
        sprintf(buf, "%d",  md.activeImportWh / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeImportKWh", buf, 0, 1, 0);
    }
    if (md.activeExportWhValid)
    {
        sprintf(buf, "%d",  md.activeExportWh / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeExportKWh", buf, 0, 1, 0);
    }
    if (md.activeImportWhValidL1)
    {
        sprintf(buf, "%d",  md.activeImportWhL1 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeImportKWhL1", buf, 0, 1, 0);
    }
    if (md.activeExportWhValidL1)
    {
        sprintf(buf, "%d",  md.activeExportWhL1 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeExportKWhL1", buf, 0, 1, 0);
    }
    if (md.activeImportWhValidL2)
    {
        sprintf(buf, "%d",  md.activeImportWhL2 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeImportKWhL2", buf, 0, 1, 0);
    }
    if (md.activeExportWhValidL2)
    {
        sprintf(buf, "%d",  md.activeExportWhL2 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeExportKWhL2", buf, 0, 1, 0);
    }
    if (md.activeImportWhValidL3)
    {
        sprintf(buf, "%d",  md.activeImportWhL3 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeImportKWhL3", buf, 0, 1, 0);
    }
    if (md.activeExportWhValidL3)
    {
        sprintf(buf, "%d",  md.activeExportWhL3 / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/activeExportKWhL3", buf, 0, 1, 0);
    }

    if (md.reactiveImportWhValid)
    {
        sprintf(buf, "%d",  md.reactiveImportWh / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/reactiveImportKWh", buf, 0, 1, 0);
    }
    if (md.reactiveExportWhValid)
    {
        sprintf(buf, "%d",  md.reactiveExportWh / 1000);
        msg_id = esp_mqtt_client_publish(MqttClient, "/PowerMeter/energy/reactiveExportKWh", buf, 0, 1, 0);
    }

    return msg_id;
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .client_id = "PowerMeter",
        .username = "YOUR MQTT USER NAME",
        .password = "YOUR MQTT PASSWORD",
        .port = 1883,
        .host = CONFIG_BROKER_URL,
    };

    MqttClient = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(MqttClient, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(MqttClient);
}
