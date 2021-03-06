#ifndef EXECUTABLEUPDATER_H
#define EXECUTABLEUPDATER_H

#include <QObject>
#include <QString>

namespace update_system {
  /**
   * @brief The CExecutableUpdater class replaces existing executable file with new one.
   * 1st phase - rename existing executable
   * 2nd phase - replace existing executable with downloaded one.
   */
  class CExecutableUpdater : public QObject {
    Q_OBJECT
  private:
    QString m_file_id;
    QString m_src_file_str;
    QString m_dst_file_str;

    CExecutableUpdater();

  public:

    CExecutableUpdater(const QString& src,
                       const QString& dst);
    ~CExecutableUpdater();

  public slots:
    void replace_executables(bool was_successful_downloaded);

  signals:
    void finished(bool);
  };
}

#endif // EXECUTABLEUPDATER_H
