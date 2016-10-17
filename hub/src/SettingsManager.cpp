#include <QApplication>
#include <QDir>
#include <QUuid>
#include <QStandardPaths>

#include "SettingsManager.h"
#include "ApplicationLog.h"
#include "updater/HubComponentsUpdater.h"

const QString CSettingsManager::ORG_NAME("Optimal-dynamics");
const QString CSettingsManager::APP_NAME("SS_Tray");

const QString CSettingsManager::SM_LOGIN("Login");
const QString CSettingsManager::SM_PASSWORD("Password");
const QString CSettingsManager::SM_REMEMBER_ME("Remember_Me");

const QString CSettingsManager::SM_REFRESH_TIME("Refresh_Time_Sec");
const QString CSettingsManager::SM_P2P_PATH("P2P_Path");

const QString CSettingsManager::SM_NOTIFICATION_DELAY_SEC("Notification_Delay_Sec");
const QString CSettingsManager::SM_PLUGIN_PORT("Plugin_Port");
const QString CSettingsManager::SM_SSH_PATH("Ssh_Path");
const QString CSettingsManager::SM_SSH_USER("Ssh_User");

const QString CSettingsManager::SM_RH_USER("Rh_User");
const QString CSettingsManager::SM_RH_PASS("Rh_Pass");
const QString CSettingsManager::SM_RH_HOST("Rh_Host");
const QString CSettingsManager::SM_RH_PORT("Rh_Port");

const QString CSettingsManager::SM_LOGS_STORAGE("Rh_Logs_Storage");
const QString CSettingsManager::SM_SSH_KEYS_STORAGE("Rh_Ssh_Keys_Storage");

const QString CSettingsManager::SM_TRAY_GUID("Tray_Guid");

const QString CSettingsManager::SM_P2P_UPDATE_FREQ("P2p_update_freq");
const QString CSettingsManager::SM_RH_UPDATE_FREQ("Rh_update_freq");
const QString CSettingsManager::SM_TRAY_UPDATE_FREQ("Tray_update_freq");
const QString CSettingsManager::SM_P2P_AUTOUPDATE("P2p_Autoupdate");
const QString CSettingsManager::SM_RH_AUTOUPDATE("Rh_Autoupdate");
const QString CSettingsManager::SM_TRAY_AUTOUPDATE("Tray_Autoupdate");

template<class T> struct setting_val_t {
  T* field;
  QString val;
};

////////////////////////////////////////////////////////////////////////////

