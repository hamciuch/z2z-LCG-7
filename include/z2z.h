// Pomocnicze funkcje dla panelu Z2Z (ESP32-S3-Touch-LCD-7)
#pragma once
#include <string>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "lvgl.h"

namespace z2z {

// Kolory używane z lambd (muszą odpowiadać core/theme.yaml)
static const uint32_t C_ACCENT = 0xFFB300;
static const uint32_t C_DOT_OFF = 0x3A4150;
static const uint32_t C_OK = 0x43A047;
static const uint32_t C_WARN = 0xFF8F00;
static const uint32_t C_ERR = 0xE53935;
static const uint32_t C_MUTED = 0x8A94A6;
static const uint32_t C_TEXT = 0xECEFF1;
static const uint32_t C_HEAT = 0xFF7043;
static const uint32_t C_COOL = 0x42A5F5;

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
  lv_obj_set_style_bg_color(o, lv_color_hex(on ? 0x5A4410 : 0x262B35), 0);
  lv_obj_set_style_border_color(o, lv_color_hex(0xFFB300), 0);
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

}  // namespace z2z
