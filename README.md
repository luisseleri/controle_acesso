# controle_acesso
# Sistema de Controle de Acesso com Raspberry Pi Pico

Este projeto implementa um sistema de controle de acesso utilizando o Raspberry Pi Pico. A autenticação é realizada por meio de uma sequência específica de pressionamentos de botões (GPIO 4 e GPIO 6), eliminando a necessidade de um teclado matricial tradicional. O sistema integra feedback visual (LED RGB e matriz WS2812) e sonoro (buzzer) para interação multimodal.

## Funcionalidades

- **Autenticação por Botões:** Sequência específica de pressionamentos para validação.
- **Feedback Visual:** LED RGB e matriz WS2812 fornecem indicadores visuais claros.
- **Feedback Sonoro:** Buzzer emite tons de confirmação ou erro.
- **Proteção Contra Brute Force:** Bloqueio temporário após múltiplas tentativas incorretas.

## Componentes Utilizados

- **Raspberry Pi Pico**
- **Botões (GPIO 4 e GPIO 6)**
- **LED RGB**
- **Matriz WS2812**
- **Buzzer Piezoelétrico**

## Estrutura do Projeto
projeto_controle_acesso/
├── build/ # Arquivos gerados durante a compilação
├── src/ # Código-fonte principal
│ ├── main.c # Arquivo principal do firmware
│ ├── auth_logic.c # Lógica de autenticação
│ └── feedback.c # Controle de feedback visual e sonoro
├── include/ # Arquivos de cabeçalho (.h)
│ ├── auth_logic.h
│ └── feedback.h
├── lib/ # Bibliotecas externas
│ ├── pico-sdk/ # SDK oficial do Raspberry Pi Pico
│ └── ws2812.pio.h # Biblioteca para a matriz WS2812
├── data/ # Dados adicionais (logs, etc.)
└── CMakeLists.txt # Configuração do CMake para compilação
└── README.md # Documentação geral do projeto

