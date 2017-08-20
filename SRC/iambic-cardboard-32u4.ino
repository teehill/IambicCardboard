#include <Keyboard.h>
#define DIT 15
#define DAH 10
#define LED 1
#define SPEAKER 9
#define DITLEN A1
// THRESH is the count before the input pin goes high. I only needed 1.
#define THRESH 1
#define TONE 2600
#define WPM 18
#define NOTE 100

byte morse_buffer[33];
int  mbp = 0;

int dot_length;
int word_space;
int letter_space;

unsigned long last_time = millis();

int word_length = 0;
int new_word = 0;

int timepoint()
{
   unsigned long t = millis();
   int td = (int) (t - last_time);
   last_time = t;
   return td;
}

void push(char c)
{
   if (mbp < 31) 
   {
      morse_buffer[mbp] = c;
      mbp++; 
   }
}

void type(char c)
{
   Keyboard.print(c);
   Serial.println(c);
   if ((c == ' ') || (c == '\n')) new_word = 1;
   else if (new_word) 
   {
      word_length = 0;
      new_word = 0;
   }

   word_length++;
}

void mistake()
{
   int i;
   for (i=0; i<word_length; i++) Keyboard.print((char) 0x8);
   word_length = 0;
}

boolean readbutton(int button){
  volatile uint8_t* ddr;
  volatile uint8_t* port;
  volatile uint8_t* pin;
  byte bitmask;
  unsigned int count=0;
  port=portOutputRegister(digitalPinToPort(button));
  ddr=portModeRegister(digitalPinToPort(button));
  bitmask=digitalPinToBitMask(button);
  pin=portInputRegister(digitalPinToPort(button));
  *port &= ~(bitmask);
  *ddr |= bitmask;
  delay(1);
  *ddr &= ~(bitmask);
  *port |= bitmask;  // comment this out if you install 100k pullups
                     // on the inputs.

  while ( ! (*pin & bitmask)) {
    count++;
  }
  *port &= ~(bitmask);
  *ddr  |= bitmask;

  return(count>THRESH?1:0);
}

void translate()
{
  Serial.println((char *) morse_buffer);
  if (mbp == 1)
  {
    if (morse_buffer[0] == '-') type('t');
    else type('e');
  }
  else if (mbp == 2)
  {
    if ((morse_buffer[0] == '-') && (morse_buffer[1] == '-')) type('m');
    else if ((morse_buffer[0] == '-') && (morse_buffer[1] == '*')) type('n');
    else if ((morse_buffer[0] == '*') && (morse_buffer[1] == '-')) type('a');
    else type('i');
  }
  else if (mbp == 3)
  {
    if ((morse_buffer[0] == '-') && (morse_buffer[1] == '-') && (morse_buffer[2] == '-')) type('o');
    else if ((morse_buffer[0] == '-') && (morse_buffer[1] == '-') && (morse_buffer[2] == '*')) type('g');
    else if ((morse_buffer[0] == '-') && (morse_buffer[1] == '*') && (morse_buffer[2] == '-')) type('k');
    else if ((morse_buffer[0] == '-') && (morse_buffer[1] == '*') && (morse_buffer[2] == '*')) type('d');
    else if ((morse_buffer[0] == '*') && (morse_buffer[1] == '-') && (morse_buffer[2] == '-')) type('w');
    else if ((morse_buffer[0] == '*') && (morse_buffer[1] == '-') && (morse_buffer[2] == '*')) type('r');
    else if ((morse_buffer[0] == '*') && (morse_buffer[1] == '*') && (morse_buffer[2] == '-')) type('u');
    else type('s');
  }
  else if (mbp == 4)
  {
     if (morse_buffer[0] == '-')
     {
         if ((morse_buffer[1] == '*') && (morse_buffer[2] == '*') && (morse_buffer[3] == '*')) type('b');
         else if ((morse_buffer[1] == '*') && (morse_buffer[2] == '-') && (morse_buffer[3] == '*')) type('c');
         else if ((morse_buffer[1] == '-') && (morse_buffer[2] == '*') && (morse_buffer[3] == '-')) type('q');
         else if ((morse_buffer[1] == '*') && (morse_buffer[2] == '*') && (morse_buffer[3] == '-')) type('x');
         else if ((morse_buffer[1] == '*') && (morse_buffer[2] == '-') && (morse_buffer[3] == '-')) type('y');
         else type('z');
     }
     else
     {
         if ((morse_buffer[1] == '*') && (morse_buffer[2] == '-') && (morse_buffer[3] == '*')) type('f');
         else if ((morse_buffer[1] == '*') && (morse_buffer[2] == '*') && (morse_buffer[3] == '*')) type('h');
         else if ((morse_buffer[1] == '-') && (morse_buffer[2] == '-') && (morse_buffer[3] == '-')) type('j');
         else if ((morse_buffer[1] == '-') && (morse_buffer[2] == '*') && (morse_buffer[3] == '-')) type('\n');
         else if ((morse_buffer[1] == '-') && (morse_buffer[2] == '*') && (morse_buffer[3] == '*')) type('l');
         else if ((morse_buffer[1] == '-') && (morse_buffer[2] == '-') && (morse_buffer[3] == '*')) type('p');
         else type('v');
     }
  }
  else if (mbp == 8) mistake();

  mbp = 0;
  memset(morse_buffer, 0, 32);
  }

void setup (){
  pinMode(DIT, INPUT);
  pinMode(DAH, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(SPEAKER, OUTPUT);

  dot_length = 1200 / WPM;
  word_space = dot_length * 10;
  letter_space = dot_length * 3;

  memset(morse_buffer, 0, 33);
}

void loop(){
  boolean dit,dah;
  
  dit=readbutton(DIT);
  dah=readbutton(DAH);

  if (dit) 
  {
    tone(SPEAKER,TONE,NOTE);
    delay(NOTE);
    push('*');
  } 
  else if (dah)
  {
    digitalWrite(LED, HIGH);
    tone(SPEAKER,TONE, NOTE*3);
    delay(NOTE*3);
    push('-');
  } else if (mbp > 0)
  {
    int gap = timepoint();
    if ((gap > word_space) && (!new_word)) type(' ');
    if (gap > letter_space) 
    { 
      translate();
    }
  }
}