static const int def_timeout = 120;
CSettingsManager::CSettingsManager() :
  m_settings(QSettings::NativeFormat, QSettings::UserScope, ORG_NAME, APP_NAME),
  m_login(m_settings.value(SM_LOGIN).toString()),
  m_password(m_settings.value(SM_PASSWORD).toString()),
  m_remember_me(m_settings.value(SM_REMEMBER_ME).toBool()),
  m_refresh_time_sec(def_timeout),
  #ifdef RT_OS_LINUX
  m_p2p_path("p2p"),
  #elif RT_OS_DARWIN
  m_p2p_path("/Applications/Subutai/p2p"),
  #elif RT_OS_WINDOWS
  m_p2p_path("p2p.exe"),
  #endif
  m_notification_delay_sec(5),
  m_plugin_port(9998),
  m_ssh_path("ssh"),
  m_ssh_user("root"),
  m_rh_host("127.0.0.1"),
  m_rh_user("ubuntu"),
  m_rh_pass("ubuntu"),
  m_rh_port(4567),
  m_logs_storage(QApplication::applicationDirPath()),
  m_ssh_keys_storage(QApplication::applicationDirPath()),
  m_tray_guid(""),
  m_p2p_update_freq(UF_MIN30),
  m_rh_update_freq(UF_MIN30),
  m_tray_update_freq(UF_MIN30),
  m_p2p_autoupdate(false),
  m_rh_autoupdate(false),
  m_tray_autoupdate(false)
{
  static const char* SSH_FOLDER = ".ssh";

  do {
    QStringList lst_home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (lst_home.empty()) break;
    QString home_folder = lst_home[0];
    QString ssh = home_folder + QDir::separator() + SSH_FOLDER;
    QDir ssh_dir(ssh);
    if (!ssh_dir.exists()) {
      if (!ssh_dir.mkdir(ssh)) break;
      m_ssh_keys_storage = ssh;
    }
  } while (0);

  setting_val_t<QString> dct_str_vals[] = {
    {&m_p2p_path, SM_P2P_PATH},
    {&m_ssh_path, SM_SSH_PATH},
    {&m_ssh_user, SM_SSH_USER},
    {&m_rh_host, SM_RH_HOST},
    {&m_rh_pass, SM_RH_PASS},
    {&m_rh_user, SM_RH_USER},
    {&m_logs_storage, SM_LOGS_STORAGE},
    {&m_ssh_keys_storage, SM_SSH_KEYS_STORAGE},
    {&m_tray_guid, SM_TRAY_GUID},
    {nullptr, ""}
  };

  for (int i = 0; dct_str_vals[i].field != nullptr; ++i) {
    if (!m_settings.value(dct_str_vals[i].val).isNull()) {
      *dct_str_vals[i].field = m_settings.value(dct_str_vals[i].val).toString();
    }
  }
  /////

  setting_val_t<bool> dct_bool_vals[] = {
    {&m_remember_me, SM_REMEMBER_ME},
    {&m_p2p_autoupdate, SM_P2P_AUTOUPDATE},
    {&m_rh_autoupdate, SM_RH_AUTOUPDATE},
    {&m_tray_autoupdate, SM_TRAY_AUTOUPDATE},
    {nullptr, ""}
  };

  for (int i = 0; dct_bool_vals[i].field != nullptr; ++i) {
    if (!m_settings.value(dct_bool_vals[i].val).isNull()) {
      *dct_bool_vals[i].field = m_settings.value(dct_bool_vals[i].val).toBool();
    }
  }
  /////

  setting_val_t<uint32_t> dct_uint32_vals[] = {
    {&m_p2p_update_freq, SM_P2P_UPDATE_FREQ},
    {&m_rh_update_freq, SM_RH_UPDATE_FREQ},
    {(uint32_t*)&m_rh_port, SM_RH_PORT},
    {&m_tray_update_freq, SM_TRAY_UPDATE_FREQ},
    {nullptr, ""}
  };

  for (int i = 0; dct_uint32_vals[i].field != nullptr; ++i) {
    if (!m_settings.value(dct_uint32_vals[i].val).isNull()) {
      *dct_uint32_vals[i].field = m_settings.value(dct_uint32_vals[i].val).toUInt();
    }
  }
  /////

  bool ok = false;
  if (!m_settings.value(SM_REFRESH_TIME).isNull()) {
    uint32_t timeout = m_settings.value(SM_REFRESH_TIME).toUInt(&ok);
    m_refresh_time_sec = ok ? timeout : def_timeout;
  }

  if (!m_settings.value(SM_NOTIFICATION_DELAY_SEC).isNull()) {
    uint32_t nd = m_settings.value(SM_NOTIFICATION_DELAY_SEC).toUInt(&ok);
    if (ok) set_notification_delay_sec(nd);
  }

  if (m_tray_guid.isEmpty()) {
    m_tray_guid = QUuid::createUuid().toString();
    m_settings.setValue(SM_TRAY_GUID, m_tray_guid);
  }
  /////
}
////////////////////////////////////////////////////////////////////////////

const QString &
CSettingsManager::update_freq_to_str(CSettingsManager::update_freq_t fr) {
  static const QString strings[] {
    "1 minute", "5 minutes", "10 minutes", "30 minutes",
    "1 hour", "3 hour", "5 hour", "Daily",
    "Weekly", "Monthly", "Never" };
  return strings[fr%UF_LAST];
}
////////////////////////////////////////////////////////////////////////////

