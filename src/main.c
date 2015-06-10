#include <stdint.h>
#include "mk20dx128.h"
#include "serial.c"
#include "led.c"


extern void delay(uint32_t);


/* pwm */

/* FTM1 clock (ie. F_CPU) prescaler. must be at least 2 */
#define FTM1_DIV (2)

static inline uint16_t pwm_freq_to_counter(uint32_t freq)
{
  /* freq max: 12MHz */
  /* freq resoultion: 366Hz */

  /* 1 / freq = counter / (f_cpu / f_div); */
  /* counter = f_cpu / (f_div * freq) */

  return (uint16_t)(F_CPU / (FTM1_DIV * freq));
}

static void pwm_start(void)
{
  /* setup ftm1 so that channel A is set as a pwm */
  /* with 50% duty and channel B is its complement. */
  /* for this to work, linked channels are used and */
  /* the hardware assumes B = A + 1 */

  /* pinout */
  /* pwm:  teensy_16, 35, PORTB0_ALT3, FTM1_CH0 */
  /* pwm#: teensy_17, 36, PORTB1_ALT3, FTM1_CH1 */

  /* ftm1 channel indices */
#define PWM_CHA (0)
#define PWM_CHB (PWM_CHA + 1)

#if (PWM_CHA != 0)
#error "TODO"
#endif /* (PWM_CHA != 0) */

  /* enable ftm1 clock */
  /* note, p.823: FTM1_FREQ can not exceed F_CPU / 2 */
  SIM_SCGC6 |= 1 << 25;

  /* disable counter clocking */
  /* important, otherwise counters do not get load */
  FTM1_SC = 0;

  /* configure pin mux */
  PORTB_PCR0 = 3 << 8;
  PORTB_PCR1 = 3 << 8;

  /* disable write protection */
  FTM1_MODE = 1 << 2;

  /* set pwm 16 bits counter and modulos */
  /* duty is set to 50%, ie. frequency / 2 */
  /* complementary channel inverted from n/2 to n */
#define PWM_FREQ 100000
  const uint16_t n = pwm_freq_to_counter(PWM_FREQ);
  FTM1_MOD = n;
  FTM1_C0V = n / 2;
  FTM1_C1V = n;

  /* load counter initial value */
  /* note p.835: only use FTM1_CNTIN = 0 */
  FTM1_CNTIN = 0;

  /* ftm1 channel setup */
  /* edge aligned pwm, clear output on match */
  FTM1_C0SC = (2 << 4) | (2 << 2);
  FTM1_C1SC = (2 << 4) | (2 << 2);

  /* output initial state */
  FTM1_OUTINIT = 1 << PWM_CHB;

  /* per channel response on match mask */
  FTM1_OUTMASK = 0;

  /* linked channels configuration */
  /* enable complementarity */
  FTM1_COMBINE = (1 << 1) | (1 << 0);

  /* set channel polarity as active high */
  FTM1_POL = 0;

  /* disable fault inputs */
  FTM1_FLTCTRL = 0;

  /* disable quadrature encoder */
  FTM1_QDCTRL = 0;

  /* synchronization configuration */
  FTM1_SYNCONF = (1 << 7) | (1 << 2);

  /* channel inversion configuration */
  FTM1_INVCTRL = 0;

  /* disable software output control */
  FTM1_SWOCTRL = 0;

  /* pwm counter loading */
  FTM1_PWMLOAD = 0;

  /* configure mode */
  /* disable write protection */
  /* initialize channel outputs */
  /* enable all registers */
  FTM1_MODE = (1 << 2) | (1 << 1) | (1 << 0);

  /* enable counter using system clock */
  /* note, p.823: FTM1_FREQ can not exceed F_CPU / 2 */
#if (FTM1_DIV == 2)
  FTM1_SC = (1 << 3) | (1 << 0);
#else
#error "TODO"
#endif
}

static void pwm_stop(void)
{
  /* disable write protection */
  FTM1_MODE = 1 << 2;

  /* disable counter clocking */
  FTM1_SC = 0 << 3;
}

__attribute__((unused))
static void pwm_set_high(void)
{
  /* set CHA high */
  FTM1_SWOCTRL = (1 << 8) | (1 << 0);
}


__attribute__((unused))
static void pwm_set_low(void)
{
  /* set CHA low */
  FTM1_SWOCTRL = (0 << 8) | (1 << 0);
}


/* main */

int main(void)
{
  led_setup();

  pwm_start();

  while (1)
  {
    led_set_val(1);
    delay(250);

    led_set_val(0);
    delay(250);
  }

  pwm_stop();

  return 0;
}
