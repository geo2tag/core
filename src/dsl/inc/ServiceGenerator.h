#ifndef SERVICEGENERATOR_H
#define SERVICEGENERATOR_H

#include <QString>
#include "ServiceDescription.h"

/**
 * This class does generation of the stubproject by given ServiceDescription
 */


// Commands
#define COPY_STUB_PROJECT QString("cp -R stubproject/ %1/")
#define SED_REPLACE_COMMAND QString("sed -i \'s|%1|%2|g\' %3")

// Directories
#define TEMPLATES_DIR QString("templates/")
#define SOURCES_SUBDIR QString("src/org/geo2tag/stubproject/")
#define TEST_SUBDIR QString("test/src/org/geo2tag/stubproject/test/")


// Template files
#define SETTINGS_FILE QString("Settings.java")
#define MONITOR_FILE QString("CustomMonitor.java")
#define TRACKABLE_FILE QString("CustomTrackable.java")
#define MANIFEST_FILE QString("AndroidManifest.xml")
#define CUSTOM_MONITOR_FILE QString("CustomMonitor.java")
#define CUSTOM_TRACKABLE_FILE QString("CustomTrackable.java")
#define GOOGLE_MAP_ACTIVITY_CLASS QString("MapGoogleActivity")
#define OSM_MAP_ACTIVITY_CLASS QString("MapJsActivity")
#define MAIN_ACTIVITY_FILE QString("MainActivity.java")
#define ANDROID_SERVICE_TEST_FILE QString("ServiceTest.java")


// Placeholders
#define ANDROID_SERVICES_PLACEHOLDER "SERVICES_PLACEHOLDER"
#define SERVER_URL_PLACEHOLDER "SERVER_URL_PLACEHOLDER"
#define DB_NAME_PLACEHOLDER "DB_NAME_PLACEHOLDER"
#define MAP_ACTIVITY_PLACEHOLDER "MAP_ACTIVITY_PLACEHOLDER"
#define MAP_ACTIVITY_TRANSFER_PLACEHOLDER "\/\/MAP_ACTIVITY_TRANSFER_PLACEHOLDER"
#define BASE_ACTIVITY_PLACEHOLDER "BASE_ACTIVITY_PLACEHOLDER" 
#define ANDROID_SERVICE_NAME_PLACEHOLDER "SERVICE_NAME_PLACEHOLDER"

#define ANDROID_SERVICE_TEST_TEMPLATE QString("Test.java")

#define MAP_ACTIVITY_FILE QString("CustomMapActivity.java")
#define MAP_ACTIVITY_TRANSFER_STRING QString("startActivity(new Intent(MainActivity.this, org.geo2tag.stubproject.CustomMapActivity.class));")
#define MAP_ACTIVITY_XML_STRING QString("<activity android:name=\"org.geo2tag.stubproject.CustomMapActivity\"></activity>");
#define ANDROID_SERVICE_XML_STRING QString("<service android:enabled=\"true\" android:name=\"org.geo2tag.stubproject.%1\" />")


class ServiceGenerator
{
private:

    QString m_outputDir;
    QString m_serviceName;
    ServiceDescription m_serviceDescription;


    QString m_mapActivity;
    QString m_mapActivityTransfer;
    QString m_servicesList;


    bool isDirectoryWritable(const QString &outputDir) const;

    void createStubProject();
    void initSettings();
    void initMonitors();
    void initTrackables();
    void initMapActivity();
    void initMainActivity();
    void initMapActivityTransition();
    void initManifestFile();
    void initTests();

    void addAndroidServiceToList(const QString& serviceName);

    QString getServicePath() const;
    QString getServiceTestPath() const;
    QString getServiceSourcesPath() const;

    bool copyFile(const QString& sourceFile, const QString& destinationDir, const QString& destinationName="");

    void replacePlaceholders(const QString& file, const QString& placeholder,
                                               const QString& value);

    QString getAndroidServiceTestFileName(const QString& serviceName) const;
    void createTestForAndroidService(const QString& serviceName);

public:

    ServiceGenerator(const QString &serviceName, const QString &outputDir, const ServiceDescription &serviceDescription);

    void generateService();
    void replacePlaceholderForMapObject(const QString &file, MapObject *object);
};

#endif // SERVICEGENERATOR_H
