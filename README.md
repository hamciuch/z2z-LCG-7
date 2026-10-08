# Panel Z2Z – Home Assistant na Waveshare ESP32-S3-Touch-LCD-7

Firmware ESPHome + LVGL dla dotykowego panelu 7" (800×480).

- **Belka główna** (zawsze na wierzchu): zegar i data, domownicy w domu/poza domem,
  temperatura na balkonie, czy pada deszcz, status serwera, połączenie z HA/WiFi.
  Dotknięcie zegara → panel ustawień (wersja, IP, aktualizacja, restart).
- **Pulpity przewijane gestem** (animacja przesuwania), kropki nawigacji na dole:
  1. **Teraz** – tylko to, co *włączone* (światła, TV, gniazdka: ekspres, czajnik, drukarka 3D)
     oraz **alerty**: przekroczone progi, „Podlej roślinę”, „Wyjmij naczynia ze zmywarki”
     (znika po otwarciu drzwi), błąd odkurzacza, auto otwarte, niska bateria auta…
     Gdy wszystko zgaśnie – kafelki znikają. Dotknięcie kafelka = wyłącz.
  2. **Ogrzewanie** – `climate.*` albo setpoint `input_number/number` + czujnik, przyciski ±
  3. **Samochód** – karty z wartościami/stanami + alerty
  4. **Odkurzacz** – stan, bateria, Start / Pauza / Do stacji
  5. **Zmywarka** – stan, pozostały czas, drzwi
  6. **Rośliny** – wilgotność gleby z paskiem i progiem
- **Wygaszanie ekranu** i **powrót na pulpit główny** po bezczynności; nowy alert budzi ekran.
- **OTA z GitHub**: GitHub Actions buduje firmware i publikuje go na GitHub Pages
  (`manifest.json`). Panel sam sprawdza aktualizacje (encja *Firmware* w HA).
- **Instalator w przeglądarce** (ESP Web Tools + Improv WiFi) – pierwsze wgranie bez instalowania niczego.
- **Strona konfiguracyjna** panelu `http://<ip-panelu>/` (progi alertów, wygaszanie, auto-aktualizacja, restart, upload .bin).

## Struktura

| Ścieżka | Co to |
|---|---|
| `config/home.yaml` | **Twoja konfiguracja** – encje, kafelki, progi, które pulpity pokazać |
| `z2z-lcd7.yaml` | Główny plik ESPHome (nazwa urządzenia, URL manifestu OTA) |
| `tiles/*.yaml` | Typy kafelków (opis zmiennych w nagłówku każdego pliku) |
| `core/*.yaml` | Belka, pulpity, motyw, ustawienia, OTA, nakładki |
| `hardware/…yaml` | Piny wyświetlacza, dotyk GT911, ekspander CH422G |
| `tools/ha_entities.jinja` | Zapytanie do HA wyciągające potrzebne encje |
| `tools/ha_export.py` | Alternatywa: pełny eksport przez REST API |
| `.github/workflows/build.yml` | Build → GitHub Pages (OTA + instalator) |

## Pierwsze uruchomienie

### 1. Sekrety w GitHub (raz)
`Settings → Secrets and variables → Actions → New repository secret`:

| Nazwa | Wartość |
|---|---|
| `API_ENCRYPTION_KEY` | wynik `openssl rand -base64 32` (zachowaj – poda go HA przy dodawaniu) |
| `WEB_PASSWORD` | hasło do strony WWW panelu (login `admin`) |
| `AP_PASSWORD` | hasło AP do konfiguracji WiFi, min. 8 znaków |

Bez sekretów workflow tylko sprawdza, czy firmware się kompiluje, i niczego nie publikuje.

### 2. GitHub Pages (raz)
`Settings → Pages → Build and deployment → Source: GitHub Actions`.
Po kolejnym pushu instalator i manifest będą pod
`https://hamciuch.github.io/z2z-lcg-7/` (repo musi być publiczne albo plan z Pages dla prywatnych).

