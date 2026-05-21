#include "connectivity/task_check_info.h"

#define FW_VERSION "2"

void Load_info_File()
{
  File file = LittleFS.open("/info.dat", "r");
  if (!file)
  {
    return;
  }
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
  }
  else
  {
    WIFI_SSID = strdup(doc["WIFI_SSID"]);
    WIFI_PASS = strdup(doc["WIFI_PASS"]);
    CORE_IOT_TOKEN = strdup(doc["CORE_IOT_TOKEN"]);
    CORE_IOT_SERVER = strdup(doc["CORE_IOT_SERVER"]);
    CORE_IOT_PORT = strdup(doc["CORE_IOT_PORT"]);
  }
  file.close();
}

void Delete_info_File()
{
  if (LittleFS.exists("/info.dat"))
  {
    LittleFS.remove("/info.dat");
  }
  ESP.restart();
}

void Save_info_File(String wifi_ssid, String wifi_pass, String CORE_IOT_TOKEN, String CORE_IOT_SERVER, String CORE_IOT_PORT)
{
  Serial.println(wifi_ssid);
  Serial.println(wifi_pass);

  DynamicJsonDocument doc(4096);
  doc["WIFI_SSID"] = wifi_ssid;
  doc["WIFI_PASS"] = wifi_pass;
  doc["CORE_IOT_TOKEN"] = CORE_IOT_TOKEN;
  doc["CORE_IOT_SERVER"] = CORE_IOT_SERVER;
  doc["CORE_IOT_PORT"] = CORE_IOT_PORT;

  File configFile = LittleFS.open("/info.dat", "w");
  if (configFile)
  {
    serializeJson(doc, configFile);
    configFile.close();
  }
  else
  {
    Serial.println("Unable to save the configuration.");
  }
  // Không restart ở đây - caller sẽ tự gửi response rồi mới restart
};

bool check_info_File(bool check)
{
  if (!check)
  {
    if (!LittleFS.begin(true))
    {
      Serial.println("❌ Lỗi khởi động LittleFS!");
      return false;
    }

    // Xóa credentials cũ nếu firmware version thay đổi
    File vf = LittleFS.open("/fw_ver.dat", "r");
    bool versionMatch = vf && (vf.readString() == FW_VERSION);
    if (vf) vf.close();
    if (!versionMatch)
    {
      LittleFS.remove("/info.dat");
      File wf = LittleFS.open("/fw_ver.dat", "w");
      if (wf) { wf.print(FW_VERSION); wf.close(); }
      Serial.println("[Info] Firmware version mới, đã xóa credentials cũ.");
    }

    Load_info_File();

    // Luôn bật AP để dashboard luôn truy cập được qua 192.168.4.1
    startAP();

    // Nếu có credentials: connect STA song song (mode đã là AP_STA từ startAP)
    if (!WIFI_SSID.isEmpty())
    {
      if (WIFI_PASS.isEmpty())
        WiFi.begin(WIFI_SSID.c_str());
      else
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }
  }

  if (WIFI_SSID.isEmpty() && WIFI_PASS.isEmpty())
  {
    return false;
  }
  return true;
}