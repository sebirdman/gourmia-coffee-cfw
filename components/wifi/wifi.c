#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/apps/sntp.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include "hardware/wifi.h"

#define EXAMPLE_ESP_WIFI_MODE_AP CONFIG_ESP_WIFI_MODE_AP //TRUE:AP FALSE:STA
#define EXAMPLE_MAX_STA_CONN CONFIG_MAX_STA_CONN

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "WIFI";

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void prv_obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    setenv("TZ", "PST+8", 1);
    tzset();

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

// The BSSID is normally the MAC address of the router being talked through.
// In this case, I am misusing it as a means to identify that this is a
// control packet. A router with the "wrong" BSSID may cause issues, but there
// are 16 million addresses, so the chance of a collision is small
#define DEFAULT_ID 0x01, 0x02, 0x03, 0x04, 0x05, 0x06

typedef struct
{
    void *payload;
    uint32_t length;
    uint32_t seconds;
    uint32_t microseconds;
} sniffer_packet_into_t;

uint8_t packet_header[] = {
    0x08, 0x03, // Data packet (normal subtype)
    0x00, 0x00, // Duration and ID
    DEFAULT_ID, // Recipient Address
    DEFAULT_ID, // Transmitter Address
    DEFAULT_ID, // BSSID Address
    0x00, 0x00, // Sequence Control

    /* Data gets substituted here */
    /* Then the ESP automatically adds the checksum for us */
};

#define ID_OFFSET 4
#define ID_LENGTH 6
#define TRANCEIVER_MAX_PACKET_BYTES 64

typedef struct
{
    unsigned frame_ctrl : 16;
    unsigned duration_id : 16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl : 16;
    uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static void promisc(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)recv_buf;

    // Pointer to where the actual 802.11 packet is within the structure
    const wifi_ieee80211_packet_t *pk =
        (wifi_ieee80211_packet_t *)ppkt->payload;

    const wifi_ieee80211_mac_hdr_t *hdr = &pk->hdr;

    if ((hdr->addr2[4] == 0x17) && (hdr->addr2[5] == 0x64))
    {

        printf("PACKET CHAN=%02d, RSSI=%02d "
               " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x,"
               " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
               " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
               ppkt->rx_ctrl.channel,
               ppkt->rx_ctrl.rssi,
               /* ADDR1 */
               hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
               hdr->addr1[3], hdr->addr1[4], hdr->addr1[5],
               /* ADDR2 */
               hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
               hdr->addr2[3], hdr->addr2[4], hdr->addr2[5],
               /* ADDR3 */
               hdr->addr3[0], hdr->addr3[1], hdr->addr3[2],
               hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);

        uint16_t data_len = ppkt->rx_ctrl.legacy_length - sizeof(packet_header) - 4;
        printf("Size %i\n", ppkt->rx_ctrl.legacy_length);

        uint8_t *raw_packet = malloc(data_len);

        memcpy(
            raw_packet,
            (ppkt->payload) + sizeof(packet_header),
            data_len);

        for (int i = 0; i < data_len; i++)
        {
            printf("%02X", raw_packet[i]);
        }
        printf("\n");

        free(raw_packet);
    }
}

static void setup_wifi_promisc()
{
    esp_wifi_set_promiscuous_rx_cb(&promisc);
    esp_wifi_set_promiscuous(true);
}

static esp_err_t prv_wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGE(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        prv_obtain_time();
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGE(TAG, "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGE(TAG, "station:" MACSTR "leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(prv_wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    setup_wifi_promisc();

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS},
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    esp_wifi_connect();

    ESP_LOGE(TAG, "wifi_init_sta finished.");
    ESP_LOGE(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}