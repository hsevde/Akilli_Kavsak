/*
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *     AKILLI KAVŞAK - Araç Sayımı + Optimizasyon
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 
 * ÖZELLİKLER:
 * - 4 yönde araç tespiti (IR sensör)
 * - Araç sayısına göre yeşil ışık süresi
 * - Akıllı trafik optimizasyonu
 * - Serial Monitor'de canlı istatistik
 */

// ========== TRAFİK IŞIKLARI ==========
// Kuzey (Yukarı)
const int KUZEY_R = 2;
const int KUZEY_Y = 3;
const int KUZEY_G = 4;

// Güney (Aşağı)
const int GUNEY_R = 5;
const int GUNEY_Y = 6;
const int GUNEY_G = 7;

// Doğu (Sağ)
const int DOGU_R = 8;
const int DOGU_Y = 9;
const int DOGU_G = 10;

// Batı (Sol)
const int BATI_R = 11;
const int BATI_Y = 12;
const int BATI_G = 13;

// ========== SENSÖRLER ==========
const int SENSOR_KUZEY = A0;
const int SENSOR_GUNEY = A1;
const int SENSOR_DOGU = A2;
const int SENSOR_BATI = A3;

// ========== ZAMAN AYARLARI ==========
const int MIN_YESIL = 3000;      // Minimum yeşil: 3 saniye
const int MAX_YESIL = 10000;     // Maksimum yeşil: 10 saniye
const int ARAC_BASINA = 1000;    // Her araç için +1 saniye
const int SARI_SURE = 2000;      // Sarı ışık: 2 saniye

// ========== SENSÖR AYARLARI ==========
const int SENSOR_ESIK = 500;     // IR sensör eşik değeri

// ========== DEĞİŞKENLER ==========
int aracSayisi[4] = {0, 0, 0, 0};           // [Kuzey, Güney, Doğu, Batı]
bool oncekiDurum[4] = {false, false, false, false};
int toplamArac = 0;

String yonIsimleri[] = {"KUZEY", "GUNEY", "DOGU", "BATI"};

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);
  
  // Başlık
  Serial.println("\n");
  Serial.println("================================");
  Serial.println("   AKILLI KAVSAK SISTEMI");
  Serial.println("================================");
  Serial.println();
  
  // Trafik ışığı pinleri
  pinMode(KUZEY_R, OUTPUT);
  pinMode(KUZEY_Y, OUTPUT);
  pinMode(KUZEY_G, OUTPUT);
  
  pinMode(GUNEY_R, OUTPUT);
  pinMode(GUNEY_Y, OUTPUT);
  pinMode(GUNEY_G, OUTPUT);
  
  pinMode(DOGU_R, OUTPUT);
  pinMode(DOGU_Y, OUTPUT);
  pinMode(DOGU_G, OUTPUT);
  
  pinMode(BATI_R, OUTPUT);
  pinMode(BATI_Y, OUTPUT);
  pinMode(BATI_G, OUTPUT);
  
  // Sensör pinleri
  pinMode(SENSOR_KUZEY, INPUT);
  pinMode(SENSOR_GUNEY, INPUT);
  pinMode(SENSOR_DOGU, INPUT);
  pinMode(SENSOR_BATI, INPUT);
  
  // Başlangıçta hepsi kırmızı
  tumKirmizi();
  
  Serial.println("Sistem hazir!");
  Serial.println("Arabalari gecirin...\n");
  delay(2000);
}

// ========== ANA DÖNGÜ ==========
void loop() {
  // 1. Sensörleri oku ve araç say
  sensorleriOku();
  
  // 2. Toplam araç sayısı
  int toplamKuzeyGuney = aracSayisi[0] + aracSayisi[1];
  int toplamDoguBati = aracSayisi[2] + aracSayisi[3];
  
  // 3. Yeşil sürelerini hesapla
  int kgSure = yesilSureHesapla(toplamKuzeyGuney);
  int dbSure = yesilSureHesapla(toplamDoguBati);
  
  // 4. İstatistikleri göster
  istatistikGoster(kgSure, dbSure);
  
  // 5. KUZEY-GÜNEY FAZ
  Serial.println("\n>>> KUZEY-GUNEY YESIL <<<");
  kuzeyGuneyYesil();
  bekle(kgSure);
  
  // Sarı geçiş
  tumSari();
  bekle(SARI_SURE);
  
  // 6. DOĞU-BATI FAZ
  Serial.println("\n>>> DOGU-BATI YESIL <<<");
  doguBatiYesil();
  bekle(dbSure);
  
  // Sarı geçiş
  tumSari();
  bekle(SARI_SURE);
  
  // 7. Sayıları sıfırla
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
  
  // Yükselen kenar (yeni araç geldi)
  if (aracVar && !oncekiDurum[index]) {
    aracSayisi[index]++;
    toplamArac++;
    
    Serial.print(">>> ARAC TESPITI: ");
    Serial.print(yonIsimleri[index]);
    Serial.print(" (+1) = ");
    Serial.println(aracSayisi[index]);
  }
  
  oncekiDurum[index] = aracVar;
}

