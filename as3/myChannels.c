#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdatomic.h>
#include <stdbool.h>

// struct for the input files
typedef struct {
    char *file_path;
    float alpha;
    float beta;
} inputFileData_t;

typedef struct {
    int bufferSize;
    int lockConfig;
    int globalCheckpointing;
    int maxKthByte;
    int outputBufferSize;
} programData_t;

typedef struct {
    int currentNumThreads;
    int waitSpace[2];
    short wait_counter_index;
} syncData_t;

// global variables
int number_of_files_per_thread;
programData_t programData;
syncData_t syncData;
pthread_mutex_t bufferMutex;         //use pthread_mutex_init instead of this
pthread_mutex_t* bufferMutexes;
pthread_mutex_t programDataMutex = PTHREAD_MUTEX_INITIALIZER;
float* outputBuffer;
pthread_mutex_t syncMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t syncConditional = PTHREAD_COND_INITIALIZER;

void* computeFile(void* arg);
void addToOutput(float value, int kthByte);
void setMutexes();
void resizeOutputBuffer();
void dynamicBarrier();


void print_hex(const char *s)
{
  while(*s)
    printf("%02x", (unsigned int) *s++);
  printf("\n");
}

//function to read first 3 bytes of file and check for BOM, will move the file pointer to the correct position
void checkForBOM(FILE* fp){
    unsigned char bom[3];
    if (fread(bom, sizeof(unsigned char), 3, fp) == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        // printf("UTF-8 BOM detected. Skipping BOM bytes.\n");
    } else {
        fseek(fp, 0, SEEK_SET); // Reset the file position indicator to the beginning
    }
}


void checkInputs(int argc, char *argv[]) {
    // check if the number of arguments is correct
    if (argc != 7) {
        printf("Incorrect number of arguments\n");
        exit(0);
    }

    //assign the program data
    int arg = atoi(argv[1]);
    if(arg <= 0){
        printf("Buffer size must be greater than 0\n");
        exit(0);
    }
    programData.bufferSize = arg;

    arg = atoi(argv[2]);
    if(arg <= 0){
        printf("Number of threads must be greater than 0\n");
        exit(0);
    }
    
    arg = atoi(argv[4]);
    if(arg < 1 || arg > 3){
        printf("Locking configuration must be 1, 2, or 3\n");
        exit(0);
    }
    programData.lockConfig = arg;
}



int main(int argc, char *argv[]) {

    // check if the inputs are valid
    checkInputs(argc, argv);

    // read the metadata file
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return 0;
    }
    checkForBOM(fp);


    // first read the number of input files
    char line[256];
    int number_of_input_files;
    if (fgets(line, sizeof(line), fp) != NULL) {
        number_of_input_files = strtol(line, NULL, 10);
    } else {
        printf("Failed to read the first line.\n");
    }

    int number_of_threads = atoi(argv[2]);
    //check if number of input files is a multiple of number of threads

    number_of_files_per_thread = number_of_input_files / number_of_threads;

    if(number_of_input_files > number_of_threads){
        if(number_of_input_files % number_of_threads != 0){
            printf("Number of input files must be a multiple of number of threads\n");
            exit(0);
        }
    } 
    else {
        number_of_threads = number_of_input_files;
        number_of_files_per_thread = 1;
    }


    pthread_t thread_ids[number_of_threads];

    // Allocate memory for inputFiles array dynamically
    inputFileData_t** inputFiles = malloc(number_of_threads * sizeof(inputFileData_t*));
    for (int i = 0; i < number_of_threads; i++) {
        inputFiles[i] = malloc(number_of_files_per_thread * sizeof(inputFileData_t));
    }

    // Allocate memory for the outputFiles array dynamically start with size 100
    programData.outputBufferSize = 100;
    outputBuffer = malloc(programData.outputBufferSize * sizeof(float));
    // initialize the output buffer to -1

    float arg = 0;
    for(int i = 0; i < number_of_threads; i++){
        //create an array of structs
        for(int j = 0; j < number_of_files_per_thread; j++){
            fgets(line, sizeof(line), fp);
            line[strcspn(line, "\n")] = '\0'; //remove newline
            line[strcspn(line, "\r")] = '\0'; //remove carriage return
     
            inputFiles[i][j].file_path = malloc((strlen(line) + 1) * sizeof(char));
            strcpy(inputFiles[i][j].file_path, line);

            fgets(line, sizeof(line), fp);
            // print_hex(line);
            arg = strtof(line, NULL);
            if(arg < 0 || arg > 1){
                printf("Alpha must be between 0 and 1\n");
                exit(0);
            }
            inputFiles[i][j].alpha = arg;

            fgets(line, sizeof(line), fp);
            // print_hex(line);
            inputFiles[i][j].beta = strtof(line, NULL);
        }

    }

    setMutexes();
    syncData.currentNumThreads = number_of_threads;

    for (int i = 0; i < number_of_threads; i++) {
        pthread_create(&thread_ids[i], NULL, computeFile, (void*)inputFiles[i]);
    }

    //wait for all threads to finish
    for(int i = 0; i < number_of_threads; i++){
        pthread_join(thread_ids[i], NULL);
    }

    // //print the output buffer rounded up to the nearest integer
    // for(int i = 0; i < programData.maxKthByte; i++){
    //     printf("%d\n", (int)ceil(outputBuffer[i]));
    // }

    //write the output buffer to the output file
    FILE *outputFile = fopen(argv[6], "w");
    if (outputFile == NULL) {
        printf("Error opening file\n");
        return 0;
    }

    for(int i = 0; i < programData.maxKthByte; i++){
        int value = (int)ceil(outputBuffer[i]);
        //max value allowed is 2^16 - 1 = 65535
        if(value > 65535){
            value = 65535;
        }
        fprintf(outputFile, "%d\n", value);
    }


    //free the memory
    for (int i = 0; i < number_of_threads; i++) {
        for (int j = 0; j < number_of_files_per_thread; j++) {
            free(inputFiles[i][j].file_path);
        }
        free(inputFiles[i]);
    }
    free(inputFiles);
    free(outputBuffer);
    free(bufferMutexes);
    fclose(fp);
    fclose(outputFile);
    return 0;
}

