# Sistema de Monitoramento Simples com FreeRTOS

Este projeto é uma aplicação embarcada utilizando **FreeRTOS** no **Raspberry Pi Pico W**. O sistema realiza o monitoramento do estado de um botão e o controle de um LED, distribuído em três tarefas distintas que cooperam entre si.

## Funcionalidades

- Leitura do botão físico (conectado ao pino GPIO5).
- Processamento do estado do botão.
- Controle de um LED RGB (LED_R_PIN - GPIO13).
- Sistema implementado com **3 tarefas** utilizando **FreeRTOS** e **filas** para comunicação entre elas.

## Estrutura do Sistema

| Tarefa     | Função                         | Intervalo                     |
|------------|--------------------------------|--------------------------------|
| Tarefa 1   | Leitura do botão               | Executada a cada 100ms         |
| Tarefa 2   | Processamento do botão         | Executada sempre que recebe dados da Tarefa 1 |
| Tarefa 3   | Controle do LED                | Executada sempre que acionada pela Tarefa 2 |

## Componentes utilizados

- Raspberry Pi Pico W
- LED RGB (pinos GPIO 13, 12, 11)
- Botão (pino GPIO5)
- FreeRTOS

## Como funciona

1. A **Tarefa 1** lê o estado do botão periodicamente.
2. A **Tarefa 2** decide se o LED deve ser ligado ou desligado de acordo com o estado do botão.
3. A **Tarefa 3** liga ou desliga o LED.
