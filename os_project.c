#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

// ANSI escape codes for colors
#define GREEN_TEXT "\x1B[32m"
#define RESET_COLOR "\x1B[0m"

#define BUFFER_SIZE 10
#define NUM_SENSORS 3
#define RECORD_INTERVAL 10

typedef struct {
    char sensor_type[20];
    float value;
    time_t timestamp;
} SensorData;

typedef struct {
    SensorData buffer[BUFFER_SIZE];
    int in;
    int out;
} BoundedBuffer;

typedef struct {
    float sum;
    int count;
    time_t last_record_time;
} SensorStats;

BoundedBuffer shared_buffer;
SensorStats sensor_stats[NUM_SENSORS];
pthread_mutex_t mutex;
pthread_mutex_t file_mutex;
pthread_mutex_t console_mutex;
sem_t empty;
sem_t full;

const char* sensor_types[NUM_SENSORS] = {"Temperature", "Humidity", "Motion"};
const char* CSV_FILENAME = "sensor_averages.csv";

void init_buffer() {
    shared_buffer.in = 0;
    shared_buffer.out = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);
    pthread_mutex_init(&console_mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    srand(time(NULL));
    
    // Initialize sensor statistics
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensor_stats[i].sum = 0;
        sensor_stats[i].count = 0;
        sensor_stats[i].last_record_time = time(NULL);
    }
    
    // Initialize CSV file with header
    FILE* file = fopen(CSV_FILENAME, "w");
    if (file != NULL) {
        fprintf(file, "Timestamp,SensorType,AverageValue,RecordCount\n");
        fclose(file);
    }
}

float generate_sensor_value(const char* sensor_type) {
    if (strcmp(sensor_type, "Temperature") == 0) {
        return 15.0 + (rand() % 250) / 10.0; // 15.0 to 40.0
    } else if (strcmp(sensor_type, "Humidity") == 0) {
        return 30.0 + (rand() % 700) / 10.0; // 30.0 to 100.0
    } else { // Motion
        return rand() % 2; // 0 or 1
    }
}

void display_sensor_input(const SensorData* data) {
    pthread_mutex_lock(&console_mutex);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&data->timestamp));
    
    printf(GREEN_TEXT "[%s] %s: %.2f" RESET_COLOR "\n", 
           time_str, data->sensor_type, data->value);
    fflush(stdout); // Ensure immediate output
    
    pthread_mutex_unlock(&console_mutex);
}

void write_to_csv(const char* sensor_type, float average, int count, time_t timestamp) {
    pthread_mutex_lock(&file_mutex);
    
    FILE* file = fopen(CSV_FILENAME, "a");
    if (file != NULL) {
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
        fprintf(file, "\"%s\",\"%s\",%.2f,%d\n", time_str, sensor_type, average, count);
        fclose(file);
        
        // Also display the average on console (in default color)
        pthread_mutex_lock(&console_mutex);
        printf("[%s] AVERAGE %s: %.2f (%d samples)\n", 
               time_str, sensor_type, average, count);
        fflush(stdout);
        pthread_mutex_unlock(&console_mutex);
    } else {
        perror("Failed to open CSV file");
    }
    
    pthread_mutex_unlock(&file_mutex);
}

void update_sensor_stats(const char* sensor_type, float value) {
    for (int i = 0; i < NUM_SENSORS; i++) {
        if (strcmp(sensor_type, sensor_types[i]) == 0) {
            sensor_stats[i].sum += value;
            sensor_stats[i].count++;
            
            if (sensor_stats[i].count >= RECORD_INTERVAL) {
                float average = sensor_stats[i].sum / sensor_stats[i].count;
                time_t now = time(NULL);
                write_to_csv(sensor_type, average, sensor_stats[i].count, now);
                
                // Reset stats
                sensor_stats[i].sum = 0;
                sensor_stats[i].count = 0;
                sensor_stats[i].last_record_time = now;
            }
            break;
        }
    }
}

void* sensor_producer(void* arg) {
    const char* sensor_type = (const char*)arg;
    
    while (1) {
        // Simulate sensor reading delay
        sleep(1 + rand() % 3);
        
        SensorData data;
        strncpy(data.sensor_type, sensor_type, sizeof(data.sensor_type));
        data.value = generate_sensor_value(sensor_type);
        data.timestamp = time(NULL);
        
        // Display the new input immediately
        display_sensor_input(&data);
        
        // Wait for empty slot in buffer
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        
        // Add data to buffer
        shared_buffer.buffer[shared_buffer.in] = data;
        shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    
    return NULL;
}

void* aggregator_consumer(void* arg) {
    while (1) {
        // Simulate processing delay
        sleep(2);
        
        // Wait for data in buffer
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        
        // Get data from buffer
        SensorData data = shared_buffer.buffer[shared_buffer.out];
        shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;
        
        // Update statistics and check for recording interval
        update_sensor_stats(data.sensor_type, data.value);
        
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
    
    return NULL;
}

int main() {
    pthread_t sensor_threads[NUM_SENSORS];
    pthread_t aggregator_thread;
    
    // Create directory for CSV if it doesn't exist
    mkdir("sensor_data", 0777);
    
    init_buffer();
    
    // Clear screen and set black background (works on most terminals)
    printf("\033[40m");
    printf("\033[2J"); // Clear screen
    
    // Create sensor threads
    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_create(&sensor_threads[i], NULL, sensor_producer, (void*)sensor_types[i]);
    }
    
    // Create aggregator thread
    pthread_create(&aggregator_thread, NULL, aggregator_consumer, NULL);
    
    // Wait for threads (though they run indefinitely)
    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(sensor_threads[i], NULL);
    }
    pthread_join(aggregator_thread, NULL);
    
    // Cleanup (though we never get here)
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&console_mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    
    // Reset terminal colors before exiting
    printf(RESET_COLOR);
    
    return 0;
}