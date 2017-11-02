void setup () {
  Serial.begin(9600);
}

void loop () {
  int potentiometerValue = analogRead(A0);
  Serial.println(potentiometerValue);
}
