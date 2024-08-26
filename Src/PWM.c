// #include "../Inc/PWM.h"
// #include <stdio.h>
// #include <fcntl.h>
// #include <unistd.h>

// typedef enum {
//     ENTRY_SOUND,         // Sound for entry
//     EXIT_SOUND           // Sound for exit
//     NEW_EMPLOYEE_SOUND,  // Sound for new employee
//     ERROR_SOUND,         // Sound for error
// } SoundType;

// typedef struct {
//     int frequency; // Frequency in Hz
//     int duration;  // Duration in milliseconds
// } Sound;

// typedef struct {
//     Sound* sounds;  // Array of sounds
//     int length;     // Number of sounds in the sequence
// } SoundSequence;

// Sound sound1[] = {
//     { 440, 200 }, // A4, 200 ms
//     { 493, 200 }, // B4, 200 ms
//     { 523, 200 }, // C5, 200 ms
//     { 493, 200 }  // B4, 200 ms
// };

// Sound sound2[] = {
//     { 261, 300 }, // C4, 300 ms
//     { 293, 300 }, // D4, 300 ms
//     { 329, 300 }, // E4, 300 ms
//     { 293, 300 }  // D4, 300 ms
// };

// SoundSequence sound_sequences[] = {
//     { sound1, sizeof(sound1) / sizeof(sound1[0]) }, // ENTRY_SOUND   
//     { sound2, sizeof(sound2) / sizeof(sound2[0]) }, // EXIT_SOUND
//     { sound3, sizeof(sound3) / sizeof(sound3[0]) }, // NEW_EMPLOYEE_SOUND
//     { sound5, sizeof(sound5) / sizeof(sound5[0]) }  // ERROR_SOUND
// };

// int num_sequences = sizeof(sound_sequences) / sizeof(sound_sequences[0]);
// /**
//  * @brief Sets up the PWM signal.
//  * 
//  * This function configures the period and duty cycle for the PWM signal
//  * using the specified parameters.
//  * 
//  * @param period The period of the signal in nanoseconds.
//  * @param duty_cycle The duty cycle of the signal in nanoseconds.
//  */
// void setup_pwm(int period, int duty_cycle) {
//     int fd;
//     char path[128];

//     // Set the period
//     snprintf(path, sizeof(path), "%speriod", PWM_PATH);
//     fd = open(path, O_WRONLY);
//     if (fd < 0) {
//         perror("Failed to open period file");
//         return;
//     }
//     dprintf(fd, "%d", period);
//     close(fd);

//     // Set the duty cycle
//     snprintf(path, sizeof(path), "%sduty_cycle", PWM_PATH);
//     fd = open(path, O_WRONLY);
//     if (fd < 0) {
//         perror("Failed to open duty_cycle file");
//         return;
//     }
//     dprintf(fd, "%d", duty_cycle);
//     close(fd);

//     // Enable PWM
//     snprintf(path, sizeof(path), "%senable", PWM_PATH);
//     fd = open(path, O_WRONLY);
//     if (fd < 0) {
//         perror("Failed to open enable file");
//         return;
//     }
//     dprintf(fd, "1");
//     close(fd);
// }
// /**
//  * @brief Stops the PWM signal.
//  * 
//  * This function disables the PWM signal.
//  */
// void stop_pwm(void) {
//     int fd;
//     char path[128];

//     // Disable PWM
//     snprintf(path, sizeof(path), "%senable", PWM_PATH);
//     fd = open(path, O_WRONLY);
//     if (fd < 0) {
//         perror("Failed to open enable file");
//         return;
//     }
//     dprintf(fd, "0");
//     close(fd);
// }
// /**
//  * @brief Plays a sound sequence based on the provided sound type.
//  * 
//  * This function generates a sequence of sounds using PWM based on the provided sound type.
//  * 
//  * @param sound_type An integer representing the type of sound sequence to play.
//  */
// void play_sound(int sound_type) {
//     // Ensure the sound_type is within the valid range
//     if (sound_type < 0 || sound_type >= num_sequences) {
//         syslog_log(LOG_ERR, __func__, "stderr", "Invalid sound_type");
//         return;
//     }

//     SoundSequence sequence = sound_sequences[sound_type];
    
//     for (int i = 0; i < sequence.length; i++) {
//         Sound sound = sequence.sounds[i];
//         setup_pwm(1000000 / sound.frequency, 500000); // 50% duty cycle
//         usleep(sound.duration * 1000); // Convert milliseconds to microseconds
//         stop_pwm();
//         usleep(100000); // Short pause between notes
//     }
// }