### 3. Encje z Home Assistant
Otwórz w HA **Narzędzia deweloperskie → Szablon**, wklej całą zawartość
[`tools/ha_entities.jinja`](tools/ha_entities.jinja) i skopiuj wynik z prawej strony.
Na jego podstawie uzupełnij `config/home.yaml` (albo wklej wynik do rozmowy z Claude).

### 4. Wgranie
Push na `main` → zakładka **Actions** zbuduje firmware (~5–10 min za pierwszym razem).
Potem otwórz `https://hamciuch.github.io/z2z-lcg-7/` w Chrome/Edge, podłącz panel USB-C
(gniazdo **USB**) i kliknij **Zainstaluj na panelu**. Instalator od razu zapyta o WiFi.

Bez USB/Chrome: panel bez WiFi wystawia sieć **„Panel Z2Z Setup”** → `http://192.168.4.1`.

### 5. Home Assistant
Panel pojawi się w *Ustawienia → Urządzenia i usługi → ESPHome* – podaj `API_ENCRYPTION_KEY`.
**Ważne:** w opcjach (⚙️) integracji tego urządzenia zaznacz
*„Allow the device to perform Home Assistant actions”* – inaczej dotyk nie wyłączy świateł
ani nie zmieni temperatury.

## Aktualizacje OTA

Każdy push na `main` = nowa wersja `RRRR.MM.DD.<nr builda>`; tag `v1.2.3` = wersja `1.2.3`
plus GitHub Release z plikami `.bin`. Panel co 6 h sprawdza `firmware/manifest.json`. Instalacja:

- HA → encja **Firmware** (`update.*`) → *Zainstaluj*,
- na panelu: dotknij zegara → *Zainstaluj aktualizację*,
- automatycznie: przełącznik **Automatyczne aktualizacje z GitHub** (HA lub strona WWW panelu).

Awaryjnie: plik `z2z-panel.ota.bin` z artefaktów builda wgrasz na `http://<ip-panelu>/`
albo z dashboardu ESPHome w HA (OTA szyfrowane kluczem API).

## Dodawanie kafelków (`config/home.yaml`)

```yaml
packages:
  l_biurko: !include { file: ../tiles/active.yaml, vars: { uid: l_biurko, name: "Biurko", icon: $ic_lightbulb, entity: light.biurko } }
```

| Kafelek | Gdzie | Kiedy widoczny |
|---|---|---|
| `active.yaml` | Teraz | encja włączona (nie `off/standby/unavailable`) |
| `active_power.yaml` | Teraz | moc > `threshold` W |
| `alert_threshold.yaml` | Teraz | wartość `below`/`above` progu (próg zmienisz w HA/WWW) |
| `alert_state.yaml` | Teraz | stan na liście `states` |
| `plant.yaml` | Rośliny + alert | wilgotność < progu |
| `dishwasher.yaml` | Zmywarka + alert | koniec programu, do otwarcia drzwi |
| `vacuum.yaml` | Odkurzacz + alert | błąd odkurzacza |
| `climate.yaml` / `setpoint.yaml` | Ogrzewanie | zawsze |
| `value_card.yaml` / `text_card.yaml` | dowolny pulpit (`page:`) | zawsze |
| `person.yaml` | belka (`slot` 0–3) | zawsze |

`uid` musi być unikalny (litery, cyfry, `_`). Ikony: `$ic_…` z `core/theme.yaml`.
Pulpit niepotrzebny? `hide_page_car: "true"`.

## Budowanie lokalnie (opcjonalnie)

```bash
pip install esphome==2026.9.1
cp secrets.example.yaml secrets.yaml   # uzupełnij
esphome run z2z-lcd7.yaml              # USB lub OTA
```

## Znane kwestie / do sprawdzenia na sprzęcie

- Dotyk odwrócony/odbity → `touch_swap_xy / touch_mirror_x / touch_mirror_y` w `config/home.yaml`.
- Podświetlenie na tej płytce jest tylko włącz/wyłącz (brak PWM) – stąd brak regulacji jasności.
- Czcionki: Nunito (SIL OFL 1.1), Material Design Icons (Apache 2.0) – w katalogu `fonts/`.
