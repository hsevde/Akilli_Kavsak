"""
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    AKILLI KAVŞAK - Python WebSocket Server
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Bu script Arduino'dan gelen Serial veriyi alır
ve WebSocket üzerinden web dashboard'a aktarır.

Kullanım:
1. pip install pyserial websockets
2. Arduino'yu bağla
3. python server.py
4. index.html dosyasını tarayıcıda aç
"""

import asyncio
import json
import serial
import serial.tools.list_ports
import websockets
from datetime import datetime
import threading
import sys

# ========== AYARLAR ==========
WEBSOCKET_PORT = 8765
BAUD_RATE = 9600

# ========== GLOBAL DEĞİŞKENLER ==========
arduino = None
clients = set()
son_veri = {
    "yon": None,
    "isik": "kirmizi",
    "araclar": [0, 0, 0, 0],
    "oncelik": None,
    "sure": 0,
    "sonraki": "KUZEY",
    "bagli": False,
    "zaman": ""
}

# ========== ARDUINO BAĞLANTISI ==========
def arduino_portlarini_listele():
    """Mevcut COM portlarını listele"""
    ports = serial.tools.list_ports.comports()
    arduino_ports = []
    
    for port in ports:
        arduino_ports.append({
            "port": port.device,
            "aciklama": port.description
        })
    
    return arduino_ports

def arduino_baglan(port=None):
    """Arduino'ya bağlan"""
    global arduino
    
    if port is None:
        # Otomatik port bul
        ports = arduino_portlarini_listele()
        if not ports:
            print("❌ Hiç COM portu bulunamadı!")
            return False
        
        # İlk portu dene
        port = ports[0]["port"]
        print(f"📍 Otomatik port seçildi: {port}")
    
    try:
        arduino = serial.Serial(port, BAUD_RATE, timeout=1)
        print(f"✅ Arduino bağlandı: {port}")
        son_veri["bagli"] = True
        return True
    except Exception as e:
        print(f"❌ Bağlantı hatası: {e}")
        return False

# ========== SERIAL OKUMA ==========
def serial_oku():
    """Arduino'dan veri oku"""
    global son_veri
    
    while True:
        if arduino and arduino.is_open:
            try:
                if arduino.in_waiting > 0:
                    line = arduino.readline().decode('utf-8').strip()
                    if line.startswith('{'):
                        try:
                            veri = json.loads(line)
                            son_veri.update(veri)
                            son_veri["bagli"] = True
                            son_veri["zaman"] = datetime.now().strftime("%H:%M:%S")
                            
                            # Tüm bağlı clientlara gönder
                            asyncio.run(broadcast(json.dumps(son_veri)))
                            
                            print(f"📨 {son_veri['zaman']} | Yön: {veri.get('yon', '-')} | Işık: {veri.get('isik', '-')} | Araçlar: {veri.get('araclar', [])}")
                        except json.JSONDecodeError:
                            pass
            except Exception as e:
                print(f"⚠️ Serial okuma hatası: {e}")
                son_veri["bagli"] = False
        
        asyncio.run(asyncio.sleep(0.1))

# ========== WEBSOCKET SERVER ==========
async def broadcast(message):
    """Tüm clientlara mesaj gönder"""
    if clients:
        await asyncio.gather(*[client.send(message) for client in clients])

async def handler(websocket, path=None):
    """WebSocket bağlantı yöneticisi"""
    clients.add(websocket)
    client_ip = websocket.remote_address[0]
    print(f"🔗 Yeni bağlantı: {client_ip} (Toplam: {len(clients)})")
    
    try:
        # Mevcut durumu gönder
        await websocket.send(json.dumps(son_veri))
        
        async for message in websocket:
            # Web'den gelen komutları Arduino'ya gönder
            try:
                komut = json.loads(message)
                if arduino and arduino.is_open:
                    if "yesilSure" in komut:
                        arduino.write(f"YESIL:{komut['yesilSure']}\n".encode())
                        print(f"⚙️ Yeşil süre ayarlandı: {komut['yesilSure']}s")
                    if "sariSure" in komut:
                        arduino.write(f"SARI:{komut['sariSure']}\n".encode())
                        print(f"⚙️ Sarı süre ayarlandı: {komut['sariSure']}s")
            except:
                pass
                
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        clients.remove(websocket)
        print(f"🔌 Bağlantı kapandı: {client_ip} (Kalan: {len(clients)})")

async def websocket_server():
    """WebSocket server başlat.

    Eğer varsayılan port kullanılamıyorsa, bir sonraki portlara
    otomatik olarak geçmeye çalışır (maksimum 10 deneme).
    """
    global WEBSOCKET_PORT
    port = WEBSOCKET_PORT
    max_tries = 10

    for attempt in range(max_tries):
        try:
            server = await websockets.serve(handler, "localhost", port)
            WEBSOCKET_PORT = port
            print(f"🌐 WebSocket Server başlatıldı: ws://localhost:{WEBSOCKET_PORT}")
            await server.wait_closed()
            return
        except OSError as e:
            print(f"⚠️ WebSocket bind hatası (port {port}): {e}")
            port += 1

    print("❌ WebSocket başlatılamadı: Tüm port denemeleri başarısız oldu.")

# ========== ANA FONKSİYON ==========
def main():
    print("\n" + "="*50)
    print("   AKILLI KAVŞAK - IoT Server")
    print("="*50 + "\n")
    
    # Portları listele
    print("📋 Mevcut COM Portları:")
    ports = arduino_portlarini_listele()
    
    if not ports:
        print("   Hiç port bulunamadı!")
        print("\n⚠️ Arduino bağlı değil, demo modda çalışacak")
    else:
        for i, p in enumerate(ports):
            print(f"   [{i}] {p['port']} - {p['aciklama']}")
    
    # Port seçimi
    port = None
    if ports:
        if len(ports) == 1:
            port = ports[0]["port"]
        else:
            try:
                secim = input("\n🔌 Port numarası seçin (Enter=otomatik): ").strip()
                if secim:
                    port = ports[int(secim)]["port"]
                else:
                    port = ports[0]["port"]
            except:
                port = ports[0]["port"]
        
        # Arduino'ya bağlan
        arduino_baglan(port)
    
    print("\n" + "-"*50)
    print("🚀 Server başlatılıyor...")
    print(f"📡 WebSocket: ws://localhost:{WEBSOCKET_PORT}")
    print("🌐 Dashboard: index.html dosyasını tarayıcıda açın")
    print("-"*50 + "\n")
    
    # Serial okuma thread'i
    if arduino:
        serial_thread = threading.Thread(target=serial_oku, daemon=True)
        serial_thread.start()
    
    # WebSocket server'ı başlat
    try:
        asyncio.run(websocket_server())
    except KeyboardInterrupt:
        print("\n\n👋 Server kapatılıyor...")
        if arduino:
            arduino.close()

if __name__ == "__main__":
    main()
