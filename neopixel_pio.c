#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"

// Definições de pinos
#define BOTAO_GPIO4 5
#define BOTAO_GPIO6 6
#define TEMPO_DEBOUNCE 50
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13
#define BUZZER_PIN 10
#define WS2812_PIN 7
#define NUM_LEDS 25

// Estrutura para representar um pixel GRB
typedef struct {
    uint8_t G, R, B;
} pixel_t;

// Buffer de pixels para a matriz de LEDs
pixel_t leds[NUM_LEDS];

// Variáveis globais para o controle da máquina PIO
PIO np_pio;
uint sm;

// Definições para proteção contra brute force
#define MAX_TENTATIVAS_INCORRETAS 5
#define TEMPO_BLOQUEIO_MS 10000

// Protótipos de funções
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npWrite();
int getIndex(int x, int y);
void leds_xy(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void matriz_verde();
void matriz_x();
void piscar_x(int vezes);
void tocar_som(int frequencia, int duracao_ms);
void tocar_feedback_sonoro(bool sucesso);
void acender_led_rgb(int r, int g, int b);
bool verificar_combinacao();

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels
    for (uint i = 0; i < NUM_LEDS; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
 * Atribui uma cor RGB a um LED específico.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
    for (uint i = 0; i < NUM_LEDS; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Sinal de RESET
}

/**
 * Função para converter coordenadas (x, y) em índice linear.
 */
int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x; // Linha par (esquerda para direita)
    } else {
        return y * 5 + (4 - x); // Linha ímpar (direita para esquerda)
    }
}

/**
 * Função para controlar LEDs com base em coordenadas (x, y).
 */
void leds_xy(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    int index = getIndex(x, y);
    npSetLED(index, r, g, b);
}

/**
 * Função para acender toda a matriz de verde.
 */
void matriz_verde() {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            leds_xy(x, y, 0, 255, 0); // Acende todos os LEDs em verde
        }
    }
    npWrite(); // Envia os dados para os LEDs físicos
}

/**
 * Função para desenhar um "X" vermelho na matriz.
 */
void matriz_x() {
    // Apaga todos os LEDs
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            leds_xy(x, y, 0, 0, 0);
        }
    }

    // Define os LEDs que formam o "X"
    int leds_x[][2] = {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4},
        {0, 4}, {1, 3}, {2, 2}, {3, 1}, {4, 0}
    };
    for (int i = 0; i < 10; i++) {
        int x = leds_x[i][0];
        int y = leds_x[i][1];
        leds_xy(x, y, 255, 0, 0); // Acende os LEDs do "X" em vermelho
    }
    npWrite(); // Envia os dados para os LEDs físicos
}

/**
 * Função para piscar o "X" vermelho.
 */
void piscar_x(int vezes) {
    for (int i = 0; i < vezes; i++) {
        matriz_x();
        sleep_ms(500);
        for (int j = 0; j < NUM_LEDS; j++) {
            npSetLED(j, 0, 0, 0); // Apaga todos os LEDs
        }
        npWrite();
        sleep_ms(500);
    }
}

/**
 * Função para tocar som no buzzer.
 */
void tocar_som(int frequencia, int duracao_ms) {
    if (frequencia == 0) {
        gpio_put(BUZZER_PIN, 0);
        return;
    }
    int periodo_us = 1000000 / frequencia;
    for (int i = 0; i < duracao_ms * 1000 / periodo_us; i++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_us(periodo_us / 2);
        gpio_put(BUZZER_PIN, 0);
        sleep_us(periodo_us / 2);
    }
}

/**
 * Feedback sonoro para senha correta ou incorreta.
 */
void tocar_feedback_sonoro(bool sucesso) {
    if (sucesso) {
        // Som ascendente para senha correta
        tocar_som(440, 200); // Lá (A4)
        tocar_som(523, 200); // Dó (C5)
        tocar_som(659, 200); // Mi (E5)
    } else {
        // Som descendente para senha incorreta
        tocar_som(659, 200); // Mi (E5)
        tocar_som(523, 200); // Dó (C5)
        tocar_som(440, 200); // Lá (A4)
    }
}

/**
 * Função para acender LEDs RGB.
 */
void acender_led_rgb(int r, int g, int b) {
    gpio_put(LED_VERMELHO, r);
    gpio_put(LED_VERDE, g);
    gpio_put(LED_AZUL, b);
}

