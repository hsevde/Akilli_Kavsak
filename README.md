# Akıllı Kavşak

Dört yönlü bir kavşakta bekleyen araç sayısına göre yeşil ışığı sıraya alan prototip. Aynı karar mantığının tarayıcıda çalışan bir simülasyonu da var. Donanım olmadan projeyi panelden izlemek mümkün.

## Nasıl çalışır

Sıra kuzey, batı, güney, doğu şeklindedir. Yeşil yanan yönde araç sayılmaz. Diğer üç yöndeki IR sensörler bekleyen araçları sayar. Süre bitince en çok araç biriken yön varsa ışık önce o yöne verilir, sonra normal sıra devam eder. Sarı yalnızca yeşilden kırmızıya geçerken yanar.

Kartta yeşil 10 saniye, sarı 2 saniyedir. Panel aynı sürelerle açılır; ekrandaki alanlardan değiştirilebilir.

## Paneli çalıştırma

`5_IoT_Dashboard/index.html` dosyasını tarayıcıda açın. **Simülasyon** seçili gelir. **Başlat** ile döngü başlar.

Bu mod kart ve sunucu gerektirmez. Araçlar simülasyonda kendiliğinden eklenir, ışıklar ve öncelik ekranda güncellenir.

## Arduino

Karta yüklenecek dosya:

`3_Arduino_projesi_ana/akilli_kavsak_final/akilli_kavsak_final.ino`

Aynı klasördeki `akilli_kavsak` ve `akilli_kavsak_yeni` önceki sürümlerdir.

| Parça | Bağlantı |
| --- | --- |
| Kuzey kırmızı / sarı / yeşil | D2 / D3 / D4 |
| Güney kırmızı / sarı / yeşil | D5 / D6 / D7 |
| Doğu kırmızı / sarı / yeşil | D8 / D9 / D10 |
| Batı kırmızı / sarı / yeşil | D11 / D12 / D13 |
| Sensörler: kuzey, güney, doğu, batı | A0, A1, A2, A3 |

Kart: Arduino Uno. Dört IR sensör, dört trafik ışığı modülü.

Araç, analog okuma `SENSOR_ESIK` değerinin altındayken sayılır. Repodaki eşik `50` boşta deneme için bırakılmıştır ve çoğu sensörde araç üretmez. Serial monitörden boş ve araç varken okunan değerlere bakıp eşiği bu iki değerin arasına alın.

## Seri port sunucusu

`5_IoT_Dashboard/server.py`, karttan satır satır JSON okuyup paneli güncellemek için yazıldı. Paneldeki **Arduino** modu `ws://localhost:8765` adresine bağlanır.

```bash
cd 5_IoT_Dashboard
pip install -r requirements.txt
python server.py
```

Final kart kodu durumunu düz metin olarak yazar, JSON üretmez ve panelden gelen süre komutlarını okumaz. İncelemek için çalışan arayüz simülasyon modudur.
