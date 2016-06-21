unsigned char Char;   // 1 byte
unsigned short Short; // 2 byte
unsigned int Int;     // 4 byte
unsigned long Long;   // 4 byte
float Float;          // 4 byte

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Sizes:");
  Serial.println(sizeof(Char));
  Serial.println(sizeof(Short));
  Serial.println(sizeof(Int));
  Serial.println(sizeof(Long));
  Serial.println(sizeof(Float));
}
