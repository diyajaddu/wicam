#include <QFile>

#include <QThread>

#include "wifimodechanger.h"
#include "config.h"


#define WPASUPPLICANT_FILENAME "/etc/wpa_supplicant/wpa_supplicant.conf"


/*
 *  make sure dnsmasq and hostapd don't start as a service every time you boot as we only want this to happen selectively.
 *
 *  $ update-rc.d -f hostapd remove
 *  $ update-rc.d -f dnsmasq remove
*/

WifiModeChanger::WifiModeChanger(QObject *parent) : QObject(parent)
{
    //QProcess::execute("systemctl stop dnsmasq");
    //QProcess::execute("systemctl stop hostapd");
    //QProcess::execute("service dhcpcd restart");
}

void WifiModeChanger::changeWifiMode(WIFI_MODE mode, const QString &wirelessDevice)
{
    /*QString dhcpcdConf( "hostname\n"
                        "clientid\n"
                        "persistent\n"
                        "option rapid_commit\n"
                        "option domain_name_servers, domain_name, domain_search, host_name\n"
                        "option classless_static_routes\n"
                        "option ntp_servers\n"
                        "option interface_mtu\n"
                        "require dhcp_server_identifier\n"
                        "slaac private\n");*/

    // Since the configuration files are not ready yet, turn the new software off as follows


    if(mode == WIFI_MODE::ACCESS_POINT_MODE)
    {

        //QProcess::execute("service stop dnsmasq");
        //QProcess::execute("service stop hostapd");

        //dhcpcdConf.append(QString("interface %1\n"
        //                  "\tstatic ip_address=192.168.4.1/24\n"
        //                  "\tnohook wpa_supplicant\n").arg(wirelessDevice));
        // Disable Client mode by order no hooking for the wpa_supplicant
        //QProcess::execute(QString("echo \'%1\' > %2").arg(dhcpcdConf).arg(
        //                      WIFI_DHCPCD_CONFIGURATION_FILE_NAME));
        //QFile dhcp_conf(WIFI_DHCPCD_CONFIGURATION_FILE_NAME);
        //if(!dhcp_conf.open(QIODevice::WriteOnly))
        //{
        //    qCritical("Can't open dhcpcd configuration file");
        //    return;
        //}
        // Write the data to file
        //dhcp_conf.write(dhcpcdConf.toStdString().c_str());
        //dhcp_conf.close();
        // Now restart the dhcpcd daemon and set up the new wireless device configuration:
        //QProcess::execute("service dhcpcd restart");
        // we are going to provide IP addresses between 192.168.4.2 and 192.168.4.20, with a lease time of 24 hours.
        // If you are providing DHCP services for other network devices (e.g. eth0),
        // you could add more sections with the appropriate interface header,
        // with the range of addresses you intend to provide to that interface.
        QProcess::execute("killall wpa_supplicant");
        QProcess::execute(QString("ifconfig %1 down").arg(wirelessDevice));
        QFile dnsmasq_conf(WIFI_DNSMASQ_CONFIGURATION_FILE_NAME);
        if(!dnsmasq_conf.open(QIODevice::WriteOnly))
        {
            qCritical("Can't open dnsmasq configuration file");
            return;
        }
        //QProcess::execute(QString("echo \'interface=%1\n"
        //                  "dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h\'"
        //                  "> %2").arg(wirelessDevice).arg(WIFI_DNSMASQ_CONFIGURATION_FILE_NAME));
        dnsmasq_conf.write(QString("interface=%1\n"
                                   "dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h").arg(
                               wirelessDevice).toStdString().c_str());
        dnsmasq_conf.close();
        // This configuration assumes we are using channel 7,
        // with a network name of NameOfNetwork,
        // and a password AardvarkBadgerHedgehog.
        // Note that the name and password should not have quotes around them.
        // The passphrase should be between 8 and 64 characters in length.

        QFile hostapd_conf(WIFI_HOSTAPD_CONFIGURATION_FILE_NAME);
        if(!hostapd_conf.open(QIODevice::WriteOnly))
        {
            qCritical("Can't open hostapd configuration file");
            return;
        }

        hostapd_conf.write(QString("interface=%1\n"
                                  "driver=nl80211\n"
                                  "ssid=NameOfNetwork\n"
                                  "hw_mode=g\n"
                                  "channel=7\n"
                                  "wmm_enabled=0\n"
                                  "macaddr_acl=0\n"
                                  "auth_algs=1\n"
                                  "ignore_broadcast_ssid=0\n"
                                  "wpa=2\n"
                                  "wpa_passphrase=12341234\n"
                                  "wpa_key_mgmt=WPA-PSK\n"
                                  "wpa_pairwise=TKIP\n"
                                  "rsn_pairwise=CCMP").arg(wirelessDevice).toStdString().c_str());
        // We now need to tell the system where to find this configuration file.
        hostapd_conf.close();

        QFile hostapd_mainconf(WIFI_HOSTAPD_MAIN_FILE_NAME);
        if(!hostapd_mainconf.open(QIODevice::WriteOnly))
        {
            qCritical("Can't open hostapd main configuration file");
            return;
        }
        hostapd_mainconf.write(QString("DAEMON_CONF=\"%1\"").arg(
                               WIFI_HOSTAPD_CONFIGURATION_FILE_NAME).toStdString().c_str());
        hostapd_mainconf.close();
        //QProcess::execute(QString("echo \'DAEMON_CONF=\"%1\"\' > %2").arg(
        //                      WIFI_HOSTAPD_CONFIGURATION_FILE_NAME).arg(WIFI_HOSTAPD_MAIN_FILE_NAME));
        // Now start up the remaining services

        QProcess::execute("/etc/init.d/hostapd restart");
        QThread::msleep(1000);
        QProcess::execute(QString("ifconfig %1 192.168.4.1").arg(wirelessDevice));
        QThread::msleep(1000);
        QProcess::execute("/etc/init.d/dnsmaq restart");
    }
    else
    {
        //QFile dhcp_conf(WIFI_DHCPCD_CONFIGURATION_FILE_NAME);
        //if(!dhcp_conf.open(QIODevice::WriteOnly))
        //{
        //    qCritical("Can't open dhcpcd configuration file");
        //    return;
        //}
        // Write the data to file
        //dhcp_conf.write(dhcpcdConf.toStdString().c_str());
        //dhcp_conf.close();
        // Now restart the dhcpcd daemon and set up the new wireless device configuration:
        QProcess::execute("/etc/init.d/hostapd stop");
        QProcess::execute("/etc/init.d/dnsmasq stop");

        // Run the wireless device as client again
        QProcess::execute(QString("ifconfig %1 up").arg(wirelessDevice));
        // attempt to connect
        //QProcess::execute(QString("sudo wpa_supplicant -i%1 -c%2 -B")
        //                  .arg(wirelessDevice).arg(WPASUPPLICANT_FILENAME));
        QProcess::execute("/etc/init.d/dhcpcd restart");

    }

}