void bekle(int ms) {
  unsigned long baslangic = millis();
  while (millis() - baslangic < ms) {
    sensorleriOku();  // Beklerken de say
    delay(50);
  }
}

// ========== IŞIK FONKSİYONLARI ==========
void tumKirmizi() {
  digitalWrite(KUZEY_R, HIGH); digitalWrite(KUZEY_Y, LOW); digitalWrite(KUZEY_G, LOW);
  digitalWrite(GUNEY_R, HIGH); digitalWrite(GUNEY_Y, LOW); digitalWrite(GUNEY_G, LOW);
  digitalWrite(DOGU_R, HIGH);  digitalWrite(DOGU_Y, LOW);  digitalWrite(DOGU_G, LOW);
  digitalWrite(BATI_R, HIGH);  digitalWrite(BATI_Y, LOW);  digitalWrite(BATI_G, LOW);
}

void tumSari() {
  digitalWrite(KUZEY_R, LOW); digitalWrite(KUZEY_Y, HIGH); digitalWrite(KUZEY_G, LOW);
  digitalWrite(GUNEY_R, LOW); digitalWrite(GUNEY_Y, HIGH); digitalWrite(GUNEY_G, LOW);
  digitalWrite(DOGU_R, LOW);  digitalWrite(DOGU_Y, HIGH);  digitalWrite(DOGU_G, LOW);
  digitalWrite(BATI_R, LOW);  digitalWrite(BATI_Y, HIGH);  digitalWrite(BATI_G, LOW);
}

void kuzeyGuneyYesil() {
  // Kuzey-Güney yeşil
  digitalWrite(KUZEY_R, LOW); digitalWrite(KUZEY_Y, LOW); digitalWrite(KUZEY_G, HIGH);
  digitalWrite(GUNEY_R, LOW); digitalWrite(GUNEY_Y, LOW); digitalWrite(GUNEY_G, HIGH);
  // Doğu-Batı kırmızı
  digitalWrite(DOGU_R, HIGH);  digitalWrite(DOGU_Y, LOW);  digitalWrite(DOGU_G, LOW);
  digitalWrite(BATI_R, HIGH);  digitalWrite(BATI_Y, LOW);  digitalWrite(BATI_G, LOW);
}

void doguBatiYesil() {
  // Kuzey-Güney kırmızı
  digitalWrite(KUZEY_R, HIGH); digitalWrite(KUZEY_Y, LOW); digitalWrite(KUZEY_G, LOW);
  digitalWrite(GUNEY_R, HIGH); digitalWrite(GUNEY_Y, LOW); digitalWrite(GUNEY_G, LOW);
  // Doğu-Batı yeşil
  digitalWrite(DOGU_R, LOW);  digitalWrite(DOGU_Y, LOW);  digitalWrite(DOGU_G, HIGH);
  digitalWrite(BATI_R, LOW);  digitalWrite(BATI_Y, LOW);  digitalWrite(BATI_G, HIGH);
}

// ========== HESAPLAMA ==========
int yesilSureHesapla(int aracSayisi) {
  int sure = MIN_YESIL + (aracSayisi * ARAC_BASINA);
  if (sure > MAX_YESIL) sure = MAX_YESIL;
  return sure;
}

// ========== İSTATİSTİK ==========
void istatistikGoster(int kgSure, int dbSure) {
  Serial.println("\n================================");
  Serial.println("   ARAC SAYILARI");
  Serial.println("================================");
  Serial.print("  Kuzey: "); Serial.print(aracSayisi[0]); Serial.println(" arac");
  Serial.print("  Guney: "); Serial.print(aracSayisi[1]); Serial.println(" arac");
  Serial.print("  Dogu:  "); Serial.print(aracSayisi[2]); Serial.println(" arac");
  Serial.print("  Bati:  "); Serial.print(aracSayisi[3]); Serial.println(" arac");
  Serial.println("--------------------------------");
  Serial.print("  Toplam: "); Serial.print(toplamArac); Serial.println(" arac");
  Serial.println("================================");
  Serial.println("   YESIL ISIK SURELERI");
  Serial.println("================================");
  Serial.print("  Kuzey-Guney: "); Serial.print(kgSure/1000); Serial.println(" saniye");
  Serial.print("  Dogu-Bati:   "); Serial.print(dbSure/1000); Serial.println(" saniye");
  Serial.println("================================\n");
}

