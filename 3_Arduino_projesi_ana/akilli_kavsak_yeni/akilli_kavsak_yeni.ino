/*
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *     AKILLI KAVŞAK - Tek Tek Yeşil (Saat Yönünde)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 
 * ÇALIŞMA MANTIĞI:
 * 1. KUZEY yeşil  → Diğerleri kırmızı
 * 2. GÜNEY yeşil  → Diğerleri kırmızı
 * 3. DOĞU yeşil   → Diğerleri kırmızı
 * 4. BATI yeşil   → Diğerleri kırmızı
 * 5. Tekrar başa dön (saat yönünde)
 * 
 * OPTİMİZASYON:
 * - Her yönde geçen araç sayısı sensörle tespit edilir
 * - Araç sayısı fazla → Yeşil süre uzun
 * - Araç sayısı az → Yeşil süre kısa
 */

// ========== TRAFİK IŞIKLARI ==========
const int KUZEY_R = 2, KUZEY_Y = 3, KUZEY_G = 4;
const int GUNEY_R = 5, GUNEY_Y = 6, GUNEY_G = 7;
const int DOGU_R = 8, DOGU_Y = 9, DOGU_G = 10;
const int BATI_R = 11, BATI_Y = 12, BATI_G = 13;

// ========== SENSÖRLER ==========
const int SENSOR_KUZEY = A0;
const int SENSOR_GUNEY = A1;
const int SENSOR_DOGU = A2;
const int SENSOR_BATI = A3;

// ========== ZAMAN AYARLARI ==========
const int MIN_YESIL = 3000;      // Minimum: 3 saniye
const int MAX_YESIL = 10000;     // Maksimum: 10 saniye
const int ARAC_BASINA = 1000;    // Her araç +1 saniye
const int SARI_SURE = 2000;      // Sarı: 2 saniye

// ========== SENSÖR AYARI ==========
const int SENSOR_ESIK = 500;

// ========== DEĞİŞKENLER ==========
int aracSayisi[4] = {0, 0, 0, 0};  // [Kuzey, Güney, Doğu, Batı]
bool oncekiDurum[4] = {false, false, false, false};
String yonIsimleri[] = {"KUZEY", "GUNEY", "DOGU", "BATI"};

void setup() {
  Serial.begin(9600);
  
  Serial.println("\n================================");
  Serial.println("  AKILLI KAVSAK - TEK TEK YESIL");
  Serial.println("================================\n");
  
  // LED pinleri
  for (int pin = 2; pin <= 13; pin++) {
    pinMode(pin, OUTPUT);
  }
  
  // Sensör pinleri
  pinMode(SENSOR_KUZEY, INPUT);
  pinMode(SENSOR_GUNEY, INPUT);
  pinMode(SENSOR_DOGU, INPUT);
  pinMode(SENSOR_BATI, INPUT);
  
  tumKirmizi();
  Serial.println("Sistem hazir!\n");
  delay(2000);
}

void loop() {
  // Başlangıçta hepsi kırmızı
  tumKirmizi();
  
  // Sensörleri oku ve araç say (30 saniye)
  Serial.println(">>> ARAC SAYIMI BASLADI (30 saniye) <<<\n");
  unsigned long sayimBaslangic = millis();
  while (millis() - sayimBaslangic < 30000) {
    sensorleriOku();
    delay(100);
  }
  
  // İstatistikleri göster
  Serial.println("\n================================");
  Serial.println("  ARAC SAYILARI");
  Serial.println("================================");
  for (int i = 0; i < 4; i++) {
    Serial.print("  ");
    Serial.print(yonIsimleri[i]);
    Serial.print(": ");
    Serial.print(aracSayisi[i]);
    Serial.println(" arac");
  }
  Serial.println("================================\n");
  
  // SAAT YÖNÜNDE her yön sırayla
  
  // 1. KUZEY YESIL
  int kuzeySure = yesilSureHesapla(aracSayisi[0]);
  Serial.print(">>> KUZEY YESIL (");
  Serial.print(kuzeySure/1000);
  Serial.println(" saniye) <<<");
  sadeceyonYesil(0);  // Sadece Kuzey yeşil
  bekle(kuzeySure);
  tumSari();
  bekle(SARI_SURE);
  
  // 2. GÜNEY YESIL
  int guneySure = yesilSureHesapla(aracSayisi[1]);
  Serial.print(">>> GUNEY YESIL (");
  Serial.print(guneySure/1000);
  Serial.println(" saniye) <<<");
  sadeceyonYesil(1);  // Sadece Güney yeşil
  bekle(guneySure);
  tumSari();
  bekle(SARI_SURE);
  
  // 3. DOĞU YESIL
  int doguSure = yesilSureHesapla(aracSayisi[2]);
  Serial.print(">>> DOGU YESIL (");
  Serial.print(doguSure/1000);
  Serial.println(" saniye) <<<");
  sadeceyonYesil(2);  // Sadece Doğu yeşil
  bekle(doguSure);
  tumSari();
  bekle(SARI_SURE);
  
  // 4. BATI YESIL
  int batiSure = yesilSureHesapla(aracSayisi[3]);
  Serial.print(">>> BATI YESIL (");
  Serial.print(batiSure/1000);
  Serial.println(" saniye) <<<");
  sadeceyonYesil(3);  // Sadece Batı yeşil
  bekle(batiSure);
  tumSari();
  bekle(SARI_SURE);
  
  // Sayıları sıfırla
  for (int i = 0; i < 4; i++) {
    aracSayisi[i] = 0;
  }
  
  Serial.println("\n--- YENI DONGU ---\n");
}

