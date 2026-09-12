#include "CalendarService.h"

#include <ArduinoJson.h>
#include <cstring>
#include <Logging.h>
#include <WiFi.h>

#include "../../network/HttpDownloader.h"
#include "WifiCredentialStore.h"
#include <HalStorage.h>

namespace {
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 6000;
constexpr uint32_t WIFI_POLL_INTERVAL_MS = 100;
}

CalendarService CALENDAR_SERVICE;

void CalendarService::begin() {
  if (loadCache()) {
    LOG_DBG("CAL", "Loaded calendar from SD cache");
  } else {
    setSafeDefault();
    LOG_DBG("CAL", "No valid calendar cache; using safe default");
  }

  // WifiCredentialStore is normally loaded by WifiSelectionActivity. Home can
  // refresh the calendar before that activity has ever opened, so explicitly
  // load the persistent WiFi credentials here as well.
  if (WIFI_STORE.loadFromFile()) {
    LOG_DBG("CAL", "Loaded saved WiFi credentials for calendar refresh");
  } else {
    LOG_DBG("CAL", "No saved WiFi credential file for calendar refresh");
  }
}

bool CalendarService::parseAndValidate(const std::string& json, CalendarData& out) const {
  if (json.empty() || json.size() > MAX_JSON_BYTES) return false;

  StaticJsonDocument<2048> doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    LOG_ERR("CAL", "Calendar JSON parse failed: %s", err.c_str());
    return false;
  }

  const JsonObjectConst calendar = doc["calendar"].as<JsonObjectConst>();
  if (calendar.isNull()) return false;

  const int day = calendar["day"] | 0;
  const int month = calendar["month"] | 0;
  const int weekday = calendar["weekday"] | 0;
  const int lunarDay = calendar["lunar_day"] | 0;
  const int lunarMonth = calendar["lunar_month"] | 0;
  const char* date = doc["date"] | "";

  if (day < 1 || day > 31 || month < 1 || month > 12 ||
      weekday < 1 || weekday > 7 || lunarDay < 1 || lunarDay > 30 ||
      lunarMonth < 1 || lunarMonth > 12) {
    return false;
  }
  if (std::strlen(date) != 10 || date[4] != '-' || date[7] != '-') return false;

  out.valid = true;
  out.date = date;
  out.day = day;
  out.month = month;
  out.weekday = weekday;
  out.lunarDay = lunarDay;
  out.lunarMonth = lunarMonth;

  const JsonObjectConst owner = doc["owner"].as<JsonObjectConst>();
  out.ownerName = owner["name"] | "";
  out.ownerPhone = owner["phone"] | "";
  out.ownerEmail = owner["email"] | "";
  return true;
}

bool CalendarService::loadCache() {
  const String cached = Storage.readFile(CACHE_PATH);
  if (cached.isEmpty()) return false;
  CalendarData parsed;
  if (!parseAndValidate(cached.c_str(), parsed)) {
    LOG_ERR("CAL", "Ignoring invalid calendar cache");
    return false;
  }
  current = std::move(parsed);
  return true;
}

bool CalendarService::saveCache(const std::string& json) const {
  if (!Storage.ensureDirectoryExists("/.crosspoint")) return false;
  return Storage.writeFile(CACHE_PATH, String(json.c_str()));
}

void CalendarService::setSafeDefault() {
  current = CalendarData{};
  current.valid = true;
  current.date = "1970-01-01";
  current.day = 1;
  current.month = 1;
  current.weekday = 4;
  current.lunarDay = 1;
  current.lunarMonth = 1;
}

