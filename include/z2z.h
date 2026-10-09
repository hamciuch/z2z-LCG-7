// Pomocnicze funkcje dla panelu Z2Z (ESP32-S3-Touch-LCD-7)
#pragma once
#include <string>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include "lvgl.h"

namespace z2z {

// Kolory używane z lambd (muszą odpowiadać core/theme.yaml)
static const uint32_t C_ACCENT = 0x4C9EFF;
static const uint32_t C_DOT_OFF = 0x2A3142;
static const uint32_t C_OK = 0x34C77B;
static const uint32_t C_WARN = 0xFF9F43;
static const uint32_t C_ERR = 0xFF5C5C;
static const uint32_t C_MUTED = 0x8691A6;
static const uint32_t C_TEXT = 0xE8ECF2;
static const uint32_t C_HEAT = 0xFF7A45;
static const uint32_t C_COOL = 0x4C9EFF;

inline std::string lower(const std::string &s) {
  std::string r = s;
  for (auto &c : r) c = (char) tolower((unsigned char) c);
  return r;
}

// Czy encja jest "aktywna" (włączona). Działa dla light/switch/input_boolean/fan/media_player.
inline bool is_active(const std::string &state) {
  std::string s = lower(state);
  if (s.empty()) return false;
  static const char *const OFF[] = {"off", "unavailable", "unknown", "standby", "none", "closed", "0"};
  for (auto *o : OFF)
    if (s == o) return false;
  return true;
}

// Czy stan pasuje do listy wartości oddzielonych przecinkami ("finished,end,done")
inline bool in_list(const std::string &state, const std::string &csv) {
  std::string s = lower(state);
  size_t start = 0;
  std::string l = lower(csv);
  while (start <= l.size()) {
    size_t end = l.find(',', start);
    if (end == std::string::npos) end = l.size();
    std::string item = l.substr(start, end - start);
    // trim
    while (!item.empty() && item.front() == ' ') item.erase(item.begin());
    while (!item.empty() && item.back() == ' ') item.pop_back();
    if (!item.empty() && item == s) return true;
    start = end + 1;
  }
  return false;
}

// Tłumaczenie typowych stanów HA na polski
inline std::string pl(const std::string &state) {
  std::string s = lower(state);
  struct M { const char *en; const char *pl; };
  static const M MAP[] = {
      {"on", "Włączone"}, {"off", "Wyłączone"}, {"unavailable", "Niedostępne"}, {"unknown", "Nieznany"},
      {"home", "W domu"}, {"not_home", "Poza domem"}, {"away", "Poza domem"},
      {"cleaning", "Sprząta"}, {"docked", "W stacji"}, {"returning", "Wraca do stacji"},
      {"idle", "Bezczynny"}, {"paused", "Wstrzymany"}, {"error", "Błąd"}, {"charging", "Ładuje"},
      {"locked", "Zamknięty"}, {"unlocked", "Otwarty"}, {"locking", "Zamykanie"}, {"unlocking", "Otwieranie"},
      {"open", "Otwarte"}, {"closed", "Zamknięte"}, {"opening", "Otwieranie"}, {"closing", "Zamykanie"},
      {"run", "Pracuje"}, {"running", "Pracuje"}, {"finished", "Zakończono"}, {"end", "Zakończono"},
      {"ready", "Gotowa"}, {"inactive", "Nieaktywna"}, {"delayedstart", "Opóźniony start"},
      {"delayed_start", "Opóźniony start"}, {"pause", "Pauza"}, {"aborting", "Przerywanie"},
      {"actionrequired", "Wymaga akcji"}, {"heat", "Grzanie"}, {"heating", "Grzeje"}, {"cool", "Chłodzenie"},
      {"cooling", "Chłodzi"}, {"auto", "Auto"}, {"playing", "Odtwarza"}, {"buffering", "Buforuje"},
      {"standby", "Czuwanie"}, {"rainy", "Deszcz"}, {"pouring", "Ulewa"}, {"sunny", "Słonecznie"},
      {"sweeping", "Odkurza"}, {"mopping", "Mopuje"}, {"sweeping_and_mopping", "Odkurza i mopuje"},
      {"mopping_after_sweeping", "Mop po odkurzaniu"}, {"charging_completed", "Naładowany"},
      {"washing", "Myje mop"}, {"drying", "Suszy mop"}, {"building", "Mapuje"}, {"sleeping", "Uśpiony"},
      {"returning_to_wash", "Wraca umyć mop"}, {"remote_control", "Sterowanie ręczne"},
      {"end_of_cycle", "Zakończono"}, {"ready_to_start", "Gotowa do startu"},
      {"program_not_selected", "Brak programu"}, {"unplugged", "Niepodłączony"}, {"charging_finished", "Naładowany"},
      {"cloudy", "Pochmurno"}, {"partlycloudy", "Częściowe zachm."}, {"clear-night", "Pogodna noc"},
      {"fog", "Mgła"}, {"snowy", "Śnieg"}, {"lightning-rainy", "Burza"}, {"windy", "Wietrznie"},
  };
  for (auto &m : MAP)
    if (s == m.en) return m.pl;
  return state;
}

// Format liczby bez śmieci przy NaN
inline std::string fmt(float v, int decimals, const char *unit) {
  if (std::isnan(v)) return std::string("--") + unit;
  char buf[32];
  snprintf(buf, sizeof(buf), "%.*f%s", decimals, v, unit);
  return buf;
}

inline void show(lv_obj_t *o, bool visible) {
  if (o == nullptr) return;
  if (visible)
    lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

inline void set_bg(lv_obj_t *o, uint32_t color) { lv_obj_set_style_bg_color(o, lv_color_hex(color), 0); }
inline void set_text_color(lv_obj_t *o, uint32_t color) { lv_obj_set_style_text_color(o, lv_color_hex(color), 0); }

// Zaznaczenie przycisku opcji (wybrana = bursztynowa ramka)
inline void mark(lv_obj_t *o, bool on) {
  lv_obj_set_style_bg_color(o, lv_color_hex(on ? 0x1E3A66 : 0x202736), 0);
  lv_obj_set_style_border_color(o, lv_color_hex(0x4C9EFF), 0);
  lv_obj_set_style_border_width(o, on ? 3 : 0, 0);
}

// Wiersz opcji: zaznacza przycisk, którego opcja == state; przyciemnia wiersz, gdy select niedostępny
inline void mark_row(lv_obj_t *row, lv_obj_t *const *btns, const char *const *opts, int n, const std::string &state) {
  bool avail = !(state == "unavailable" || state == "unknown" || state.empty());
  lv_obj_set_style_opa(row, avail ? LV_OPA_COVER : LV_OPA_40, 0);
  for (int i = 0; i < n; i++) mark(btns[i], avail && state == opts[i]);
}

inline void enable(lv_obj_t *o, bool on) {
  if (on) lv_obj_remove_state(o, LV_STATE_DISABLED);
  else lv_obj_add_state(o, LV_STATE_DISABLED);
  lv_obj_set_style_opa(o, on ? LV_OPA_COVER : LV_OPA_40, 0);
}

// Liczy widoczne kafelki na stronie (pomija obiekty wskazane jako stałe)
inline int visible_children(lv_obj_t *parent, lv_obj_t *skip1, lv_obj_t *skip2) {
  int n = 0;
  uint32_t cnt = lv_obj_get_child_count(parent);
  for (uint32_t i = 0; i < cnt; i++) {
    lv_obj_t *c = lv_obj_get_child(parent, i);
    if (c == skip1 || c == skip2) continue;
    if (!lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) n++;
  }
  return n;
}


// Ustawia widoczność; zwraca true, jeśli obiekt właśnie się POJAWIŁ
inline bool set_visible(lv_obj_t *o, bool visible) {
  bool was = !lv_obj_has_flag(o, LV_OBJ_FLAG_HIDDEN);
  show(o, visible);
  return visible && !was;
}

// days_from_civil (H. Hinnant) – epoch UTC bez zależności od strefy
inline int64_t days_from_civil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned) (y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int64_t) doe - 719468;
}

