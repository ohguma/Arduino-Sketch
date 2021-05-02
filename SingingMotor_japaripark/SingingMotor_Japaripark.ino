/*
  「ようこそジャパリパークへ」を鳴らすモータ
   2017-04-09 ohguma
  
  (参考URL)
  http://triring.net/program/Arduino/SingingMotor/index.html

  ==使用機器
  - Strwaberry-linux
  ダ・ヴィンチ32U with Arduino Bootloader
  https://strawberry-linux.com/catalog/items?code=25005

  - HiLetgo
  デュアルDC ステッパ モータードライバー TB6612FNG モジュール
  https://www.amazon.co.jp/dp/B014GZ9JTC

  ==変更点
  参考URLのSingingMotorプログラムをそのまま使うとPWM duty比が50%で
  モーターが回ってしまうため、ONの期間を十分短い時間に変更した。

*/

#include "note.h"
#include "music_data.h"


#define SW_START 7

#define AIN1 15
#define AIN2 17
#define PWMA 5

#define BIN1 16
#define BIN2 14
#define PWMB 11

void setup()
{
  Serial.begin(9600);
  pinMode(SW_START,  INPUT_PULLUP);
  pinMode(AIN1,  OUTPUT);
  pinMode(AIN2,  OUTPUT);
  pinMode(PWMA,   OUTPUT);
  pinMode(BIN1,  OUTPUT);
  pinMode(BIN2,  OUTPUT);
  pinMode(PWMB,   OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  Serial.print(millis());
  Serial.print(":");
  if (digitalRead(SW_START) == LOW) {
    Serial.println("play");
    playMusicFan(WelcomeJapariPark);
  } else {
    Serial.println("wait");
  }
  delay(50);
}

void motorCw() {
  digitalWrite(AIN1, HIGH );
  digitalWrite(AIN2, LOW  );
  digitalWrite(BIN1, HIGH );
  digitalWrite(BIN2, LOW  );
}

void motorStop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void playFan(int freq, long duration)
{
  if (freq == 0) {
    stopFan(duration);
    return;    
  }
  int wavelength = (int)(1000000L / freq);
  int on_wavelength = 100;
  int off_wavelength = wavelength - on_wavelength;
  long endTime = micros() + duration;

  motorCw(); // モーター正回転
  while (endTime > micros()) {
    digitalWrite(PWMA, HIGH);   // sets the pin on
    digitalWrite(PWMB, HIGH);   // sets the pin on
    delayMicroseconds(on_wavelength);        // pauses for 50 microseconds
    digitalWrite(PWMA, LOW);    // sets the pin off
    digitalWrite(PWMB, LOW);    // sets the pin off
    delayMicroseconds(off_wavelength);        // pauses for 50 microseconds
  }
  motorStop(); // モーター停止
}

void stopFan(long duration) 
{
    motorStop(); // モーター停止
    delayMicroseconds(duration);        // pauses for 50 microseconds
}

void playMusicFan(unsigned char *data)
{
  int i;
  int flag = HIGH;
  int freq;
  long tempo = 120L;
  /*
    テンポの変換係数
    (60000 / tempo) / 480 =(60000 / 480)/tempo = 125/tempo
  */
  tempo = (long)data[0];
  long tick = 1250000L / tempo;
  i = 2;
  do {
    if (flag == HIGH) flag = LOW ;
    else              flag = HIGH;
    freq = NoteCode2Freq(data[i]);
    playFan(freq, (long)data[i + 1] * tick);
    i += 2;
    digitalWrite(LED_BUILTIN, flag);
  } while (data[i] != 0);
  digitalWrite(LED_BUILTIN, LOW );
}

int NoteCode2Freq(byte note_code)
{
  int freq = 0;

  int octave = (int)((note_code & 0x00f0) >> 4);
  int note   = (int)( note_code & 0x000f      );
  //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
  int inv_octave[] = {   1, 128, 64, 32, 16,  8,  4,  2,  1,  1,  1,  1,  1,  1,  1,  1 };

  switch (note) {
    case  0 : freq = 0;                                             break;
    case  1 : freq = (int)((42240L / inv_octave[octave] + 5) / 10); break;   // C  ド
    case  2 : freq = (int)((45056L / inv_octave[octave] + 5) / 10); break;   // C#
    case  3 : freq = (int)((47520L / inv_octave[octave] + 5) / 10); break;   // D  レ
    case  4 : freq = (int)((49920L / inv_octave[octave] + 5) / 10); break;   // D#
    case  5 : freq = (int)((52800L / inv_octave[octave] + 5) / 10); break;   // E  ミ
    case  6 : freq = (int)((56320L / inv_octave[octave] + 5) / 10); break;   // F  ファ
    case  7 : freq = (int)((59400L / inv_octave[octave] + 5) / 10); break;   // F#
    case  8 : freq = (int)((63360L / inv_octave[octave] + 5) / 10); break;   // G  ソ
    case 99 : freq = 8000;  /* この部分のcaseは無視されるので、dummyを挿入 */  break;
    case  9 : freq = (int)((67584L / inv_octave[octave] + 5) / 10); break;   // G#
    case 10 : freq = (int)((70400L / inv_octave[octave] + 5) / 10); break;   // A  ラ
    case 11 : freq = (int)((76032L / inv_octave[octave] + 5) / 10); break;   // A#
    case 12 : freq = (int)((79200L / inv_octave[octave] + 5) / 10); break;   // B  シ
    default : freq = 0;                                             break;
  }
  if (octave == 0 ) freq = 0;
  return freq;
}