bool CalendarService::refresh() {
  std::string json;

  // Home refresh can happen before WifiSelectionActivity has loaded the store,
  // and credentials can have changed since boot. Reload the persistent store
  // immediately before using it.
  WIFI_STORE.loadFromFile();

  // Calendar refresh is an explicit, user-triggered network session.
  // Reuse an already-connected WiFi session. Otherwise try the last successful
  // saved network first, then every other saved credential until one connects.
  bool ownsWifiSession = false;
  std::string connectedCredentialSsid;

  if (WiFi.status() != WL_CONNECTED) {
    const std::string lastSsid = WIFI_STORE.getLastConnectedSsid();
    const size_t credentialCount = WIFI_STORE.getCredentialCount();

    if (!lastSsid.empty()) {
      const auto lastCredential = WIFI_STORE.findCredential(lastSsid);
      if (lastCredential) {
        const auto& cred = *lastCredential;
        ownsWifiSession = true;
        WiFi.persistent(false);
        WiFi.disconnect(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(cred.ssid.c_str(), cred.password.c_str());
        LOG_DBG("CAL", "Trying saved WiFi: %s", cred.ssid.c_str());

        const unsigned long started = millis();
        while (WiFi.status() != WL_CONNECTED &&
               millis() - started < WIFI_CONNECT_TIMEOUT_MS) {
          delay(WIFI_POLL_INTERVAL_MS);
        }

        if (WiFi.status() == WL_CONNECTED) {
          connectedCredentialSsid = cred.ssid;
        }
      }
    }

    // If the last-connected entry is missing/stale, try every saved credential.
    if (WiFi.status() != WL_CONNECTED) {
      for (size_t i = 0; i < credentialCount; ++i) {
        const auto credential = WIFI_STORE.getCredentialAt(i);
        if (!credential) continue;
        if (credential->ssid == lastSsid) continue;

        ownsWifiSession = true;
        WiFi.persistent(false);
        WiFi.disconnect(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(credential->ssid.c_str(), credential->password.c_str());
        LOG_DBG("CAL", "Trying saved WiFi: %s", credential->ssid.c_str());

        const unsigned long started = millis();
        while (WiFi.status() != WL_CONNECTED &&
               millis() - started < WIFI_CONNECT_TIMEOUT_MS) {
          delay(WIFI_POLL_INTERVAL_MS);
        }

        if (WiFi.status() == WL_CONNECTED) {
          connectedCredentialSsid = credential->ssid;
          break;
        }
      }
    }

    if (credentialCount == 0) {
      LOG_DBG("CAL", "No saved WiFi credentials for calendar refresh");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    LOG_DBG("CAL", "Calendar refresh skipped: WiFi unavailable");
    if (ownsWifiSession) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      LOG_DBG("CAL", "WiFi disabled after failed calendar connection");
    }
    return false;
  }

  const bool fetched = HttpDownloader::fetchUrl(URL, json, FETCH_TIMEOUT_MS);
  if (!fetched) {
    LOG_ERR("CAL", "Calendar HTTPS fetch failed; keeping last valid data");
    if (ownsWifiSession) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      LOG_DBG("CAL", "WiFi disabled after failed calendar fetch");
    }
    return false;
  }

  CalendarData parsed;
  if (!parseAndValidate(json, parsed)) {
    LOG_ERR("CAL", "Calendar server returned invalid data");
    if (ownsWifiSession) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      LOG_DBG("CAL", "WiFi disabled after invalid calendar response");
    }
    return false;
  }

  if (!saveCache(json)) {
    LOG_ERR("CAL", "Calendar cache write failed; using fetched data in RAM");
  }

  current = std::move(parsed);
  LOG_INF("CAL", "Calendar refreshed: %s", current.date.c_str());

  if (!connectedCredentialSsid.empty() &&
      connectedCredentialSsid != WIFI_STORE.getLastConnectedSsid()) {
    WIFI_STORE.setLastConnectedSsid(connectedCredentialSsid);
  }

  if (ownsWifiSession) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    LOG_DBG("CAL", "WiFi disabled after successful calendar refresh");
  }

  return true;
}

std::string CalendarService::monthText() const {
  return "THÁNG " + std::to_string(current.month);
}

std::string CalendarService::dayText() const {
  return std::to_string(current.day);
}

std::string CalendarService::weekdayText() const {
  static const char* const weekdays[] = {
      "", "THỨ HAI", "THỨ BA", "THỨ TƯ", "THỨ NĂM", "THỨ SÁU", "THỨ BẢY", "CHỦ NHẬT"};
  return (current.weekday >= 1 && current.weekday <= 7) ? weekdays[current.weekday] : "";
}

std::string CalendarService::lunarText() const {
  return "Âm lịch: " + std::to_string(current.lunarDay) + " tháng " +
         std::to_string(current.lunarMonth);
}
