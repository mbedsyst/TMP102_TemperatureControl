#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define TEMP_SENSOR_PATH "/sys/bus/i2c/devices/1-0048/temp1_input"
#define LOG_FILE "temperature_log.txt"

void log_temperature() 
{
    FILE *sensor_file, *log_file;
    int temperature_raw;
    double temperature_celsius;
    time_t now;
    char time_str[64];

    sensor_file = fopen(TEMP_SENSOR_PATH, "r");
    if (!sensor_file) 
    {
        perror("Failed to open sensor file");
        return;
    }
    
    if (fscanf(sensor_file, "%d", &temperature_raw) != 1) 
    {
        perror("Failed to read temperature data");
        fclose(sensor_file);
        return;
    }
    fclose(sensor_file);

    temperature_celsius = temperature_raw / 1000.0;

    now = time(NULL);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    log_file = fopen(LOG_FILE, "a");
    if (!log_file) 
    {
        perror("Failed to open log file");
        return;
    }
    
    fprintf(log_file, "%s - Temperature: %.2f°C\n", time_str, temperature_celsius);
    fclose(log_file);
}

int main() 
{
    while (1) 
    {
        log_temperature();
        sleep(15);
    }
    return 0;
}
