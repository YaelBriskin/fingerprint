#ifndef PWM_H
#define PWM_H

#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0/"

void setup_pwm(int period, int duty_cycle);
void stop_pwm();
void play_sound(int button_in, int button_new);

#endif // PWM_H
