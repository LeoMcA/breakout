void setup () {
  Serial.begin(9600);
}

void loop () {
  int value = analogRead(A0);
  int byte = value / 4;
  if (!byte) byte++; // don't send 0's
  Serial.write(byte);
  delay(10);
}
