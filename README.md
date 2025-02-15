# Controle de LEDs e Display com Joystick na BitDogLab:
- Este projeto utiliza um microcontrolador Raspberry Pi Pico W para controlar LEDs e um display OLED SSD1306 com base nos movimentos de um joystick analógico. Também inclui a manipulação de bordas no display e o controle de LEDs via PWM.


# Componentes utilizados:
- LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
- Botão do Joystick conectado à GPIO 22.
- Joystick conectado aos GPIOs 26 e 27.
- Botão A conectado à GPIO 5.
- Display SSD1306 conectado via I2C (GPIO 14 e GPIO15).

# Descrição:
- Controle de LEDs via PWM: O LED azul e o LED vermelho são controlados pelos eixos Y e X do joystick, respectivamente. O PWM é habilitado/desabilitado pelo botão A.

- Controle do LED Verde: O LED verde é controlado pelo botão do joystick, que também alterna entre diferentes estilos de borda no display.

- Display OLED SSD1306: O display exibe um quadrado que se move com base nos movimentos do joystick. Diferentes estilos de borda são desenhados no display quando o LED verde está ativo.

- Joystick Analógico: O joystick é lido através de dois canais ADC (X e Y). O valor do joystick é mapeado para controlar o PWM dos LEDs e o movimento do quadrado no display.