// "2026-10-08T16:45:00+00:00" / "...Z" -> epoch UTC; -1 gdy to nie jest znacznik czasu
inline int64_t parse_iso(const std::string &s) {
  int Y, M, D, h, m, sec = 0;
  if (s.size() < 16 || sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &Y, &M, &D, &h, &m, &sec) < 5) return -1;
  int64_t t = days_from_civil(Y, M, D) * 86400 + h * 3600 + m * 60 + sec;
  // przesunięcie strefy: szukamy +HH:MM / -HH:MM po części czasu
  size_t p = s.find_first_of("+-", 19);
  if (p != std::string::npos && p + 5 < s.size() + 1) {
    int oh = 0, om = 0;
    if (sscanf(s.c_str() + p + 1, "%d:%d", &oh, &om) >= 1) {
      int off = oh * 3600 + om * 60;
      t += (s[p] == '+') ? -off : off;
    }
  }
  return t;
}

// Pozostały czas jako tekst. Akceptuje: znacznik czasu ISO, liczbę minut, "H:MM:SS".
inline std::string remaining(const std::string &raw, int64_t now_utc) {
  if (raw.empty() || raw == "unknown" || raw == "unavailable") return "";
  int mins = -1;
  int64_t ts = parse_iso(raw);
  if (ts > 0 && now_utc > 0) {
    mins = (int) ((ts - now_utc + 59) / 60);
  } else {
    int h, m, sec;
    if (sscanf(raw.c_str(), "%d:%d:%d", &h, &m, &sec) == 3) mins = h * 60 + m;
    else {
      char *end = nullptr;
      float v = strtof(raw.c_str(), &end);
      if (end != raw.c_str()) mins = (int) v;
    }
  }
  if (mins < 0) return "";
  char b[40];
  if (mins >= 60) snprintf(b, sizeof(b), "%d h %02d min", mins / 60, mins % 60);
  else snprintf(b, sizeof(b), "%d min", mins);
  return b;
}