uint32_t CSettingsManager::update_freq_to_sec(CSettingsManager::update_freq_t fr) {
  static const int min = 60;
  static const int hr = min*60;
  static const int day = hr*24;
  static const uint32_t time_sec[] {
    min, min*5, min*10, min*30,
        hr, hr*3, hr*5, day,
        day*7, day*28, 0 }; //let's say 1 month = 4 week
  return time_sec[fr%UF_LAST];
}
////////////////////////////////////////////////////////////////////////////

void
CSettingsManager::set_logs_storage(const QString &logs_storage) {
  m_logs_storage = logs_storage;
  m_settings.setValue(SM_LOGS_STORAGE, m_logs_storage);
  CApplicationLog::Instance()->SetDirectory(m_logs_storage.toStdString());
}
////////////////////////////////////////////////////////////////////////////

void
CSettingsManager::set_p2p_update_freq(int fr) {
  m_p2p_update_freq = (update_freq_t) fr%UF_LAST;
  m_settings.setValue(SM_P2P_UPDATE_FREQ, (int8_t)m_p2p_update_freq);
  update_system::CHubComponentsUpdater::Instance()->set_p2p_update_freq();
}

void
CSettingsManager::set_rh_update_freq(int fr) {
  m_rh_update_freq = (update_freq_t) fr%UF_LAST;
  m_settings.setValue(SM_RH_UPDATE_FREQ, (int8_t)m_rh_update_freq);
  update_system::CHubComponentsUpdater::Instance()->set_rh_update_freq();
}

void
CSettingsManager::set_tray_update_freq(int fr) {
  m_tray_update_freq = (update_freq_t) fr%UF_LAST;
  m_settings.setValue(SM_TRAY_UPDATE_FREQ, (int8_t)m_tray_update_freq);
  update_system::CHubComponentsUpdater::Instance()->set_tray_update_freq();
}
////////////////////////////////////////////////////////////////////////////

void
CSettingsManager::set_p2p_autoupdate(const bool p2p_autoupdate) {
  m_p2p_autoupdate = p2p_autoupdate;
  m_settings.setValue(SM_P2P_AUTOUPDATE, m_p2p_autoupdate);
  update_system::CHubComponentsUpdater::Instance()->set_p2p_autoupdate();
}

void
CSettingsManager::set_rh_autoupdate(const bool rh_autoupdate) {
  m_rh_autoupdate = rh_autoupdate;
  m_settings.setValue(SM_RH_AUTOUPDATE, m_rh_autoupdate);
  update_system::CHubComponentsUpdater::Instance()->set_rh_autoupdate();
}

void
CSettingsManager::set_tray_autoupdate(const bool tray_autoupdate) {
  m_tray_autoupdate = tray_autoupdate;
  m_settings.setValue(SM_TRAY_AUTOUPDATE, m_tray_autoupdate);
  update_system::CHubComponentsUpdater::Instance()->set_tray_autoupdate();
}
////////////////////////////////////////////////////////////////////////////

#define SET_FIELD_DEF(f, fn, t) void CSettingsManager::set_##f(const t f) {m_##f = f; m_settings.setValue(fn, m_##f);}
SET_FIELD_DEF(login, SM_LOGIN, QString&)
SET_FIELD_DEF(password, SM_PASSWORD, QString&)
SET_FIELD_DEF(remember_me, SM_REMEMBER_ME, bool)
SET_FIELD_DEF(refresh_time_sec, SM_REFRESH_TIME, uint32_t)
SET_FIELD_DEF(p2p_path, SM_P2P_PATH, QString&)
SET_FIELD_DEF(plugin_port, SM_PLUGIN_PORT, uint16_t)
SET_FIELD_DEF(ssh_path, SM_SSH_PATH, QString&)
SET_FIELD_DEF(ssh_user, SM_SSH_USER, QString&)
SET_FIELD_DEF(rh_user, SM_RH_USER, QString&)
SET_FIELD_DEF(rh_pass, SM_RH_PASS, QString&)
SET_FIELD_DEF(rh_host, SM_RH_HOST, QString&)
SET_FIELD_DEF(rh_port, SM_RH_PORT, quint16)
SET_FIELD_DEF(ssh_keys_storage, SM_SSH_KEYS_STORAGE, QString&)
#undef SET_FIELD_DEF
