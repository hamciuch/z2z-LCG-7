#!/usr/bin/env python3
"""Panel Z2Z – pełny eksport encji z Home Assistant przez REST API (alternatywa
dla tools/ha_entities.jinja).

Użycie:
  HA_URL=http://homeassistant.local:8123 HA_TOKEN=<long-lived token> python3 tools/ha_export.py > encje.txt

Token: Profil użytkownika → Bezpieczeństwo → Tokeny o długim czasie życia.
Skrypt tylko czyta (/api/states). Wynik nie zawiera tokenu.
"""
import json
import os
import sys
import urllib.request

DOMAINS = {
    "person", "light", "switch", "input_boolean", "media_player", "climate", "water_heater",
    "input_number", "number", "sensor", "binary_sensor", "weather", "vacuum", "lock",
    "device_tracker", "plant", "fan",
}
SKIP_DC = {"timestamp", "date", "signal_strength", "voltage", "current", "frequency", "data_rate", "data_size"}


def main() -> int:
    url = os.environ.get("HA_URL", "http://homeassistant.local:8123").rstrip("/")
    token = os.environ.get("HA_TOKEN")
    if not token:
        print("Ustaw HA_TOKEN (long-lived access token).", file=sys.stderr)
        return 1
    req = urllib.request.Request(f"{url}/api/states", headers={"Authorization": f"Bearer {token}"})
    with urllib.request.urlopen(req, timeout=30) as r:
        states = json.load(r)

    rows = {}
    for s in states:
        eid = s["entity_id"]
        domain = eid.split(".", 1)[0]
        a = s.get("attributes", {})
        if domain not in DOMAINS:
            continue
        dc = a.get("device_class", "")
        # zmywarki mają czujniki timestamp (czas zakończenia) – tych nie pomijamy
        if dc in SKIP_DC and not any(k in eid for k in ("dishwasher", "zmywar")):
            continue
        line = f"- {eid} | {a.get('friendly_name', '')} | {s['state']}"
        if a.get("unit_of_measurement"):
            line += f" {a['unit_of_measurement']}"
        if dc:
            line += f" | dc={dc}"
        if domain == "climate":
            line += (f" | temp={a.get('current_temperature')} set={a.get('temperature')}"
                     f" action={a.get('hvac_action')}")
        rows.setdefault(domain, []).append(line)

    for domain in sorted(rows):
        print(f"\n## {domain} ({len(rows[domain])})")
        print("\n".join(sorted(rows[domain])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
