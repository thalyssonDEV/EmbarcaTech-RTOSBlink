#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/** @def LED_R_PIN
 *  @brief Pino do LED (vermelho) do LED RGB.
 */
#define LED_R_PIN   13   

/** @def BUTTON_A
 *  @brief Pino do botão A.
 */
#define BUTTON_A    5    

/** @def BUTTON_READ_PERIOD_MS
 *  @brief Período de leitura do botão em milissegundos.
 */
#define BUTTON_READ_PERIOD_MS 100

/** Fila para compartilhar o estado do botão (inteiro: 0 ou 1). */
QueueHandle_t xQueueButton;

/** Fila para enviar comando de controle do LED (0 = desligar, 1 = acender). */
QueueHandle_t xQueueLED;

/**
 * @brief Tarefa para leitura do botão físico.
 *
 * Esta tarefa realiza a leitura do pino configurado para o botão a cada 100ms.
 * O estado lido é enviado para a fila xQueueButton. Com o pull-up
 * ativado, a leitura retorna 1 quando o botão não está pressionado e 0 quando pressionado.
 *
 * @param pvParameters Parâmetros passados para a tarefa (não utilizado).
 */
void vTaskButtonRead(void *pvParameters);

/**
 * @brief Tarefa para processamento do estado do botão.
 *
 * Esta tarefa aguarda receber o estado do botão pela fila xQueueButton. Se o botão
 * estiver pressionado (estado 0), envia o comando para acender o LED para a fila xQueueLED.
 * Caso contrário, envia o comando para desligar o LED.
 *
 * @param pvParameters Parâmetros passados para a tarefa (não utilizado).
 */
void vTaskButtonProcess(void *pvParameters);

/**
 * @brief Tarefa para controle do LED.
 *
 * Esta tarefa aguarda comandos de controle (0 ou 1) pela fila xQueueLED e atualiza
 * o estado do LED físico (pino LED_R_PIN) de acordo com o comando recebido.
 *
 * @param pvParameters Parâmetros passados para a tarefa (não utilizado).
 */
void vTaskLEDControl(void *pvParameters);

int main(void) {
    /* Inicializa o sistema padrão (stdio, etc.) */
    stdio_init_all();

    /* Inicializa o pino do LED como saída */
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_put(LED_R_PIN, 0); // Inicialmente desligado

    /* Inicializa o pino do botão e ativa o pull-up interno */
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    /* Cria as filas de comunicação */
    xQueueButton = xQueueCreate(10, sizeof(int));
    if (xQueueButton == NULL) {
        while(1); // Erro: trava o sistema se a fila não for criada.
    }
    
    xQueueLED = xQueueCreate(10, sizeof(int));
    if (xQueueLED == NULL) {
        while(1);
    }
    
    /* Criação das tarefas */
    xTaskCreate(vTaskButtonRead, "ButtonRead", 256, NULL, 1, NULL);     /**< Tarefa 1: Leitura do botão */
    xTaskCreate(vTaskButtonProcess, "ButtonProcess", 256, NULL, 2, NULL);   /**< Tarefa 2: Processamento do botão */
    xTaskCreate(vTaskLEDControl, "LEDControl", 256, NULL, 3, NULL);         /**< Tarefa 3: Controle do LED */

    /* Inicia o escalonador do FreeRTOS */
    vTaskStartScheduler();

    while(1);
    return 0;
}

void vTaskButtonRead(void *pvParameters) {
    int buttonState;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;) {
        /* Leitura do botão: 
         * Com pull-up ativado, gpio_get retorna 1 quando o botão não é pressionado e 0 quando pressionado.
         */
        buttonState = gpio_get(BUTTON_A);
        
        /* Envia o estado lido para a fila */
        xQueueSend(xQueueButton, &buttonState, 0);
        
        /* Aguarda 100ms para a próxima leitura */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BUTTON_READ_PERIOD_MS));
    }
}

void vTaskButtonProcess(void *pvParameters) {
    int receivedState;
    int ledCommand;
    for (;;) {
        /* Aguarda o próximo estado do botão da fila */
        if (xQueueReceive(xQueueButton, &receivedState, portMAX_DELAY) == pdPASS) {
            /* Se o botão estiver pressionado (estado 0), acende o LED; caso contrário, desliga */
            if (receivedState == 0) {
                ledCommand = 1; /**< Comando para acender o LED */
            } else {
                ledCommand = 0; /**< Comando para apagar o LED */
            }
            /* Envia o comando para a fila da tarefa de controle do LED */
            xQueueSend(xQueueLED, &ledCommand, 0);
        }
    }
}

void vTaskLEDControl(void *pvParameters) {
    int ledCommand;
    for (;;) {
        /* Aguarda comando de LED na fila */
        if (xQueueReceive(xQueueLED, &ledCommand, portMAX_DELAY) == pdPASS) {
            if (ledCommand == 1) {
                /* Liga o LED */
                gpio_put(LED_R_PIN, 1);
            } else {
                /* Desliga o LED */
                gpio_put(LED_R_PIN, 0);
            }
        }
    }
}