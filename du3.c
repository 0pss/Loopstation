#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define SAMPLE_FORMAT ma_format_s16
#define CHANNELS     1
#define SAMPLE_RATE   44100
#define BUFFER_SIZE  (int)(SAMPLE_RATE * CHANNELS * 50) // Adjust the buffer size as needed

//######################################################
// TODO:
// 	- fadein/out
//	- Multitrack&mute

int16_t audioBuffer[BUFFER_SIZE];
ma_uint64 audioBufferPos = 0;
ma_bool32 isRecording = MA_TRUE; // Flag to indicate whether recording is active
ma_bool32 isSTOP = MA_FALSE; // Flag to indicate whether recording is active
ma_uint64 firstNonSilent = 0;
ma_uint64 lastNonSilent = BUFFER_SIZE - 1;
ma_uint64 lengthRecordBuffer = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);

    if (isRecording) {
        const int16_t* pInputS16 = (const int16_t*)pInput; // Assuming the input is in 16-bit signed integer format

        // Write the input audio samples to the global buffer
        ma_uint64 bytesToCopy = frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels);
        ma_uint64 bytesAvailable = (BUFFER_SIZE - audioBufferPos) * sizeof(int16_t);

        if (bytesToCopy > bytesAvailable || isSTOP) {
            // If attempting to copy more data than available space in the buffer, copy only what fits
            bytesToCopy = bytesAvailable;
            isRecording = MA_FALSE; // Stop recording when the buffer is full

            printf("Recording stopped. Buffer full.\n");
            
            // Detect the first and last non-silent sample positions
            
            for (ma_uint64 i = 0; i < lengthRecordBuffer; ++i) {

                if (audioBuffer[i] > 80 || audioBuffer[i] < -80) {
                    firstNonSilent = i;
                    break;
                }
            }
            
            
            for (ma_uint64 i = lengthRecordBuffer - 1; i > firstNonSilent; --i) {
                if (audioBuffer[i] > 80 || audioBuffer[i] < -80) {
                    lastNonSilent = i;
                    break;
                }
            }            	
            
            

            // Adjust the buffer position to play the non-silent section
            audioBufferPos = firstNonSilent;
            
           printf("Audio Buffer:\n");
            for (ma_uint64 i = firstNonSilent; i < lastNonSilent; ++i) {
                printf("%d\n", audioBuffer[i]);
            }
        }else{

        // Copy input audio samples to the global buffer
        
        memcpy(audioBuffer + audioBufferPos, pInputS16, bytesToCopy);
        audioBufferPos += bytesToCopy / sizeof(int16_t);
        lengthRecordBuffer += bytesToCopy;
        }
        
    }else{
    
    ma_uint32 framesAvailable = BUFFER_SIZE - audioBufferPos;
    ma_uint32 framesToWrite = frameCount > framesAvailable ? framesAvailable : frameCount;


    if (framesToWrite > 0) {
        MA_COPY_MEMORY(pOutput, audioBuffer + audioBufferPos, frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));
        audioBufferPos += framesToWrite;


    }
    
    // Cut off the last portion of the buffer during playback to avoid a pause
    if (audioBufferPos >= lastNonSilent) {
        audioBufferPos = firstNonSilent; // Reset the buffer position to create a continuous loop
    }
    
    }


}

int main() {
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;

    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = NULL;
    deviceConfig.capture.format     = ma_format_s16;
    deviceConfig.capture.channels   = 1;
    deviceConfig.capture.shareMode  = ma_share_mode_shared;
    deviceConfig.playback.pDeviceID = NULL;
    deviceConfig.playback.format    = ma_format_s16;
    deviceConfig.playback.channels  = 1;
    deviceConfig.dataCallback       = data_callback;
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        return result;
    }

    ma_device_start(&device);

    printf("Recording...\n");
    int c;

    while((c=getchar())!= 'q'){      
    
        if( c == '1'){
        isSTOP =!isSTOP;
        }   
    }

    printf("Playback...\n");


    printf("Press Enter to quit...");
    getchar();


    // Uninitialize the sound and device
    ma_device_uninit(&device);


    return 0;
}

