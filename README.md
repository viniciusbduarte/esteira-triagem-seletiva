# Esteira de Triagem Seletiva Automatizada com ATmega328P

## 1. Introdução

O presente projeto visa o desenvolvimento de uma **esteira de triagem seletiva automatizada**, capaz de identificar, classificar e direcionar caixas de diferentes tamanhos.  
A proposta consiste na construção de um sistema embarcado baseado no microcontrolador **ATmega328P**, que utiliza um sensor ultrassônico para medir a altura das caixas, classificando-as em três categorias: **Pequeno (P)**, **Médio (M)** e **Grande (G)**.

Após a classificação, o sistema aciona dispositivos de desvio (representados por LEDs e/ou pistões) para redirecionar as caixas. Além disso, o sistema realiza a contagem das caixas classificadas e exibe as informações em displays de 7 segmentos, permitindo o monitoramento em tempo real do processo.

---

## 2. Objetivos do Projeto

### 2.1 Objetivo Geral
Desenvolver uma esteira automatizada capaz de medir, classificar e direcionar caixas por tamanho, utilizando sensores e um microcontrolador.

### 2.2 Objetivos Específicos
- Implementar a leitura de distância com o sensor ultrassônico HC-SR04.
- Classificar caixas em três categorias (P, M e G).
- Controlar a velocidade da esteira por PWM.
- Acionar atuadores (pistões/LEDs) para desvio das caixas.
- Contabilizar o número de caixas por categoria.
- Exibir informações em displays de 7 segmentos.
- Implementar controle do sistema por botões físicos.

---

## 3. Componentes do Sistema

### 3.1 Microcontrolador
- **ATmega328P**
  - Responsável pelo controle geral do sistema.
  - Utilização de timers, interrupções e periféricos de I/O.
  - Programação em linguagem C (AVR-GCC).

### 3.2 Sensor de Distância
- **HC-SR04 (sensor ultrassônico)**
  - Mede a altura da caixa através do tempo de retorno do eco.
  - Comunicação via pinos Trigger e Echo.
  - Timer1 utilizado para medição precisa do tempo.

### 3.3 Sistema de Acionamento
- LEDs representando pistões de desvio.
- Saídas digitais do ATmega328P controlam os atuadores.

### 3.4 Sistema de Exibição
- Displays de 7 segmentos multiplexados.
- Controle via Timer0 e interrupções.
- Exibição da categoria da caixa e contagem total.

### 3.5 Controle do Sistema
- 4 botões físicos:
  - 2 botões para controle da velocidade da esteira (PWM).
  - 1 botão para pausar o sistema.
  - 1 botão para retomar o funcionamento.

### 3.6 Motor da Esteira
- Controle via PWM.
- Ajuste dinâmico de velocidade conforme entrada do usuário.

---

## 4. Arquitetura do Sistema

O sistema é dividido em módulos funcionais:

- **Módulo de Hardware**  
  Inicialização de portas, timers, interrupções e periféricos.

- **Módulo de Medição**  
  Leitura do sensor ultrassônico e cálculo da distância.

- **Módulo de Classificação**  
  Definição da categoria da caixa com base na altura medida.

- **Módulo de Controle**  
  Acionamento de pistões/LEDs e controle do motor PWM.

- **Módulo de Interface**  
  Atualização do display e leitura dos botões.

---

## 5. Funcionamento do Sistema

1. A caixa passa pela esteira.
2. O sensor HC-SR04 mede a altura da caixa.
3. O microcontrolador calcula a distância usando o Timer1.
4. A caixa é classificada como P, M ou G.
5. O sistema aciona o pistão correspondente.
6. A contagem da categoria é incrementada.
7. O display mostra a categoria e o número de caixas.
8. O operador pode ajustar a velocidade ou pausar o sistema.

---

## 6. Diagrama do Circuito Eletrônico

A Figura 1 apresenta o diagrama do circuito eletrônico do sistema, incluindo o microcontrolador ATmega328P, o sensor ultrassônico HC-SR04, os displays de 7 segmentos, os botões de controle e os atuadores.

**Figura 1 – Diagrama do circuito eletrônico do sistema**

![Diagrama do circuito](./images/circuito.png)

---

## 7. Modelagem e Demonstração do Sistema

A Figura 2 apresenta uma modelagem conceitual do sistema, representando a esteira, as caixas, o sensor ultrassônico e os mecanismos de desvio.

**Figura 2 – Ilustração da esteira de triagem seletiva - Visão Frontal**

![Modelagem da esteira](./images/modelagem_esteira.png)

**Figura 3 – Ilustração da esteira de triagem seletiva - Visão Superior**

![Modelagem da esteira](./images/modelagem_esteira1.png)
---

## 8. Algoritmo de Medição de Distância

A medição de distância é realizada pelo tempo de duração do pulso Echo do HC-SR04, utilizando o Timer1 do ATmega328P.

A distância é calculada pela fórmula:

\[
Distância(cm) = \frac{Tempo \cdot Velocidade\ do\ Som}{2}
\]

Considerando o prescaler e a frequência do microcontrolador, obtém-se a conversão direta do tempo para centímetros.

---

## 9. Controle PWM da Esteira

O motor da esteira é controlado por PWM, permitindo:
- Aumento ou redução da velocidade.
- Interação do usuário por botões físicos.

---

## 10. Resultados Esperados

Espera-se que o sistema seja capaz de:
- Classificar corretamente caixas em três categorias.
- Operar em tempo real com alta precisão.
- Permitir controle manual da velocidade da esteira.
- Exibir informações claras no display.
- Operar de forma modular e escalável.

---

## 11. Conclusão

O projeto da esteira de triagem seletiva demonstra a aplicação prática de sistemas embarcados, sensores e controle digital em um contexto industrial.  
A utilização do microcontrolador ATmega328P, aliada ao sensor ultrassônico HC-SR04 e ao controle PWM, permite a construção de um sistema eficiente, modular e expansível.

---