void resizeOutputBuffer(){
    programData.outputBufferSize *= 2;
    outputBuffer = realloc(outputBuffer, programData.outputBufferSize * sizeof(float));
    //also realloc the mutexes
    bufferMutexes = realloc(bufferMutexes, programData.outputBufferSize * sizeof(pthread_mutex_t));
}

void* computeFile(void* arg) {
    inputFileData_t* inputFiles = (inputFileData_t*) arg;

    int filesRemaining = number_of_files_per_thread;
    int kthByte[number_of_files_per_thread];
    long int filePointers[number_of_files_per_thread];
    bool filesFinished[number_of_files_per_thread];
    float prevSample[number_of_files_per_thread];
    //initialize the arrays
    for(int i = 0; i < number_of_files_per_thread; i++){
        filePointers[i] = 0;
        filesFinished[i] = false;
        prevSample[i] = -1;
        kthByte[i] = 0;
    }

    while(filesRemaining){
        for(int i = 0; i < number_of_files_per_thread; i++){
            if(filesFinished[i]){
                continue;
            }

            FILE* fp = fopen(inputFiles[i].file_path, "r");
            if(fp == NULL){
                perror("Error opening file");
                return NULL;
            }

            fseek(fp, filePointers[i], SEEK_SET);
            if(filePointers[i] == 0){
                checkForBOM(fp);
                filePointers[i] = 3;
            }

            char* buffer = malloc(programData.bufferSize * sizeof(char));

            //read data from file byte by byte
            int bufferIndex = 0;
            int intBufferIndex = 0;
            int currentByte = -1;
            bool valueRead = 0;
            while ((currentByte = fgetc(fp)) != EOF) {
                valueRead = true;
                if (currentByte == '\r') {
                    continue;  // Skip over '\r' characters
                }

                if(currentByte != '\n'){
                    buffer[intBufferIndex] = currentByte;
                    intBufferIndex++;
                }

                bufferIndex++;

                if (bufferIndex == programData.bufferSize) {
                    break;
                }
            }

            if(!valueRead){
                filesFinished[i] = true;        
                filesRemaining--;
                fclose(fp);    
                free(buffer);
                continue;
            }

            unsigned short sample = atoi(buffer);

            //if this is the first sample, just set it to the current sample
            if(prevSample[i] == -1){
                prevSample[i] = sample;
            }

            //apply the low pass filter
            //new_sample_value = alpha * sample_value + (1 - alpha) * previous_sample_value 
            float newSample = (inputFiles[i].alpha * sample) + ((1 - inputFiles[i].alpha) * prevSample[i]);
            prevSample[i] = newSample;

            //apply the amplification
            newSample = newSample * inputFiles[i].beta;

            //write the new sample to the output buffer
            addToOutput(newSample, kthByte[i]);
            kthByte[i]++;
            if(kthByte[i] == programData.outputBufferSize){
                //printf("Resizing output buffer\n");
                resizeOutputBuffer();
            }

            //update the file pointer
            filePointers[i] = ftell(fp);

            if(currentByte == EOF){
                filesFinished[i] = true;
                filesRemaining--;
            }
            fclose(fp);
            free(buffer);
        }
        //k bytes read from all files in this thread
        //WANT TO STALL HERE EVERY LOOP UNTIL ALL THREADS REACH THIS POINT
        dynamicBarrier();
        //printf("Thread %ld reached barrier\n", pthread_self());
        if(filesRemaining == 0){
            pthread_mutex_lock(&syncMutex);
            syncData.currentNumThreads--;
            if (syncData.waitSpace[syncData.wait_counter_index] >= syncData.currentNumThreads) {
                syncData.wait_counter_index = !syncData.wait_counter_index;  // Flip between 0 and 1
                syncData.waitSpace[syncData.wait_counter_index] = 0;
            }
            pthread_mutex_unlock(&syncMutex);
            pthread_cond_broadcast(&syncConditional);
            break;
        }
    
    }

    // take the max kth byte from all threads and write it to global variable
    pthread_mutex_lock(&programDataMutex);
    {
        for(int i = 0; i < number_of_files_per_thread; i++){
            if(kthByte[i] > programData.maxKthByte){
                programData.maxKthByte = kthByte[i];
            }
        }
    }
    pthread_mutex_unlock(&programDataMutex);
    // printf("Thread %ld finished\n", pthread_self()  );
    
    return NULL;
}