// ========== SENSÖR FONKSİYONLARI ==========
void sensorleriOku() {
  aracTespit(0, analogRead(SENSOR_KUZEY));
  aracTespit(1, analogRead(SENSOR_GUNEY));
  aracTespit(2, analogRead(SENSOR_DOGU));
  aracTespit(3, analogRead(SENSOR_BATI));
}

void aracTespit(int index, int deger) {
  bool aracVar = (deger < SENSOR_ESIK);
  
  if (aracVar && !oncekiDurum[index]) {
    aracSayisi[index]++;
    Serial.print(">>> ");
    Serial.print(yonIsimleri[index]);
    Serial.print(": +1 arac (Toplam: ");
    Serial.print(aracSayisi[index]);
    Serial.println(")");
  }
  
  oncekiDurum[index] = aracVar;
}

void bekle(int ms) {
  unsigned long baslangic = millis();
  while (millis() - baslangic < ms) {
    sensorleriOku();
    delay(50);
  }
}

// ========== IŞIK FONKSİYONLARI ==========
void tumKirmizi() {
  digitalWrite(KUZEY_R, HIGH); digitalWrite(KUZEY_Y, LOW); digitalWrite(KUZEY_G, LOW);
  digitalWrite(GUNEY_R, HIGH); digitalWrite(GUNEY_Y, LOW); digitalWrite(GUNEY_G, LOW);
  digitalWrite(DOGU_R, HIGH); digitalWrite(DOGU_Y, LOW); digitalWrite(DOGU_G, LOW);
  digitalWrite(BATI_R, HIGH); digitalWrite(BATI_Y, LOW); digitalWrite(BATI_G, LOW);
}

void tumSari() {
  digitalWrite(KUZEY_R, LOW); digitalWrite(KUZEY_Y, HIGH); digitalWrite(KUZEY_G, LOW);
  digitalWrite(GUNEY_R, LOW); digitalWrite(GUNEY_Y, HIGH); digitalWrite(GUNEY_G, LOW);
  digitalWrite(DOGU_R, LOW); digitalWrite(DOGU_Y, HIGH); digitalWrite(DOGU_G, LOW);
  digitalWrite(BATI_R, LOW); digitalWrite(BATI_Y, HIGH); digitalWrite(BATI_G, LOW);
}

void sadeceyonYesil(int yon) {
  // Önce hepsini kırmızı yap
  tumKirmizi();
  
  // Sadece belirtilen yönü yeşil yap
  switch(yon) {
    case 0:  // Kuzey
      digitalWrite(KUZEY_R, LOW);
      digitalWrite(KUZEY_G, HIGH);
      break;
    case 1:  // Güney
      digitalWrite(GUNEY_R, LOW);
      digitalWrite(GUNEY_G, HIGH);
      break;
    case 2:  // Doğu
      digitalWrite(DOGU_R, LOW);
      digitalWrite(DOGU_G, HIGH);
      break;
    case 3:  // Batı
      digitalWrite(BATI_R, LOW);
      digitalWrite(BATI_G, HIGH);
      break;
  }
}

// ========== HESAPLAMA ==========
int yesilSureHesapla(int aracSayisi) {
  int sure = MIN_YESIL + (aracSayisi * ARAC_BASINA);
  if (sure > MAX_YESIL) sure = MAX_YESIL;
  return sure;
}

