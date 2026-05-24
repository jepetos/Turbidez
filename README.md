# Turbidez
Dispositivo para análise de turbidez de liquidos
**versão 1.6**.

Este documento foi estruturado para servir tanto como documentação técnica do projeto quanto como um guia rápido de instalação e calibração do hardware.

---

```markdown
# 🚰 Arduino Electronic-Dreno (I2C) - v1.6

O **Electronic-Dreno** é um sistema inteligente de automação e segurança para controle de drenagem baseado na análise de turbidez da água. Utilizando um sensor óptico, o sistema monitora a qualidade da água em tempo real, executa descargas automáticas preventivas ou corretivas e possui uma camada rígida de proteção mecânica para evitar transbordamentos ou queima de componentes.

---

## 🚀 Novidades da Versão 1.6

* **🐛 Correção Crítica de Sintaxe:** Corrigido o bug na função `digitalRead(RELAY_NC_PIN)` que anulava a leitura de feedback do relé.
* **💡 Inicialização de Hardware:** Correção do pino PWM do LED de Incidência (Pino 11) no setup para garantir o controle dinâmico correto.
* **📺 Gestão Inteligente do LCD:** O display agora acorda automaticamente (`WakeUp`) sempre que houver uma variação física significativa na leitura do sensor, otimizando o tempo de vida útil do backlight sem perder monitoramento visual.
* **⏱️ Filtro de Cooldown:** Ajuste no tempo de espera pós-dreno (5 segundos) para evitar leituras falsas causadas pela turbulência imediata da descarga.

---

## 🛠️ Funcionalidades Principais

1.  **Drenagem por Turbidez:** Acionamento automático quando a tensão do sensor atinge o limite crítico configurado (`TURBIDITY_THRESHOLD`).
2.  **Drenagem Preventiva (Por Tempo):** Caso a água permaneça limpa por muito tempo, o sistema realiza uma descarga temporizada preventiva (ajustável em `DRAIN_INTERVAL`).
3.  **Contador e Limitador Crítico (`MAX_DRAINS`):** Se o sistema atingir 10 drenos seguidos sem intervenção manual, ele trava e dispara um alarme sonoro/visual, evitando desperdício de água em caso de vazamentos persistentes.
4.  **Checagem Mecânica Antifreio:** Utiliza o pino `NF` (Normalmente Fechado) do relé como feedback. Se o relé colar os contatos ou falhar ao abrir, o sistema entra em modo de segurança total e trava o processo.
5.  **Modo Standby:** Desliga o LCD após 30 segundos de inatividade para economia de energia.

---

## 📐 Pinagem do Hardware (Arduino Uno)

| Componente | Pino Arduino | Tipo | Descrição |
| :--- | :---: | :---: | :--- |
| **Sensor de Turbidez** | `A0` | Analógico | Leitura de tensão (0V a 5V) proporcional à sujeira |
| **Potenciômetro** | `A1` | Analógico | Ajuste de brilho do LED indicador local |
| **LED Status/Indicador**| `3` | Saída (PWM) | LED de feedback controlado pelo potenciômetro |
| **LED Incidência** | `11` | Saída (PWM) | LED auxiliar que brilha proporcionalmente à opacidade |
| **Módulo Relé (Sinal)** | `9` | Saída Digital| Controle da bobina do relé do dreno |
| **Feedback Relé (NF)** | `7` | Entrada Digital| Monitoramento do contato físico do relé (INPUT_PULLUP) |
| **Buzzer** | `8` | Saída Digital| Avisos sonoros e alarmes de falha técnica |
| **LCD I2C (SDA)** | `A4` | Comunicação| Linha de dados do display |
| **LCD I2C (SCL)** | `A5` | Comunicação| Linha de clock do display |

---

## 📦 Bibliotecas Necessárias

Certifique-se de ter as seguintes bibliotecas instaladas na sua IDE do Arduino antes de compilar:

* `Wire.h` (Nativa)
* `LiquidCrystal_I2C` (Por Frank de Brabander)
* `LED.h` / `Relay.h` (Bibliotecas customizadas de gerenciamento de periféricos)
* `GBALib_Potentiometer.h` (Biblioteca de mapeamento do potenciômetro)

---

## 🔧 Guia de Calibração Física

Os sensores de turbidez comerciais operam com **Lógica Inversa** (Água limpa gera tensão alta; água suja gera tensão baixa). 

1. Adicione água limpa ao sensor e verifique o valor de tensão no Monitor Serial (ex: $4.2\text{V}$).
2. Adicione o nível de sujeira que deve disparar o dreno e anote a nova tensão (ex: $2.1\text{V}$).
3. Vá no código e ajuste a variável de corte:
   ```cpp
   float TURBIDITY_THRESHOLD = 2.5; // Altere para o valor ideal do seu ambiente

```

4. *Nota sobre o sentido físico:* Caso o seu sensor funcione de maneira direta (tensão sobe quando a água suja), lembre-se de alterar o sinal de comparação no bloco 5 do `loop()` de `>` para `<`.

---

## 🚨 Códigos de Erro no LCD

* `ERR: FALHA RELE!` / `SISTEMA TRAVADO`: O Arduino enviou o comando para desligar o dreno, mas o pino de feedback físico indicou que o circuito continua fechado (relé colado ou travado). **Ação: Desligue a alimentação imediatamente.**
* `ERR: LIMITE MAX` / `10 DRENOS EXEC.`: O sensor detectou água suja 10 vezes seguidas e executou os drenos, mas a água não limpou. O sistema trava para evitar inundação ou esvaziamento completo do reservatório. **Ação: Verifique a entrada de água limpa ou limpe o sensor.**

---

## 📄 Licença

Este projeto é de código aberto sob a licença MIT. Sinta-se livre para modificar, adaptar e compartilhar melhorias.

```

---

