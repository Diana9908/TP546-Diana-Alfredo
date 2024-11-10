#define BLYNK_TEMPLATE_ID "TMPL2YUQtThYw"
#define BLYNK_TEMPLATE_NAME "Sistema de dissuasão e monitoramento"
#define BLYNK_AUTH_TOKEN "Nq-4nNswLvQr0dcyfJd1ehRFcd8crjk7"
#define DISTANCIA_MAXIMA 300
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <NewPing.h>

const int pinoTrig = D1;
const int pinoEcho = D2;
const int pinoAlarme = D3;
const int pinoVibracao = D5;

NewPing sonar(pinoTrig, pinoEcho, DISTANCIA_MAXIMA);

int distancia;
bool alarmeAtivo = false;
bool ledVirtualAtivo = false;
unsigned long inicioAlarme = 0;
unsigned long inicioLedVibracao = 0;
const unsigned long duracaoAlarme = 3000;
const unsigned long duracaoLedVibracao = 3000;
const unsigned long duracaoMaxVibracao = 1000;
const unsigned long intervaloVibracao = 2000;
unsigned long ultimaVibracao = 0;
unsigned long ultimaAtivacaoVibracao = 0;

const unsigned long ignorarUltrassonico = 2000;

char auth[] = "Nq-4nNswLvQr0dcyfJd1ehRFcd8crjk7";
char ssid[] = "DianaGalaxy";
char senha[] = "LaVidaesBella*99";

BlynkTimer temporizador;
void configurarPinos();
void enviarDadosSensor();
void verificarAlarmeELed();
void detectarVibracao();
int obterDistanciaConsistente();

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, senha);
  configurarPinos();
  temporizador.setInterval(1000L, enviarDadosSensor);
  temporizador.setInterval(100L, verificarAlarmeELed);
  temporizador.setInterval(100L, detectarVibracao);
}

void loop()
{
  Blynk.run();
  temporizador.run();
}

void configurarPinos()
{
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoAlarme, OUTPUT);
  pinMode(pinoVibracao, INPUT);

  digitalWrite(pinoAlarme, LOW);
}

int obterDistanciaConsistente()
{
  int d1 = sonar.ping_cm();
  delay(30);
  int d2 = sonar.ping_cm();
  delay(30);
  int d3 = sonar.ping_cm();

  if (abs(d1 - d2) < 5 && abs(d2 - d3) < 5)
  {
    return (d1 + d2 + d3) / 3;
  }
  else
  {
    return -1;
  }
}

void enviarDadosSensor()
{
  if (millis() - ultimaVibracao < ignorarUltrassonico)
  {
    return;
  }

  int distanciaConsistente = obterDistanciaConsistente();
  if (distanciaConsistente == -1)
  {
    Serial.println("Leitura de distância inconsistente");
    return;
  }
  distancia = distanciaConsistente;
  Blynk.virtualWrite(V1, distancia);

  if (distancia > 0 && distancia <= 20 && !alarmeAtivo)
  {
    digitalWrite(pinoAlarme, HIGH);
    Blynk.virtualWrite(V0, 200);
    alarmeAtivo = true;
    inicioAlarme = millis();
    Blynk.logEvent("movimento_detectado", "Alerta! Movimento detectado a menos de 1 metro.");
  }
}

void detectarVibracao()
{
  unsigned long tempoAtual = millis();
  if (digitalRead(pinoVibracao) == HIGH)
  {
    if (ultimaAtivacaoVibracao == 0)
    {
      ultimaAtivacaoVibracao = tempoAtual;
    }
    else if (tempoAtual - ultimaAtivacaoVibracao > duracaoMaxVibracao)
    {
      return;
    }
    if (!ledVirtualAtivo)
    {
      Blynk.virtualWrite(V4, 200);
      ledVirtualAtivo = true;
      inicioLedVibracao = tempoAtual;
      ultimaVibracao = tempoAtual;
      Serial.println("Vibração detectada");
      Blynk.logEvent("intrusao_detectada", "Alerta! Intruso detectado.");
    }
  }
  else
  {
    ultimaAtivacaoVibracao = 0;
  }
}

void verificarAlarmeELed()
{
  if (alarmeAtivo && (millis() - inicioAlarme >= duracaoAlarme))
  {
    digitalWrite(pinoAlarme, LOW);
    Blynk.virtualWrite(V0, 0);
    alarmeAtivo = false;
  }

  if (ledVirtualAtivo && (millis() - inicioLedVibracao >= duracaoLedVibracao))
  {
    Blynk.virtualWrite(V4, 0);
    ledVirtualAtivo = false;
  }
}