// ── Strefy (zone): nazwa ikony MDI (bez "mdi:") → znak z fontu i32 ─────────────
// Lista musi odpowiadać glifom zone_glyphs w core/theme.yaml (generuje tools/).
struct NamedGlyph { const char *name; const char *glyph; };
static const NamedGlyph ZONE_ICONS[] = {
    {"home", "\xF3\xB0\x8B\x9C"},
    {"home-heart", "\xF3\xB0\xA0\xA7"},
    {"home-variant", "\xF3\xB0\x8B\x9E"},
    {"home-city", "\xF3\xB0\xB4\x95"},
    {"home-group", "\xF3\xB0\xB7\x94"},
    {"briefcase", "\xF3\xB0\x83\x96"},
    {"office-building", "\xF3\xB0\xA6\x91"},
    {"domain", "\xF3\xB0\x87\x97"},
    {"school", "\xF3\xB0\x91\xB4"},
    {"school-outline", "\xF3\xB1\x86\x80"},
    {"cart", "\xF3\xB0\x84\x90"},
    {"shopping", "\xF3\xB0\x92\x9A"},
    {"store", "\xF3\xB0\x93\x9C"},
    {"dumbbell", "\xF3\xB0\x87\xA6"},
    {"weight-lifter", "\xF3\xB1\x85\x9D"},
    {"hospital-building", "\xF3\xB0\x8B\xA1"},
    {"hospital-box", "\xF3\xB0\x8B\xA0"},
    {"medical-bag", "\xF3\xB0\x9B\xAF"},
    {"stethoscope", "\xF3\xB0\x93\x99"},
    {"airplane", "\xF3\xB0\x80\x9D"},
    {"airport", "\xF3\xB0\xA1\x8B"},
    {"beach", "\xF3\xB0\x82\x92"},
    {"palm-tree", "\xF3\xB1\x81\x95"},
    {"island", "\xF3\xB1\x81\x8F"},
    {"pine-tree", "\xF3\xB0\x90\x85"},
    {"tree", "\xF3\xB0\x94\xB1"},
    {"forest", "\xF3\xB1\xA2\x97"},
    {"church", "\xF3\xB0\x85\x84"},
    {"silverware-fork-knife", "\xF3\xB0\xA9\xB0"},
    {"food", "\xF3\xB0\x89\x9A"},
    {"coffee", "\xF3\xB0\x85\xB6"},
    {"gas-station", "\xF3\xB0\x8A\x98"},
    {"pool", "\xF3\xB0\x98\x86"},
    {"swim", "\xF3\xB0\x93\xA3"},
    {"soccer", "\xF3\xB0\x92\xB8"},
    {"basketball", "\xF3\xB0\xA0\x86"},
    {"tennis", "\xF3\xB0\xB6\xA0"},
    {"run", "\xF3\xB0\x9C\x8E"},
    {"bike", "\xF3\xB0\x82\xA3"},
    {"account-group", "\xF3\xB0\xA1\x89"},
    {"human-male-female-child", "\xF3\xB1\xA0\xA3"},
    {"baby-carriage", "\xF3\xB0\x9A\x8F"},
    {"teddy-bear", "\xF3\xB1\xA3\xBB"},
    {"car", "\xF3\xB0\x84\x8B"},
    {"train", "\xF3\xB0\x94\xAC"},
    {"bus", "\xF3\xB0\x83\xA7"},
    {"bank", "\xF3\xB0\x81\xB0"},
    {"theater", "\xF3\xB0\x94\x8D"},
    {"music", "\xF3\xB0\x9D\x9A"},
    {"book-open-variant", "\xF3\xB1\x93\xB7"},
    {"library", "\xF3\xB0\x8C\xB1"},
    {"factory", "\xF3\xB0\x88\x8F"},
    {"warehouse", "\xF3\xB0\xBE\x81"},
    {"city", "\xF3\xB0\x85\x86"},
    {"castle", "\xF3\xB0\x84\x9A"},
    {"heart", "\xF3\xB0\x8B\x91"},
    {"star", "\xF3\xB0\x93\x8E"},
    {"map-marker", "\xF3\xB0\x8D\x8E"},
    {"parking", "\xF3\xB0\x8F\xA3"},
    {"dog", "\xF3\xB0\xA9\x83"},
    {"paw", "\xF3\xB0\x8F\xA9"},
    {"campfire", "\xF3\xB0\xBB\x9D"},
    {"tent", "\xF3\xB0\x94\x88"},
    {"ski", "\xF3\xB1\x8C\x84"},
    {"fish", "\xF3\xB0\x88\xBA"},
    {"golf", "\xF3\xB0\xA0\xA3"},
    {"baby-face-outline", "\xF3\xB0\xB9\xBD"},
    {"account-heart", "\xF3\xB0\xA2\x99"},
    {"human-cane", "\xF3\xB1\x96\x81"},
    {"account-tie", "\xF3\xB0\xB3\xA3"},
    {"account-tie-woman", "\xF3\xB1\xAA\x8C"},
    {"account-school", "\xF3\xB1\xA8\xA0"},
    {"greenhouse", "\xF3\xB0\x80\xAD"}
};
inline const char *zone_glyph(const std::string &name) {
  for (auto &z : ZONE_ICONS)
    if (name == z.name) return z.glyph;
  return "\xF3\xB0\x8D\x8E";  // map-marker
}

