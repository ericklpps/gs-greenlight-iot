# Projeto Green Light - FIAP Global Solution 2025 (2TDS)

## 🌱 Introdução

O **Green Light** é um sistema IoT desenvolvido com objetivo de monitorar a temperatura e umidade em salas de aula, emitindo alertas quando a temperatura ultrapassa níveis críticos, incentivando ações corretivas para conforto e segurança dos alunos.

## 🧠 Visão Geral da Solução

A arquitetura do sistema envolve:

- **ESP32 (simulado no Wokwi)** com sensor **DHT22**, publicando dados via **MQTT**.
- **Broker MQTT** hospedado na VM do professor (`172.208.54.189:1883`), com autenticação.
- **Node-RED** rodando localmente no computador, conectado ao broker remoto para receber, processar e reagir aos dados.

## 🔧 Componentes

- **Microcontrolador**: ESP32 (simulado)
- **Sensor**: DHT22 (temperatura e umidade)
- **Atuador**: LED onboard do ESP32 (GPIO 2)
- **Broker MQTT**:
  - Endereço: `172.208.54.189`
  - Porta: `1883`
  - Usuário: `gs2025`
  - Senha: `q1w2e3r4`
- **Tópicos MQTT**:
  - **Publicação**: `2TDS/esp32/teste`
  - **Subscrição**: `553927/Esp32-gs/comando`

## 🧩 Funcionamento

1. O ESP32 se conecta ao Wi-Fi simulado (`Wokwi-GUEST`) e ao broker MQTT da VM.
2. A cada 10 segundos, envia temperatura e umidade no tópico `2TDS/esp32/teste` no formato JSON.
3. O Node-RED local está conectado ao mesmo broker e recebe os dados.
4. Exibe os dados em tempo real no dashboard e verifica se a temperatura ultrapassa 30°C.
5. Se a temperatura > 30°C, envia `"ON"` ao tópico `553927/Esp32-gs/comando` (LED acende).
6. Caso contrário, envia `"OFF"` (LED apaga).

## 🖥️ Execução

### Requisitos

- VS Code com extensão **PlatformIO** e **Wokwi** instaladas.
- Acesso à VM.
- Node-RED instalado e rodando localmente.

### Passos

1. **ESP32/Wokwi**
   - Abrir projeto no VS Code.
   - Certificar-se que `main.cpp` tem o código correto.
   - Executar a simulação com `Wokwi: Start Simulator`.
   - Verificar conexão ao Wi-Fi e broker MQTT.

2. **Node-RED**
   - Acessar `http://localhost:1880`
   - Configurar o broker (host: `172.208.54.189`, user: `gs2025`, senha: `q1w2e3r4`)
   - Criar fluxo:
     - `mqtt in` (dados do ESP32)
     - `change` (extrai temperatura e umidade)
     - `gauge` (visualização no dashboard)
     - `switch` (verifica se temperatura > 30°C)
     - `change` (define "ON"/"OFF")
     - `mqtt out` (envia comando ao ESP32)
   - Deploy do fluxo
   - Acessar dashboard: `http://localhost:1880/ui`

## 📈 Exemplo de Payload

```json
{
  "ID": "553927",
  "Temperatura": 31.2,
  "Umidade": 58.3
}
