#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pwm.h"

// Definições dos pinos dos LEDs, botões e canais ADC
#define LED_BLUE_PIN 12 // LED Azul
#define LED_RED_PIN 13 // LED Vermelho
#define LED_GREEN_PIN 11 // LED verde (para o botão do joystick)
#define JOYSTICK_BTN_PIN 22 // Botão do joystick para o LED verde
#define BUTTON_A_PIN 5 // Botão A para ativar/desativar os LED PWM
#define JOYSTICK_Y_ADC_CHANNEL 0
#define JOYSTICK_X_ADC_CHANNEL 1

// Valor central do joystick (12-bit ADC) e zona morta
#define JOYSTICK_CENTER 2048
#define JOYSTICK_DEADZONE 200  

// Definições para o display SSD1306 via I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define SSD1306_ADDR 0x3C
#define WIDTH 128
#define HEIGHT 64

// Variáveis globais para PWM, LED verde e borda do display
volatile bool pwm_enabled = true;
volatile bool green_led_state = false;
volatile uint8_t border_style = 0;
volatile absolute_time_t last_joy_time;
volatile bool joy_button_already_pressed = false; // Para o botão do joystick

// Variáveis globais para o botão A
volatile absolute_time_t last_button_a_time;
volatile bool button_a_already_pressed = false;

// Configuração do PWM para um pino específico
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 4095); // Resolução 12-bit
    pwm_init(slice_num, &config, true);
}

// Mapeia o valor do joystick para um valor de PWM
uint16_t map_joystick_to_pwm(uint16_t joystick_value) {
    if (abs((int16_t)joystick_value - JOYSTICK_CENTER) < JOYSTICK_DEADZONE)
        return 0;
    else if (joystick_value < JOYSTICK_CENTER)
        return JOYSTICK_CENTER - joystick_value;
    else
        return joystick_value - JOYSTICK_CENTER;
}

// Desenha uma borda simples
void draw_single_border(ssd1306_t *ssd) {
    ssd1306_rect(ssd, 3, 3, 122, 58, true, false);
}

// Desenha uma borda dupla com espaçamento
void draw_double_border(ssd1306_t *ssd) {
    ssd1306_rect(ssd, 3, 3, 122, 58, true, false);
    ssd1306_rect(ssd, 6, 6, 116, 52, true, false);
}

// Desenha uma borda tripla
void draw_triple_border(ssd1306_t *ssd) {
    ssd1306_rect(ssd, 3, 3, 122, 58, true, false);
    ssd1306_rect(ssd, 6, 6, 116, 52, true, false);
    ssd1306_rect(ssd, 9, 9, 110, 46, true, false);
}

// Callback da IRQ para os botões
void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    if (gpio == JOYSTICK_BTN_PIN) {
        if (absolute_time_diff_us(last_joy_time, now) > 50000) { // Debounce de 50ms
            last_joy_time = now;
            if (!gpio_get(JOYSTICK_BTN_PIN)) { // Botão pressionado
                if (!joy_button_already_pressed) {
                    joy_button_already_pressed = true;
                    green_led_state = !green_led_state;
                    gpio_put(LED_GREEN_PIN, green_led_state);
                    border_style = (border_style + 1) % 3;
                }
            } else { // Botão liberado
                joy_button_already_pressed = false;
            }
        }
    } else if (gpio == BUTTON_A_PIN) {
        if (absolute_time_diff_us(last_button_a_time, now) > 50000) { // Debounce de 50ms
            last_button_a_time = now;
            if (!gpio_get(BUTTON_A_PIN)) { // Botão pressionado
                if (!button_a_already_pressed) {
                    button_a_already_pressed = true;
                    pwm_enabled = !pwm_enabled;
                    if (!pwm_enabled) {
                        pwm_set_gpio_level(LED_BLUE_PIN, 0);
                        pwm_set_gpio_level(LED_RED_PIN, 0);
                    }
                }
            } else { // Botão liberado
                button_a_already_pressed = false;
            }
        }
    }
}

int main() {
    stdio_init_all();

    // Inicializa o I2C para o display SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, SSD1306_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configura o botão do joystick e o LED verde
    gpio_init(JOYSTICK_BTN_PIN);
    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);

    // Configura o botão A para IRQ
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Registra callback para os botões usando IRQ
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, gpio_callback);
    gpio_set_irq_enabled(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    // Inicializa o ADC para os eixos do joystick (GPIO 26 e 27)
    adc_init();
    adc_gpio_init(26); // Eixo Y
    adc_gpio_init(27); // Eixo X

    // Configura os LEDs (PWM para azul e vermelho)
    setup_pwm(LED_BLUE_PIN);
    setup_pwm(LED_RED_PIN);

    // Variáveis para o display e posição do quadrado
    char str_x[6], str_y[6];
    int16_t x_pos = HEIGHT / 2.5;
    int16_t y_pos = WIDTH / 2;
    uint16_t last_x_value = JOYSTICK_CENTER;
    uint16_t last_y_value = JOYSTICK_CENTER;

    while (1) {
        // Leitura dos valores do joystick
        adc_select_input(JOYSTICK_Y_ADC_CHANNEL);
        uint16_t y_value = adc_read();
        adc_select_input(JOYSTICK_X_ADC_CHANNEL);
        uint16_t x_value = adc_read();

        // Verifica se houve variação significativa nos eixos
        if (abs((int16_t)x_value - (int16_t)last_x_value) > 10 ||
            abs((int16_t)y_value - (int16_t)last_y_value) > 10) {
            uint16_t blue_pwm = map_joystick_to_pwm(y_value);
            uint16_t red_pwm = map_joystick_to_pwm(x_value);

            int16_t delta_x = (int16_t)x_value - JOYSTICK_CENTER;
            int16_t delta_y = (int16_t)y_value - JOYSTICK_CENTER;

            int16_t x_offset = (-delta_y * 40) / 2048; // Movimento em X controlado pelo eixo Y (invertido)
            int16_t y_offset = (delta_x * 20) / 2048; // Movimento em Y controlado pelo eixo X

            x_pos += x_offset;
            y_pos += y_offset;

            // Garante que o quadrado fique dentro dos limites do display
            x_pos = (x_pos < 0) ? 0 : (x_pos > 56) ? 56 : x_pos;
            y_pos = (y_pos < 0) ? 0 : (y_pos > 120) ? 120 : y_pos;

            // Atualiza os LEDs azul e vermelho via PWM, se habilitado
            if (pwm_enabled) {
                pwm_set_gpio_level(LED_BLUE_PIN, blue_pwm);
                pwm_set_gpio_level(LED_RED_PIN, red_pwm);
            } else {
                pwm_set_gpio_level(LED_BLUE_PIN, 0);
                pwm_set_gpio_level(LED_RED_PIN, 0);
            }

            last_x_value = x_value;
            last_y_value = y_value;
        }

        // Atualiza o display com as informações e o quadrado
        sprintf(str_x, "%d", x_value);
        sprintf(str_y, "%d", y_value);

        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, x_pos, y_pos, 8, 8, true, true);

        // Desenha a borda de acordo com o estilo, se o LED verde estiver ativo
        if (green_led_state) {
            switch (border_style) {
                case 0:
                    draw_double_border(&ssd);
                    break;
                case 1:
                    draw_single_border(&ssd);
                    break;
                case 2:
                    draw_triple_border(&ssd);
                    break;
            }
        }

        ssd1306_send_data(&ssd);
        sleep_ms(10);
    }
    
    return 0;
}
