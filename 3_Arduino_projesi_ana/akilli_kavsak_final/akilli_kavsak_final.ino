/*
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *     AKILLI KAVŞAK - Öncelikli Yön Sistemi
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 
 * SIRA: Kuzey → Batı → Güney → Doğu (saat yönü tersi)
 * 
 * ÖNCELİK SİSTEMİ:
 * - Her yön yeşil yanarken diğer yönler araç sayar
 * - En fazla araç olan yön öncelikli olur
 * - Öncelikli yön bitince normal sıradan devam
 * - Tüm süreler sabit 10 saniye
 * 
 * SARI IŞIK:
 * - Sadece yeşilden kırmızıya geçişte
 * - Kırmızıda bekleyenler hep kırmızı
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
const int YESIL_SURE = 10000;    // Sabit 10 saniye
const int SARI_SURE = 2000;      // 2 saniye

// ========== SENSÖR AYARI ==========
// ARAÇ YOKKEN TEST İÇİN: Eşik çok düşük (hiç araç algılamaz)
// Gerçek test için bu değeri sensöre göre ayarlayacağız
const int SENSOR_ESIK = 50;  // Çok düşük eşik - hiç araç algılamaz

// ========== YÖN TANIMLARI ==========
#define KUZEY 0
#define BATI 1
#define GUNEY 2
#define DOGU 3

// Normal sıra (saat yönü tersi)
const int normalSira[] = {KUZEY, BATI, GUNEY, DOGU};
String yonIsimleri[] = {"KUZEY", "BATI", "GUNEY", "DOGU"};

// ========== DEĞİŞKENLER ==========
int aracSayisi[4] = {0, 0, 0, 0};
bool oncekiDurum[4] = {false, false, false, false};
int siradakiIndex = 0;  // Normal sırada neredeyiz

void setup() {
  Serial.begin(9600);
  
  Serial.println("\n================================");
  Serial.println("  AKILLI KAVŞAK - ONCELIK SISTEMI");
  Serial.println("================================");
  Serial.println("Sira: Kuzey → Bati → Guney → Dogu");
  Serial.println("Oncelik: En fazla arac olan yon");
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
  // Araç sayılarını sıfırla
  for (int i = 0; i < 4; i++) {
    aracSayisi[i] = 0;
  }
  
  // Normal sıradaki yön
  int suankiYon = normalSira[siradakiIndex];
  
  Serial.println("================================");
  Serial.print(">>> ");
  Serial.print(yonIsimleri[suankiYon]);
  Serial.println(" YESIL <<<");
  Serial.println("================================\n");
  
  // Sadece bu yönü yeşil yap
  yonYesil(suankiYon);
  
  // 10 saniye bekle ve diğer yönlerdeki araçları say
  bekleVeSay(YESIL_SURE, suankiYon);
  
  // Yeşilden sarıya geç (sadece bu yön)
  yonSari(suankiYon);
  delay(SARI_SURE);
  
  // Tüm ışıkları kırmızı yap
  tumKirmizi();
  
  // Araç sayılarını göster
  Serial.println("\n--- ARAC SAYILARI ---");
  for (int i = 0; i < 4; i++) {
    if (i != suankiYon && aracSayisi[i] > 0) {
      Serial.print(yonIsimleri[i]);
      Serial.print(": ");
      Serial.print(aracSayisi[i]);
      Serial.println(" arac");
    }
  }
  
  // Öncelikli yön var mı kontrol et
  int oncelikliYon = enFazlaAracOlanYon(suankiYon);
  
  if (oncelikliYon != -1) {
    // Öncelikli yön var!
    Serial.print("\n>>> ONCELIK: ");
    Serial.print(yonIsimleri[oncelikliYon]);
    Serial.print(" (");
    Serial.print(aracSayisi[oncelikliYon]);
    Serial.println(" arac) <<<\n");
    
    delay(1000);
    
    // Öncelikli yönü yeşil yap
    yonYesil(oncelikliYon);
    bekle(YESIL_SURE);
    
    // Yeşilden sarıya geç
    yonSari(oncelikliYon);
    delay(SARI_SURE);
    
    // Tüm ışıkları kırmızı yap
    tumKirmizi();
    
    // Araç sayısını sıfırla
    aracSayisi[oncelikliYon] = 0;
  }
  
  // Normal sırada bir sonraki yöne geç
  siradakiIndex = (siradakiIndex + 1) % 4;
  
  Serial.println("\n--- SONRAKI YON ---\n");
  delay(1000);
}

// ========== SENSÖR FONKSİYONLARI ==========
void sensorleriOku() {
  aracTespit(KUZEY, analogRead(SENSOR_KUZEY));
  aracTespit(BATI, analogRead(SENSOR_BATI));
  aracTespit(GUNEY, analogRead(SENSOR_GUNEY));
  aracTespit(DOGU, analogRead(SENSOR_DOGU));
}

void aracTespit(int yon, int deger) {
  bool aracVar = (deger < SENSOR_ESIK);
  
  if (aracVar && !oncekiDurum[yon]) {
    aracSayisi[yon]++;
    Serial.print("  ");
    Serial.print(yonIsimleri[yon]);
    Serial.print(": +1 arac (Toplam: ");
    Serial.print(aracSayisi[yon]);
    Serial.println(")");
  }
  
  oncekiDurum[yon] = aracVar;
}

void bekleVeSay(int ms, int aktifYon) {
  unsigned long baslangic = millis();
  while (millis() - baslangic < ms) {
    // Aktif yön hariç diğer yönleri say
    for (int i = 0; i < 4; i++) {
      if (i != aktifYon) {
        int pin = 0;
        switch(i) {
          case KUZEY: pin = SENSOR_KUZEY; break;
          case BATI: pin = SENSOR_BATI; break;
          case GUNEY: pin = SENSOR_GUNEY; break;
          case DOGU: pin = SENSOR_DOGU; break;
        }
        aracTespit(i, analogRead(pin));
      }
    }
    delay(50);
  }
}

void bekle(int ms) {
  delay(ms);
}

// ========== ÖNCELİK FONKSİYONU ==========
int enFazlaAracOlanYon(int aktifYon) {
  int maxArac = 0;
  int maxYon = -1;
  
  // En fazla araç olan yönü bul (aktif yön hariç)
  for (int i = 0; i < 4; i++) {
    if (i != aktifYon && aracSayisi[i] > maxArac) {
      maxArac = aracSayisi[i];
      maxYon = i;
    }
  }
  
  // En az 1 araç olmalı
  if (maxArac > 0) {
    return maxYon;
  }
  
  return -1;  // Öncelikli yön yok
}

// ========== IŞIK FONKSİYONLARI ==========
void tumKirmizi() {
  digitalWrite(KUZEY_R, HIGH); digitalWrite(KUZEY_Y, LOW); digitalWrite(KUZEY_G, LOW);
  digitalWrite(BATI_R, HIGH); digitalWrite(BATI_Y, LOW); digitalWrite(BATI_G, LOW);
  digitalWrite(GUNEY_R, HIGH); digitalWrite(GUNEY_Y, LOW); digitalWrite(GUNEY_G, LOW);
  digitalWrite(DOGU_R, HIGH); digitalWrite(DOGU_Y, LOW); digitalWrite(DOGU_G, LOW);
}

void yonYesil(int yon) {
  // Önce hepsini kırmızı yap
  tumKirmizi();
  
  // Sadece bu yönü yeşil yap
  switch(yon) {
    case KUZEY:
      digitalWrite(KUZEY_R, LOW);
      digitalWrite(KUZEY_G, HIGH);
      break;
    case BATI:
      digitalWrite(BATI_R, LOW);
      digitalWrite(BATI_G, HIGH);
      break;
    case GUNEY:
      digitalWrite(GUNEY_R, LOW);
      digitalWrite(GUNEY_G, HIGH);
      break;
    case DOGU:
      digitalWrite(DOGU_R, LOW);
      digitalWrite(DOGU_G, HIGH);
      break;
  }
}

void yonSari(int yon) {
  // Sadece bu yönü sarı yap (diğerleri kırmızı kalır)
  switch(yon) {
    case KUZEY:
      digitalWrite(KUZEY_G, LOW);
      digitalWrite(KUZEY_Y, HIGH);
      break;
    case BATI:
      digitalWrite(BATI_G, LOW);
      digitalWrite(BATI_Y, HIGH);
      break;
    case GUNEY:
      digitalWrite(GUNEY_G, LOW);
      digitalWrite(GUNEY_Y, HIGH);
      break;
    case DOGU:
      digitalWrite(DOGU_G, LOW);
      digitalWrite(DOGU_Y, HIGH);
      break;
  }
}



