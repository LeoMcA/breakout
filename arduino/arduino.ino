void setup () {
  Serial.begin(9600);
}

void loop () {
  int value = analogRead(A0);
  int asByte = value / 4;
  if (!asByte) asByte++;
  Serial.write(asByte);
  delay(10);
}