// Wartość dla klucza z "a=x;b=y;" (np. stan sensora stref) – "" gdy brak
inline std::string kv(const std::string &s, const std::string &key) {
  size_t p = 0;
  while (p < s.size()) {
    size_t e = s.find(';', p);
    if (e == std::string::npos) e = s.size();
    size_t eq = s.find('=', p);
    if (eq != std::string::npos && eq < e && s.compare(p, eq - p, key) == 0 && eq - p == key.size())
      return s.substr(eq + 1, e - eq - 1);
    p = e + 1;
  }
  return "";
}

// ── Pogoda: stan HA (condition) → ikona MDI i polski opis ────────────────────
struct WeatherCond { const char *cond; const char *glyph; const char *text; };
static const WeatherCond WEATHER[] = {
    {"clear-night", "\xF3\xB0\x96\x94", "pogodna noc"},
    {"cloudy", "\xF3\xB0\x96\x90", "pochmurno"},
    {"exceptional", "\xF3\xB0\x97\x96", "ostrzeżenie"},
    {"fog", "\xF3\xB0\x96\x91", "mgła"},
    {"hail", "\xF3\xB0\x96\x92", "grad"},
    {"lightning", "\xF3\xB0\x96\x93", "burza"},
    {"lightning-rainy", "\xF3\xB0\x99\xBE", "burza z deszczem"},
    {"partlycloudy", "\xF3\xB0\x96\x95", "częściowe zachm."},
    {"pouring", "\xF3\xB0\x96\x96", "ulewa"},
    {"rainy", "\xF3\xB0\x96\x97", "deszcz"},
    {"snowy", "\xF3\xB0\x96\x98", "śnieg"},
    {"snowy-rainy", "\xF3\xB0\x99\xBF", "deszcz ze śniegiem"},
    {"sunny", "\xF3\xB0\x96\x99", "słonecznie"},
    {"windy", "\xF3\xB0\x96\x9D", "wietrznie"},
    {"windy-variant", "\xF3\xB0\x96\x9E", "wietrznie"}
};
inline const char *weather_glyph(const std::string &c) {
  for (auto &w : WEATHER)
    if (c == w.cond) return w.glyph;
  return "\xF3\xB0\x96\x90";  // weather-cloudy
}
inline const char *weather_text(const std::string &c) {
  for (auto &w : WEATHER)
    if (c == w.cond) return w.text;
  return "";
}

