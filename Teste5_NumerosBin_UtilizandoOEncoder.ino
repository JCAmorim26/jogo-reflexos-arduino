// Array com os números que serão sorteados (0 a 7)
byte numeros[8] = {0, 1, 2, 3, 4, 5, 6, 7};
long pequeno;
byte b;

// Definição dos pinos
const int pinoBotao = 8;     // Pino SW do encoder
const int pinoEnable138 = 9; // Ativação do 74HC138
const int pinoCLK = 10;      // Pino CLK do encoder
const int pinoDT = 11;       // Pino DT do encoder

// Variáveis de controle do Encoder e Nível
int estadoAnteriorCLK;
int nivel = 1; // Nível inicial padrão
// Array com os tempos de delay em milissegundos para os níveis 1, 2, 3 e 4
const unsigned long tempos[4] = {5000, 3500, 2000, 1000};

// Função que implementa o algoritmo de Fisher-Yates
void embaralhar() {
  for (int i = 7; i > 0; i--) {
    int j = random(i + 1);
    byte temp = numeros[i];
    numeros[i] = numeros[j];
    numeros[j] = temp;
  }
}

void setup() {
  // Configura os pinos 2, 3 e 4 como saída (bits 2, 3 e 4 do PORTD)
  DDRD = B11100; 
  
  // Configura o pino do 74HC138
  pinMode(pinoEnable138, OUTPUT);
  digitalWrite(pinoEnable138, LOW);

  // Configura os pinos do Encoder
  pinMode(pinoBotao, INPUT_PULLUP);
  pinMode(pinoCLK, INPUT);
  pinMode(pinoDT, INPUT);
  
  // Lê o estado inicial do pino CLK para referência do encoder
  estadoAnteriorCLK = digitalRead(pinoCLK);

  Serial.begin(9600);
  randomSeed(analogRead(0)); 

  Serial.println("Sistema pronto!");
  Serial.print("Nivel atual: ");
  Serial.print(nivel);
  Serial.print(" (Delay: ");
  Serial.print(tempos[nivel - 1]);
  Serial.println(" ms)");
  Serial.println("Gire o encoder para mudar a dificuldade, ou pressione para iniciar.");
}

void loop() {
  // 1. LEITURA DO GIRO DO ENCODER (Ajuste de Nível)
  int estadoAtualCLK = digitalRead(pinoCLK);
  
  // Se o estado mudou e agora é HIGH, houve um "passo" (rotação)
  if (estadoAtualCLK != estadoAnteriorCLK && estadoAtualCLK == 1) {
    // Verifica o pino DT para determinar a direção
    if (digitalRead(pinoDT) != estadoAtualCLK) {
      nivel++; // Sentido horário (Direita) - Aumenta nível
      if(nivel > 4) nivel = 4; // Limita no nível 4
    } else {
      nivel--; // Sentido anti-horário (Esquerda) - Diminui nível
      if(nivel < 1) nivel = 1; // Limita no nível 1
    }
    
    Serial.print("Dificuldade alterada para: Nivel ");
    Serial.print(nivel);
    Serial.print(" (Delay: ");
    Serial.print(tempos[nivel - 1]);
    Serial.println(" ms)");
  }
  
  // Salva o estado atual para a próxima volta do loop
  estadoAnteriorCLK = estadoAtualCLK;


  // 2. LEITURA DO BOTÃO DO ENCODER (Início do Sorteio)
  if (digitalRead(pinoBotao) == LOW) {
    delay(50); // Debounce
    
    if (digitalRead(pinoBotao) == LOW) {
      Serial.println("\n--- Jogo Iniciado! ---");
      
      digitalWrite(pinoEnable138, HIGH); // Ativa o 74HC138
      embaralhar();

      // Pega o tempo de delay correspondente ao nível atual (índice 0 a 3)
      unsigned long delayDoSorteio = tempos[nivel - 1];

      // Sorteia os 8 números com a velocidade escolhida
      for (int indice = 0; indice < 8; indice++) {
        pequeno = numeros[indice];
        b = (byte)pequeno;

        Serial.print("Numero = ");
        Serial.print(pequeno);
        Serial.print(" | Binario = ");
        for (int i = 7; i >= 0; i--) {
          Serial.print(bitRead(b, i));
        }
        Serial.println(); 
        
        // Atualiza os pinos 2, 3 e 4
        PORTD = (b << 2);

        // Aplica o delay dinâmico baseado no nível
        delay(delayDoSorteio); 
      }

      digitalWrite(pinoEnable138, LOW); // Desativa o 74HC138 no final
      PORTD = (0 << 2); // Limpa as saídas do Arduino

      Serial.println("--- Fim da rodada. Ajuste o nivel ou pressione para jogar de novo. ---");

      // Trava aqui até você soltar o botão, caso tenha segurado tempo demais
      while(digitalRead(pinoBotao) == LOW) {
        delay(10);
      }
      
      // Atualiza o estado do CLK após sair do loop bloqueante do botão 
      // para evitar que ele registre um giro falso ao terminar o jogo
      estadoAnteriorCLK = digitalRead(pinoCLK);
    }
  }
}