void addToOutput(float value, int kthByte){
    switch(programData.lockConfig){
        case(1):
            pthread_mutex_lock(&bufferMutex);
            {
                outputBuffer[kthByte] += value;
            }
            pthread_mutex_unlock(&bufferMutex);
            break;
        case(2):
            //each element in the array has its own mutex
            pthread_mutex_lock(&bufferMutexes[kthByte]);
            {
                outputBuffer[kthByte] += value;
            }
            pthread_mutex_unlock(&bufferMutexes[kthByte]);
            break;
        case(3):
            //compare_and_exchange_strong
            float expected_value = outputBuffer[kthByte];
            float desired_value = expected_value + value;
            bool swapped = false;
            while(!swapped){
                swapped = atomic_compare_exchange_weak(&outputBuffer[kthByte], &expected_value, desired_value);
                expected_value = outputBuffer[kthByte];
                desired_value = expected_value + value;
            }
            break;

        }
}

void setMutexes(){
    switch(programData.lockConfig){
        case(1):
            if(pthread_mutex_init(&bufferMutex, NULL) != 0){
                perror("Error initializing mutex");
                exit(1);
            }
            break;
        case(2):
            bufferMutexes = malloc(programData.outputBufferSize * sizeof(pthread_mutex_t));
            for(int i = 0; i < programData.outputBufferSize; i++){
               if(pthread_mutex_init(&bufferMutexes[i], NULL) != 0){
                   perror("Error initializing mutex");
                   exit(1);
               }
            }
            break;
        case(3):
            break;
    }
}

void dynamicBarrier() { //s.o. method for dynamic barrier
    pthread_mutex_lock(&syncMutex);
    const short waitIndex = syncData.wait_counter_index;
    syncData.waitSpace[waitIndex]++;
    if (syncData.waitSpace[waitIndex] < syncData.currentNumThreads) {
        do {
            pthread_cond_wait(&syncConditional, &syncMutex);
        } 
        while (syncData.waitSpace[waitIndex] < syncData.currentNumThreads);
    } 
    else {
        //go to the other index and reset the wait count
        syncData.wait_counter_index = !syncData.wait_counter_index; 
        syncData.waitSpace[syncData.wait_counter_index] = 0;
    }
    pthread_mutex_unlock(&syncMutex);
    pthread_cond_broadcast(&syncConditional);
}