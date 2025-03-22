#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Configurações do Wi-Fi
const char* ssid = "REDE";  // Substitua pelo seu SSID
const char* password = "PASS";  // Substitua pela sua senha Wi-Fi

// Configurações do ECG
int ecgPin = 34;  // Pino onde o sensor AD8232 está conectado
int valorECG = 0; // Variável para armazenar o valor do ECG
String servidorURL = "http://172.22.68.45:5000/dados";  // URL do servidor Flask

// Configurações da Biometria
HardwareSerial mySerial(1);  // Usando UART1 no ESP32 (GPIO16, GPIO17)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
bool autenticado = false;  // Variável para controlar se o usuário está autenticado
int userId = -1;  // Variável para armazenar o ID do usuário autenticado

void setup() {
  Serial.begin(115200);

  // Inicializa a comunicação com o sensor de biometria
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Erro: Sensor de biometria não detectado!");
    while (1);
  }

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.print("IP_Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (!autenticado) {
    Serial.println("Aguardando autenticação...");
    userId = verificarDigital();  // Tenta autenticar o usuário

    if (userId > 0) {
      Serial.print("Usuário autenticado! ID: ");
      Serial.println(userId);
      autenticado = true;  // Usuário autenticado com sucesso
    } else {
      Serial.println("Falha na autenticação!");
      delay(2000);  // Aguarda 2 segundos antes de tentar novamente
    }
  }

  if (autenticado) {
    // Leitura do ECG
    valorECG = analogRead(ecgPin);
    Serial.println(valorECG);

    // Criar JSON com os dados do ECG e o ID do usuário
    String jsonPayload = "{\"ecg\": " + String(valorECG) + ", \"id\": " + String(userId) + "}";
    Serial.println(jsonPayload);

    // Enviar dados para o servidor
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(servidorURL);
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
        Serial.println("Dados enviados com sucesso!");
      } else {
        Serial.println("Erro ao enviar dados: " + String(httpResponseCode));
      }
      http.end();
    }
    delay(100);  // Delay para a próxima leitura
  }
}

// Função para verificar a digital
int verificarDigital() {
  Serial.println("Posicione o dedo para autenticação...");
  int resultado = finger.getImage();

  if (resultado != FINGERPRINT_OK) return -1;

  resultado = finger.image2Tz();
  if (resultado != FINGERPRINT_OK) return -1;

  resultado = finger.fingerFastSearch();
  if (resultado == FINGERPRINT_OK) {
    return finger.fingerID;  // Retorna o ID do usuário autenticado
  }

  return -1;  // Falha na autenticação
}
