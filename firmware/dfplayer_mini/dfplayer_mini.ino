#include <DFRobotDFPlayerMini.h>

HardwareSerial dfSerial(1);  // UART2
DFRobotDFPlayerMini dfPlayer;

bool hasReset = false;

void setup()
{
  Serial.begin(115200);
  delay(3000);  // IMPORTANT: Wait for DFPlayer to boot

  // UART2: RX, TX
  dfSerial.begin(9600, SERIAL_8N1, 26, 25);

  Serial.println("Initializing DFPlayer...");

  if (!dfPlayer.begin(dfSerial))
  {
    Serial.println("DFPlayer not found!");
    while (true);
  }

  Serial.println("DFPlayer ready.");

  dfPlayer.volume(22);  // Range: 0-30
}

void loop()
{
  for (int i=1; i<22; i++) {
    delay(5000);
    Serial.println("Playing 0002.mp3");
    dfPlayer.playMp3Folder(i);
  }
}