/**
 * Verificação de senha com proteção contra brute force.
 */
bool verificar_combinacao() {
    static int tentativas_incorretas = 0;

    // Verifica se o sistema está bloqueado
    if (tentativas_incorretas >= MAX_TENTATIVAS_INCORRETAS) {
        printf("Sistema bloqueado por %d segundos.\n", TEMPO_BLOQUEIO_MS / 1000);
        acender_led_rgb(1, 0, 0); // LED RGB vermelho
        for (int i = 0; i < NUM_LEDS; i++) {
            npSetLED(i, 0, 0, 255); // Matriz azul
        }
        npWrite();
        sleep_ms(TEMPO_BLOQUEIO_MS);
        tentativas_incorretas = 0; // Reseta após o bloqueio

        // Retorna ao estado inicial após o bloqueio
        acender_led_rgb(1, 0, 0); // LED RGB vermelho
        matriz_x();               // Matriz exibe "X" vermelho
        return false;
    }

    int contador_gpio4 = 0;
    int contador_gpio6 = 0;
    int total_pressoes = 0;

    while (total_pressoes < 5) {
        if (!gpio_get(BOTAO_GPIO4)) {
            contador_gpio4++;
            total_pressoes++;
            tocar_som(1000, 100);
            sleep_ms(200);
        }
        if (!gpio_get(BOTAO_GPIO6)) {
            contador_gpio6++;
            total_pressoes++;
            tocar_som(1000, 100);
            sleep_ms(200);
        }
        sleep_ms(50);
    }

    // Verifica se a combinação está correta
    if (contador_gpio4 == 3 && contador_gpio6 == 2) {
        tentativas_incorretas = 0; // Reseta o contador de tentativas incorretas
        return true;
    } else {
        tentativas_incorretas++; // Incrementa o contador de tentativas incorretas
        printf("Sequência incorreta! Tentativas incorretas: %d\n", tentativas_incorretas);
        acender_led_rgb(1, 0, 0);
        piscar_x(3);
        tocar_feedback_sonoro(false);
        return false;
    }
}

/**
 * Função principal.
 */
int main() {
    stdio_init_all();
    sleep_ms(2000); // Aguarda 2 segundos para garantir que a serial esteja pronta
    printf("Iniciando sistema...\n");

    // Configuração dos GPIOs
    gpio_init(BOTAO_GPIO4);
    gpio_set_dir(BOTAO_GPIO4, GPIO_IN);
    gpio_pull_up(BOTAO_GPIO4);
    printf("GPIO4 configurado.\n");

    gpio_init(BOTAO_GPIO6);
    gpio_set_dir(BOTAO_GPIO6, GPIO_IN);
    gpio_pull_up(BOTAO_GPIO6);
    printf("GPIO6 configurado.\n");

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    printf("LEDs RGB configurados.\n");

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    printf("Buzzer configurado.\n");

    // Inicializa a matriz de LEDs WS2812
    npInit(WS2812_PIN);
    printf("Matriz de LEDs inicializada.\n");

    // Estado inicial: LED RGB vermelho e matriz apagada
    acender_led_rgb(1, 0, 0); // LED RGB vermelho
    matriz_x();               // Matriz exibe "X" vermelho
    printf("Estado inicial configurado.\n");

    while (true) {
        printf("Digite a combinação:\n");
        if (verificar_combinacao()) {
            printf("Senha correta!\n");
            acender_led_rgb(0, 1, 0); // Verde
            matriz_verde();
            tocar_feedback_sonoro(true);
            printf("Pressione qualquer botão para fechar.\n");

            // Aguarda o usuário pressionar qualquer botão para fechar
            while (gpio_get(BOTAO_GPIO4) && gpio_get(BOTAO_GPIO6)) {
                sleep_ms(50);
            }

            // Retorna ao estado inicial
            acender_led_rgb(1, 0, 0); // LED RGB vermelho
            matriz_x();               // Matriz exibe "X" vermelho
        } else {
            printf("Senha incorreta!\n");
            // Retorna ao estado inicial após erro
            acender_led_rgb(1, 0, 0); // LED RGB vermelho
            matriz_x();               // Matriz exibe "X" vermelho
        }
    }
}