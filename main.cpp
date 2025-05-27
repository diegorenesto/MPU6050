// Sistema di Rilevamento Vibrazioni con MPU6050
// Autore: Diego Renesto
// Data: 27 Maggio 2025

#include <Wire.h>    // Libreria per comunicazione I2C
#include <MPU6050.h> // Libreria per il sensore MPU6050

// Inizializzazione del sensore
MPU6050 mpu;

// Definizione dei pin
const int LED_ALLARME = 13; // LED rosso per allarme vibrazioni
const int BUZZER = 12;      // Buzzer per allarme acustico
const int LED_STATUS = 11;  // LED verde per stato operativo

// Variabili per la gestione delle vibrazioni
float accelerazioneX, accelerazioneY, accelerazioneZ;
float magnitudoAccelerazione;
float soglia_vibrazione = 2.0; // Soglia in g per attivare l'allarme
bool allarme_attivo = false;
unsigned long tempo_ultimo_allarme = 0;
const unsigned long DURATA_ALLARME = 3000; // Durata allarme in millisecondi

// Variabili per la calibrazione
float offset_x = 0, offset_y = 0, offset_z = 0;
const int CAMPIONI_CALIBRAZIONE = 100;

void setup()
{
  // Inizializzazione comunicazione seriale
  Serial.begin(9600);
  Serial.println("Sistema di Rilevamento Vibrazioni - Avvio...");

  // Configurazione pin di output
  pinMode(LED_ALLARME, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);

  // Inizializzazione I2C
  Wire.begin();

  // Inizializzazione del sensore MPU6050
  Serial.println("Inizializzazione MPU6050...");
  mpu.initialize();

  // Verifica connessione del sensore
  if (mpu.testConnection())
  {
    Serial.println("Connessione MPU6050 riuscita!");
    digitalWrite(LED_STATUS, HIGH); // Accende LED di stato
  }
  else
  {
    Serial.println("Errore: Impossibile connettersi al MPU6050!");
    while (1)
    {
      // Lampeggio LED di errore
      digitalWrite(LED_STATUS, HIGH);
      delay(200);
      digitalWrite(LED_STATUS, LOW);
      delay(200);
    }
  }

  // Configurazione sensibilità accelerometro (±2g)
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  // Calibrazione del sensore
  calibraSensore();

  Serial.println("Sistema pronto per il rilevamento vibrazioni!");
  Serial.println("Soglia vibrazione impostata a: " + String(soglia_vibrazione) + " g");
  Serial.println("----------------------------------------");
}

void loop()
{
  // Lettura dei valori grezzi dall'accelerometro
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Conversione in unità g (9.81 m/s²)
  accelerazioneX = (ax / 16384.0) - offset_x; // 16384 LSB/g per ±2g
  accelerazioneY = (ay / 16384.0) - offset_y;
  accelerazioneZ = (az / 16384.0) - offset_z;

  // Calcolo della magnitudine totale dell'accelerazione
  magnitudoAccelerazione = sqrt(pow(accelerazioneX, 2) +
                                pow(accelerazioneY, 2) +
                                pow(accelerazioneZ, 2));

  // Rimozione dell'accelerazione gravitazionale (1g)
  float vibrazione = abs(magnitudoAccelerazione - 1.0);

  // Verifica se la vibrazione supera la soglia
  if (vibrazione > soglia_vibrazione)
  {
    if (!allarme_attivo)
    {
      attiva_allarme();
      Serial.println("ALLARME! Vibrazione rilevata: " + String(vibrazione) + " g");
    }
  }

  // Gestione durata allarme
  if (allarme_attivo && (millis() - tempo_ultimo_allarme > DURATA_ALLARME))
  {
    disattiva_allarme();
  }

  // Output su monitor seriale ogni 500ms
  static unsigned long ultimo_output = 0;
  if (millis() - ultimo_output > 500)
  {
    stampa_dati_sensore();
    ultimo_output = millis();
  }

  delay(50); // Piccola pausa per stabilità letture
}

// Funzione per la calibrazione del sensore
void calibraSensore()
{
  Serial.println("Calibrazione in corso... Mantenere il sensore fermo!");

  float sum_x = 0, sum_y = 0, sum_z = 0;

  for (int i = 0; i < CAMPIONI_CALIBRAZIONE; i++)
  {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    sum_x += ax / 16384.0;
    sum_y += ay / 16384.0;
    sum_z += az / 16384.0;

    delay(10);
  }

  offset_x = sum_x / CAMPIONI_CALIBRAZIONE;
  offset_y = sum_y / CAMPIONI_CALIBRAZIONE;
  offset_z = (sum_z / CAMPIONI_CALIBRAZIONE) - 1.0; // Compensazione gravità

  Serial.println("Calibrazione completata!");
  Serial.println("Offset X: " + String(offset_x));
  Serial.println("Offset Y: " + String(offset_y));
  Serial.println("Offset Z: " + String(offset_z));
}

// Funzione per attivare l'allarme
void attiva_allarme()
{
  allarme_attivo = true;
  tempo_ultimo_allarme = millis();

  digitalWrite(LED_ALLARME, HIGH); // Accende LED rosso

  // Sequenza di beep del buzzer
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

// Funzione per disattivare l'allarme
void disattiva_allarme()
{
  allarme_attivo = false;
  digitalWrite(LED_ALLARME, LOW); // Spegne LED rosso
  digitalWrite(BUZZER, LOW);      // Spegne buzzer
}

// Funzione per stampare i dati del sensore
void stampa_dati_sensore()
{
  Serial.print("Acc X: ");
  Serial.print(accelerazioneX, 3);
  Serial.print(" g | Acc Y: ");
  Serial.print(accelerazioneY, 3);
  Serial.print(" g | Acc Z: ");
  Serial.print(accelerazioneZ, 3);
  Serial.print(" g | Magnitudine: ");
  Serial.print(magnitudoAccelerazione, 3);
  Serial.print(" g | Vibrazione: ");
  Serial.print(abs(magnitudoAccelerazione - 1.0), 3);
  Serial.println(" g");
}
