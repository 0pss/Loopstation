#include<stdio.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <unistd.h>     //STDIN_FILENO
#include <pthread.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <sys/time.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define DURATION_IN_SECONDS 60.0
#define BUF_SIZE (SAMPLE_RATE * CHANNELS * DURATION_IN_SECONDS)
#define BUF_SIZE_INT ((int)BUF_SIZE)


//################################################
// TODO:
// Das abspielen muss auch mitgezählt werden
//    -> sync verschiedeneer Spuren
//    -> an welcher stelle kommt der Overdub rein


// Structure to hold thread arguments
struct ThreadArgs {
    short *buf;
    snd_pcm_t *capture_handle;
    snd_pcm_t *playback_handle;
    int length;
    // You can add more variables here if needed
};

// global variables
int ActiveRecordBuffer = 0;
struct ThreadArgs recordBuffer0;
struct ThreadArgs recordBuffer1;
int position = 0;
int playposition = 0;
// end globals

// Function to handle errors
void error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

// Function to be executed by the thread
void *record(void *arg) {
    // Cast the void pointer back to the structure type
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int err;


    // Perform some work in the thread
    
    // Open capture (record) handle
    if ((err = snd_pcm_open(&args->capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        error("Cannot open capture audio device");
    }

    // Set capture parameters
    if ((err = snd_pcm_set_params(args->capture_handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  CHANNELS,
                                  SAMPLE_RATE,
                                  0,
                                  1)) < 0) {  // 0.5 seconds
        error("Cannot set capture parameters");
    }
    
    
    // Record audio
    printf("Recording for %f seconds...\n", DURATION_IN_SECONDS);
    if ((err = snd_pcm_readi(args->capture_handle, args->buf, BUF_SIZE)) != BUF_SIZE) {
        error("Error capturing audio");
    }
    
    //snd_pcm_close(args->capture_handle);
    // Thread is done, exit
    pthread_exit(NULL);
}

// Function to be executed by the thread
void *play(void *arg) {
    // Cast the void pointer back to the structure type
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    
    int err;
    
    
    // Example: Changing thread priority
    struct sched_param param;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(SCHED_FIFO); // Set priority to maximum
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    
    
	printf("HERE\n");

    // Perform some work in the thread
    
    // Open playback handle
    if ((err = snd_pcm_open(&args->playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        error("Cannot open playback audio device");
    }
    
    printf("NOW HERE\n");

    // Set playback parameters
    if ((err = snd_pcm_set_params(args->playback_handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  CHANNELS,
                                  SAMPLE_RATE,
                                  0,
                                  1)) < 0) {  // 0.5 seconds
        error("Cannot set playback parameters");
    }

    printf("Playing Audio\n");
    // Record audio
    while(1){
		snd_pcm_writei(args->playback_handle, args->buf, args->length);
    }
    printf("Done\n");

       // snd_pcm_close(playback_handle);
    // Thread is done, exit
    pthread_exit(NULL);
}


void *recordHandler(void *arg){
    pthread_t myThread;
    snd_pcm_t *one_capture_handle;
    
    recordBuffer0.buf = (short *)malloc(BUF_SIZE_INT * sizeof(short));
    if (recordBuffer0.buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }
    recordBuffer0.length = BUF_SIZE_INT; 
    
    recordBuffer1.buf = (short *)malloc(BUF_SIZE_INT * sizeof(short));
    if (recordBuffer1.buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }
    recordBuffer1.length = BUF_SIZE_INT; 
    
   int len = ((int)DURATION_IN_SECONDS*101*441);
   
   recordBuffer0.capture_handle = one_capture_handle;
   recordBuffer1.capture_handle = one_capture_handle;
    
    while(1){
        ActiveRecordBuffer = 0;
        printf("Recording with recordbuffer 0\n");


        // Create the thread and pass the structure as an argument
        if (pthread_create(&myThread, NULL, record, (void *)&recordBuffer0) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        position = 0;
        for(position=0; position<=len;position+=441){
            usleep(10000); // Sleep for 0.01 seconds
        }
        
        printf("Recording with recordbuffer 1\n");
        ActiveRecordBuffer = 1;
        
        // Create the thread and pass the structure as an argument
        if (pthread_create(&myThread, NULL, record, (void *)&recordBuffer1) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        position = 0;
        for(position=0; position<=len;position+=441){
            usleep(10000); // Sleep for 0.01 seconds
        }
    }
}


void *playHandler(void *arg){
    pthread_t track1;
    pthread_t track2;
    pthread_t track3;
    pthread_t track4;

    snd_pcm_t *one_capture_handle;
    
    recordBuffer0.buf = (short *)malloc(BUF_SIZE_INT * sizeof(short));
    if (recordBuffer0.buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }
    recordBuffer0.length = BUF_SIZE_INT; 
    
    recordBuffer1.buf = (short *)malloc(BUF_SIZE_INT * sizeof(short));
    if (recordBuffer1.buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }
    recordBuffer1.length = BUF_SIZE_INT; 
    
   int len = ((int)DURATION_IN_SECONDS*101*441);
   
   recordBuffer0.capture_handle = one_capture_handle;
   recordBuffer1.capture_handle = one_capture_handle;
    
    while(1){
        ActiveRecordBuffer = 0;
        printf("Recording with recordbuffer 0\n");


        // Create the thread and pass the structure as an argument
        if (pthread_create(&myThread, NULL, record, (void *)&recordBuffer0) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        position = 0;
        for(position=0; position<=len;position+=441){
            usleep(10000); // Sleep for 0.01 seconds
        }
        
        printf("Recording with recordbuffer 1\n");
        ActiveRecordBuffer = 1;
        
        // Create the thread and pass the structure as an argument
        if (pthread_create(&myThread, NULL, record, (void *)&recordBuffer1) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        position = 0;
        for(position=0; position<=len;position+=441){
            usleep(10000); // Sleep for 0.01 seconds
        }
    }
}










int main(void){   
    int c;   
    static struct termios oldt, newt;
    pthread_t myThread;
    pthread_t myThread2;
    
    
    
    int recorded = 0;
    
    if (pthread_create(&myThread, NULL, recordHandler, NULL) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    
    printf("recordhandler established\n\n");

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);          

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /*This is your part:
    I choose 'e' to end input. Notice that EOF is also turned off
    in the non-canonical mode*/
    
    int start = -1;
    int end = -1;
    int len =0;
    int last_active_recordbuffer = -1;
    struct ThreadArgs track0;
    
    while((c=getchar())!= 'q'){      
    
        if( c == '1' && start > 0){
            printf("Stop Position in Buffer: %d \n", position);
            end = position;
            printf("Star: %d, End: %d, Recordec: %d", start, end, recorded);

	    }   
    
    
        if( c == '1' && start < 0){
            printf("recording Track 1\n");
            printf("Start Position in Buffer: %d \n", position);
            last_active_recordbuffer = ActiveRecordBuffer;
            start = position;
        }      
        
	
            
        if(start>0 && end>0 && recorded == 0){
            if(start<end && ActiveRecordBuffer == last_active_recordbuffer){
                printf("Allocating...\n");
                len =end-start;
                track0.buf = (short *)malloc(len * sizeof(short));
                if (track0.buf == NULL) {
                    fprintf(stderr, "Failed to allocate memory for buffer\n");
                    exit(EXIT_FAILURE);
                }
                track0.length = len;
                printf("DONE Allocating...\n");
                int i =0;
                printf("coping 2 playbuffer! Length: %d \n", (len));
                
                if(last_active_recordbuffer == 0){
                    for(start; start<=end;start++){
                        track0.buf[i] = recordBuffer0.buf[start];
                        i++;
                        //printf("%d",i);
                    }
                }else{
                    for(start; start<=end;start++){
                        track0.buf[i] = recordBuffer1.buf[start];
                        i++;
                        //printf("%d",i);
                    }
                }
                printf("copied 2 playbuffer!");
                start=-1;
                end=-1;
                recorded = 1;
                if (pthread_create(&myThread2, NULL, play, (void *)&track0) != 0) {
                    fprintf(stderr, "Error creating thread\n");
                    return 1;
                }
                
	        }
	    
            if(ActiveRecordBuffer != last_active_recordbuffer){
                printf("Allocating...XtraLong...\n");
                len =(BUF_SIZE_INT-start)+end;
                track0.buf = (short *)malloc(len * sizeof(short));
                if (track0.buf == NULL) {
                fprintf(stderr, "Failed to allocate memory for buffer\n");
                exit(EXIT_FAILURE);
                }
                track0.length = len;
                printf("DONE Allocating...\n");
                int i =0;
                printf("coping 2 playbuffer! Length: %d \n", len);
                
                if(last_active_recordbuffer == 0){
                    for(start; start<=BUF_SIZE_INT;start++){
                    track0.buf[i] = recordBuffer0.buf[start];
                    i++;
                    //printf("%d",i);
                    }
                            
                        for(int j = 0; j<=end;j++){
                    track0.buf[i] = recordBuffer1.buf[j];
                    i++;
                    //printf("%d",i);
                    }
                }else{
                
                        for(start; start<=BUF_SIZE_INT;start++){
                    track0.buf[i] = recordBuffer1.buf[start];
                    i++;
                    //printf("%d",i);
                    }
                                
                        for(int j = 0; j<=end;j++){
                    track0.buf[i] = recordBuffer0.buf[j];
                    i++;
                    //printf("%d",i);
                    }
                    
                }
                printf("copied 2 playbuffer!");
                start=-1;
                end=-1;
                recorded = 1;
                if (pthread_create(&myThread2, NULL, play, (void *)&track0) != 0) {
                fprintf(stderr, "Error creating thread\n");
                return 1;
                }
                
            }
        }
        
        if(start>0 && end>0 && recorded>0){
            printf("OVERDUBBING!!\n");
            int i =0;
            printf("coping 2 playbuffer! Length: %d \n", (len));
            
            if(len < end-start){
                end = start+len;
            }
            
            for(start; start<=end;start++){
                track0.buf[i] += recordBuffer0.buf[start];
                i++;
                //printf("%d",i);
            }
            printf("copied 2 playbuffer!");
            start=-1;
            end=-1;
        }
                  
    }
    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


    return 0;
}