// Dzień tygodnia (0 = pn) z daty "RRRR-MM-DD…" (algorytm Sakamoto)
inline int weekday_mon0(const char *iso) {
  int y = atoi(iso), m = atoi(iso + 5), d = atoi(iso + 8);
  if (y < 2000 || m < 1 || m > 12) return -1;
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) y -= 1;
  int w = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;  // 0 = niedziela
  return (w + 6) % 7;
}

// "1 gracz", "3 graczy"… – polska odmiana
inline std::string players(int n) {
  const char *w = "graczy";
  if (n == 1) w = "gracz";
  else if (n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % 100 > 14)) w = "gracze";
  char b[32];
  snprintf(b, sizeof b, "%d %s", n, w);
  return b;
}


// ISO 8601 z HA ("2026-10-09T22:00:00+00:00") → czas uniksowy (0 = błąd / data bez godziny)
inline uint32_t iso_ts(const char *iso) {
  if (!iso || strlen(iso) < 16) return 0;
  int y = atoi(iso), m = atoi(iso + 5), d = atoi(iso + 8), hh = atoi(iso + 11), mi = atoi(iso + 14), off = 0;
  if (y < 2000 || m < 1 || m > 12) return 0;
  if (strlen(iso) >= 25 && (iso[19] == '+' || iso[19] == '-'))
    off = (iso[19] == '-' ? -1 : 1) * (atoi(iso + 20) * 60 + atoi(iso + 23));
  // dni od 1970-01-01 (Howard Hinnant, days_from_civil)
  int yy = y - (m <= 2);
  int era = (yy >= 0 ? yy : yy - 399) / 400;
  unsigned yoe = (unsigned) (yy - era * 400);
  unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  long days = (long) era * 146097 + (long) doe - 719468;
  return (uint32_t) ((int64_t) days * 86400 + hh * 3600 + mi * 60 - off * 60);
}
// … → czas lokalny (strefa z komponentu time)
inline bool iso_local(const char *iso, struct tm *out) {
  time_t t = iso_ts(iso);
  if (!t) return false;
  return localtime_r(&t, out) != nullptr;
}
// "HH:MM" czasu lokalnego
inline std::string hhmm(uint32_t ts) {
  if (!ts) return "";
  time_t t = ts; struct tm l{};
  localtime_r(&t, &l);
  char b[8]; snprintf(b, sizeof b, "%02d:%02d", l.tm_hour, l.tm_min);
  return b;
}
// numer dnia (lokalnie) – do porównań "dziś / jutro"
inline int local_yday(uint32_t ts) {
  time_t t = ts; struct tm l{};
  localtime_r(&t, &l);
  return l.tm_year * 400 + l.tm_yday;
}


// Bieżący czas uniksowy (ustawia go komponent time); 0, dopóki zegar nie jest zsynchronizowany
inline uint32_t now_ts() {
  time_t t = ::time(nullptr);
  return t > 1700000000 ? (uint32_t) t : 0;
}


// Bateryjka 4-stopniowa w rogu kafelka: >75% 4 kreski, >50% 3 (szare), >25% 2 (żółta), niżej 1 (czerwona).
// Glify: battery / battery-80 / battery-50 / battery-20 (muszą być w foncie i24).
static const uint32_t C_YELLOW = 0xFACC15;
inline void battery(lv_obj_t *lbl, float pct) {
  if (isnan(pct)) { lv_obj_add_flag(lbl, LV_OBJ_FLAG_HIDDEN); return; }
  lv_obj_remove_flag(lbl, LV_OBJ_FLAG_HIDDEN);
  const char *g; uint32_t c;
  if (pct > 75) { g = "\U000F0079"; c = 0x4A5468; }
  else if (pct > 50) { g = "\U000F0081"; c = 0x4A5468; }
  else if (pct > 25) { g = "\U000F007E"; c = C_YELLOW; }
  else { g = "\U000F007B"; c = C_ERR; }
  lv_label_set_text(lbl, g);
  set_text_color(lbl, c);
}

// "12 min", "1 h 05 min" – czas od podanej chwili (czas uniksowy)
inline std::string since(uint32_t start) {
  uint32_t now = now_ts();
  if (!start || !now || now < start) return "";
  uint32_t m = (now - start) / 60;
  char b[24];
  if (m >= 60) snprintf(b, sizeof b, "%u h %02u min", m / 60, m % 60);
  else snprintf(b, sizeof b, "%u min", m);
  return b;
}

}  // namespace z2z
