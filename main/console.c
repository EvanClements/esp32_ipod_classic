#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "audio.h"
#include "esp_console.h"
#include "esp_log.h"
#include "ota.h"
#include "usb_msc.h"

static const char *TAG = "console";

static int cmd_list(int argc, char **argv)
{
    if (!usb_msc_app_owns_storage()) {
        printf("storage is owned by the USB host — unplug USB first\n");
        return 1;
    }
    const char *path = (argc > 1) ? argv[1] : USB_MSC_MOUNT_POINT;

    DIR *dir = opendir(path);
    if (!dir) {
        printf("cannot open %s\n", path);
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char full[512];
        snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) {
            printf("  %s/\n", entry->d_name);
        } else if (stat(full, &st) == 0) {
            printf("  %-40s %ld bytes\n", entry->d_name, (long)st.st_size);
        } else {
            printf("  %s\n", entry->d_name);
        }
    }
    closedir(dir);
    return 0;
}

static int cmd_play(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: play /sdcard/<file>\n");
        return 1;
    }
    if (!usb_msc_app_owns_storage()) {
        printf("storage is owned by the USB host — unplug USB first\n");
        return 1;
    }
    return audio_play(argv[1]) == ESP_OK ? 0 : 1;
}

static int cmd_stop(int argc, char **argv)
{
    return audio_stop() == ESP_OK ? 0 : 1;
}

static int cmd_sine(int argc, char **argv)
{
    int seconds = (argc > 1) ? atoi(argv[1]) : 3;
    if (seconds < 1 || seconds > 60) {
        seconds = 3;
    }
    printf("440 Hz for %d s...\n", seconds);
    esp_err_t err = audio_sine_test(seconds);
    if (err != ESP_OK) {
        printf("sine test failed: %s\n", esp_err_to_name(err));
    }
    return err == ESP_OK ? 0 : 1;
}

static int cmd_wifi_set(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: wifi_set <ssid> <password>\n");
        return 1;
    }
    esp_err_t err = ota_wifi_set_credentials(argv[1], argv[2]);
    printf("%s\n", err == ESP_OK ? "stored" : esp_err_to_name(err));
    return err == ESP_OK ? 0 : 1;
}

static int cmd_wifi_join(int argc, char **argv)
{
    esp_err_t err = ota_wifi_join();
    if (err != ESP_OK) {
        printf("join failed: %s\n", esp_err_to_name(err));
    }
    return err == ESP_OK ? 0 : 1;
}

static int cmd_ota(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: ota <url-of-.bin>\n");
        return 1;
    }
    audio_stop();
    esp_err_t err = ota_run(argv[1]); /* reboots on success */
    printf("OTA failed: %s\n", esp_err_to_name(err));
    return 1;
}

static const esp_console_cmd_t commands[] = {
    { .command = "list", .help = "List files: list [dir]", .func = cmd_list },
    { .command = "play", .help = "Play a file: play /sdcard/track.mp3", .func = cmd_play },
    { .command = "stop", .help = "Stop playback", .func = cmd_stop },
    { .command = "sine", .help = "DAC smoke test: sine [seconds] (before first play only)", .func = cmd_sine },
    { .command = "wifi_set", .help = "Store WiFi credentials: wifi_set <ssid> <pass>", .func = cmd_wifi_set },
    { .command = "wifi_join", .help = "Connect using stored credentials", .func = cmd_wifi_join },
    { .command = "ota", .help = "Flash firmware from URL: ota http://host/fw.bin", .func = cmd_ota },
};

void console_start(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "ipod>";
    repl_config.max_cmdline_length = 512;

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    esp_console_register_help_command();
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&commands[i]));
    }

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
    ESP_LOGI(TAG, "console ready — type 'help'");
}
