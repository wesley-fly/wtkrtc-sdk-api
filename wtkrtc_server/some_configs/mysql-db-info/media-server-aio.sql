CREATE DATABASE  IF NOT EXISTS `asterisk-11` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `asterisk-11`;
-- MySQL dump 10.13  Distrib 5.7.13, for linux-glibc2.5 (x86_64)
--
-- Host: 127.0.0.1    Database: asterisk-11
-- ------------------------------------------------------
-- Server version	5.6.44

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `cdr`
--

DROP TABLE IF EXISTS `cdr`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cdr` (
  `cdr_id` int(11) DEFAULT NULL,
  `calldate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `clid` varchar(80) NOT NULL DEFAULT '',
  `src` varchar(80) NOT NULL DEFAULT '',
  `dst` varchar(80) NOT NULL DEFAULT '',
  `dcontext` varchar(80) NOT NULL DEFAULT '',
  `channel` varchar(80) NOT NULL DEFAULT '',
  `dstchannel` varchar(80) NOT NULL DEFAULT '',
  `lastapp` varchar(80) NOT NULL DEFAULT '',
  `lastdata` varchar(80) NOT NULL DEFAULT '',
  `duration` int(11) NOT NULL DEFAULT '0',
  `billsec` int(11) NOT NULL DEFAULT '0',
  `disposition` varchar(45) NOT NULL DEFAULT '',
  `amaflags` int(11) NOT NULL DEFAULT '0',
  `accountcode` varchar(20) NOT NULL DEFAULT '',
  `uniqueid` varchar(32) NOT NULL DEFAULT '',
  `flag` char(2) DEFAULT NULL,
  `userfield` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`calldate`,`src`,`dst`),
  KEY `src` (`src`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cdr`
--

LOCK TABLES `cdr` WRITE;
/*!40000 ALTER TABLE `cdr` DISABLE KEYS */;
INSERT INTO `cdr` VALUES (NULL,'2017-10-13 15:44:25','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000000','SIP/802-00000001','Dial','SIP/802,20',24,14,'ANSWERED',3,'','1507880665.0',NULL,''),(NULL,'2017-10-13 15:46:32','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000002','SIP/801-00000003','Dial','SIP/801,20',44,26,'ANSWERED',3,'','1507880792.2',NULL,''),(NULL,'2017-10-13 15:48:17','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000004','SIP/7002-00000005','Dial','SIP/7002,45',25,16,'ANSWERED',3,'','1507880897.4',NULL,''),(NULL,'2017-10-13 15:50:42','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-00000006','SIP/7001-00000007','Dial','SIP/7001,45',73,65,'ANSWERED',3,'','1507881042.6',NULL,''),(NULL,'2017-10-13 15:57:22','\"+867003\" <7003>','7003','7002','webtest','SIP/7003-00000008','SIP/7002-00000009','Dial','SIP/7002,45',203,196,'ANSWERED',3,'','1507881442.8',NULL,''),(NULL,'2017-10-13 16:00:53','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-0000000a','SIP/7003-0000000b','Dial','SIP/7003,45',28,12,'ANSWERED',3,'','1507881653.10',NULL,''),(NULL,'2017-10-13 16:07:34','\"+867001\" <7001>','7001','7004','webtest','SIP/7001-0000000c','SIP/7004-0000000d','Dial','SIP/7004,45',47,27,'ANSWERED',3,'','1507882054.12',NULL,''),(NULL,'2017-10-13 16:08:41','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-0000000e','SIP/7001-0000000f','Dial','SIP/7001,45',51,42,'ANSWERED',3,'','1507882121.14',NULL,''),(NULL,'2017-10-13 16:10:41','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000000','SIP/7002-00000001','Dial','SIP/7002,45',23,16,'ANSWERED',3,'','1507882241.0',NULL,''),(NULL,'2017-10-13 16:13:07','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-00000000','SIP/7001-00000001','Dial','SIP/7001,45',65,59,'ANSWERED',3,'','1507882387.0',NULL,''),(NULL,'2017-10-13 16:14:36','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000002','SIP/7002-00000003','Dial','SIP/7002,45',88,82,'ANSWERED',3,'','1507882476.2',NULL,''),(NULL,'2017-10-13 16:16:10','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-00000004','SIP/7001-00000005','Dial','SIP/7001,45',29,21,'ANSWERED',3,'','1507882570.4',NULL,''),(NULL,'2017-10-13 16:20:28','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000006','SIP/7002-00000007','Dial','SIP/7002,45',74,68,'ANSWERED',3,'','1507882828.6',NULL,''),(NULL,'2017-10-13 16:21:49','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-00000008','SIP/7003-00000009','Dial','SIP/7003,45',16,16,'ANSWERED',3,'','1507882909.8',NULL,''),(NULL,'2017-10-13 16:22:14','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-0000000a','SIP/7003-0000000b','Dial','SIP/7003,45',22,22,'ANSWERED',3,'','1507882934.10',NULL,''),(NULL,'2017-10-13 16:22:52','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-0000000c','SIP/7003-0000000d','Dial','SIP/7003,45',17,17,'ANSWERED',3,'','1507882972.12',NULL,''),(NULL,'2017-10-13 16:23:33','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-0000000e','SIP/7003-0000000f','Dial','SIP/7003,45',22,15,'ANSWERED',3,'','1507883013.14',NULL,''),(NULL,'2017-10-13 16:24:14','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000010','SIP/7002-00000011','Dial','SIP/7002,45',27,21,'ANSWERED',3,'','1507883054.16',NULL,''),(NULL,'2017-10-13 16:25:51','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-00000012','SIP/7003-00000013','Dial','SIP/7003,45',33,26,'ANSWERED',3,'','1507883151.18',NULL,''),(NULL,'2017-10-13 16:27:11','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-00000014','SIP/7003-00000015','Dial','SIP/7003,45',25,18,'ANSWERED',3,'','1507883231.20',NULL,''),(NULL,'2017-10-13 16:27:43','\"+867003\" <7003>','7003','7002','webtest','SIP/7003-00000016','','Dial','SIP/7002,45',13,13,'ANSWERED',3,'','1507883263.22',NULL,''),(NULL,'2017-10-13 17:46:54','\"+867003\" <7003>','7003','7002','webtest','SIP/7003-00000000','SIP/7002-00000001','Dial','SIP/7002,45',14,14,'BUSY',3,'','1507888014.0',NULL,''),(NULL,'2017-10-13 17:49:07','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-00000002','SIP/7003-00000003','Dial','SIP/7003,45',182,173,'ANSWERED',3,'','1507888147.2',NULL,''),(NULL,'2017-10-13 17:52:15','\"+867003\" <7003>','7003','7002','webtest','SIP/7003-00000004','SIP/7002-00000005','Dial','SIP/7002,45',19,13,'ANSWERED',3,'','1507888335.4',NULL,''),(NULL,'2017-10-13 17:52:39','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-00000006','SIP/7003-00000007','Dial','SIP/7003,45',55,36,'ANSWERED',3,'','1507888359.6',NULL,''),(NULL,'2017-10-13 17:53:52','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000008','SIP/7002-00000009','Dial','SIP/7002,45',26,18,'ANSWERED',3,'','1507888432.8',NULL,''),(NULL,'2017-10-13 17:54:25','\"+867001\" <7001>','7001','7003','webtest','SIP/7001-0000000a','SIP/7003-0000000b','Dial','SIP/7003,45',34,27,'ANSWERED',3,'','1507888465.10',NULL,''),(NULL,'2017-10-13 17:55:55','\"+867001\" <7001>','7001','7004','webtest','SIP/7001-0000000c','SIP/7004-0000000d','Dial','SIP/7004,45',31,22,'ANSWERED',3,'','1507888555.12',NULL,''),(NULL,'2017-10-13 18:36:41','\"+867004\" <7004>','7004','7001','webtest','SIP/7004-00000000','SIP/7001-00000001','Dial','SIP/7001,45',30,24,'ANSWERED',3,'','1507891001.0',NULL,''),(NULL,'2017-10-13 18:37:18','\"+867001\" <7001>','7001','7004','webtest','SIP/7001-00000002','SIP/7004-00000003','Dial','SIP/7004,45',27,16,'ANSWERED',3,'','1507891038.2',NULL,''),(NULL,'2017-10-15 15:55:40','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-00000004','SIP/7003-00000005','Dial','SIP/7003,45',79,65,'ANSWERED',3,'','1508054140.4',NULL,''),(NULL,'2017-10-15 15:57:09','\"+867003\" <7003>','7003','7002','webtest','SIP/7003-00000006','SIP/7002-00000007','Dial','SIP/7002,45',70,57,'ANSWERED',3,'','1508054229.6',NULL,''),(NULL,'2017-10-15 15:58:23','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-00000008','SIP/7003-00000009','Dial','SIP/7003,45',52,45,'ANSWERED',3,'','1508054303.8',NULL,''),(NULL,'2017-10-15 16:00:05','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-0000000a','SIP/7003-0000000b','Dial','SIP/7003,45',33,26,'ANSWERED',3,'','1508054405.10',NULL,''),(NULL,'2017-10-15 16:01:08','\"+867002\" <7002>','7002','7003','webtest','SIP/7002-0000000c','SIP/7003-0000000d','Dial','SIP/7003,45',41,34,'ANSWERED',3,'','1508054468.12',NULL,''),(NULL,'2017-10-15 16:28:44','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-0000000e','SIP/7002-0000000f','Dial','SIP/7002,45',40,33,'ANSWERED',3,'','1508056124.14',NULL,''),(NULL,'2017-10-15 16:30:12','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000010','SIP/7002-00000011','Dial','SIP/7002,45',57,50,'ANSWERED',3,'','1508056212.16',NULL,''),(NULL,'2017-10-15 16:33:06','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000012','SIP/7002-00000013','Dial','SIP/7002,45',27,20,'ANSWERED',3,'','1508056386.18',NULL,''),(NULL,'2017-10-16 17:57:10','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000014','SIP/801-00000015','Dial','SIP/801,20',102,93,'ANSWERED',3,'','1508147830.20',NULL,''),(NULL,'2017-10-20 16:18:45','\"+86802\" <802>','802','96000','callcenter','SIP/802-00000016','SIP/802-00000017','Hangup','',7,7,'BUSY',3,'','1508487525.22',NULL,''),(NULL,'2017-10-20 17:16:27','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000018','SIP/7002-00000019','Dial','SIP/7002,45',29,21,'ANSWERED',3,'','1508490987.24',NULL,''),(NULL,'2017-10-20 17:17:04','\"+867001\" <7001>','7001','96000','webtest','SIP/7001-0000001a','','Dial','SIP/96000,45',13,13,'ANSWERED',3,'','1508491024.26',NULL,''),(NULL,'2017-10-20 17:17:41','\"+867001\" <7001>','7001','96000','webtest','SIP/7001-0000001b','','Dial','SIP/96000,45',13,13,'ANSWERED',3,'','1508491061.27',NULL,''),(NULL,'2017-10-20 17:18:16','\"+867001\" <7001>','7001','96000','webtest','SIP/7001-0000001c','','Dial','SIP/96000,45',13,13,'ANSWERED',3,'','1508491096.28',NULL,''),(NULL,'2017-10-20 17:19:01','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-0000001d','SIP/801-0000001e','Hangup','',27,27,'FAILED',3,'','1508491141.29',NULL,''),(NULL,'2017-10-20 17:19:38','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-0000001f','SIP/801-00000020','Dial','SIP/801,20',204,198,'ANSWERED',3,'','1508491178.31',NULL,''),(NULL,'2017-11-02 12:11:06','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000021','SIP/801-00000022','Hangup','',17,17,'ANSWERED',3,'','1509595866.33',NULL,''),(NULL,'2017-11-02 12:12:05','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000023','SIP/801-00000024','Hangup','',11,11,'ANSWERED',3,'','1509595925.35',NULL,''),(NULL,'2017-11-02 12:12:31','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000025','SIP/801-00000026','Hangup','',12,12,'ANSWERED',3,'','1509595951.37',NULL,''),(NULL,'2017-11-02 12:13:14','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000027','SIP/802-00000028','Dial','SIP/802,20',464,457,'ANSWERED',3,'','1509595994.39',NULL,''),(NULL,'2017-11-02 15:37:40','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000000','SIP/7002-00000001','Dial','SIP/7002,45',33,24,'ANSWERED',3,'','1509608260.0',NULL,''),(NULL,'2017-11-03 10:30:28','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000002','SIP/801-00000003','Dial','SIP/801,20',35,27,'ANSWERED',3,'','1509676228.2',NULL,''),(NULL,'2017-11-03 10:31:14','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000004','SIP/801-00000005','Dial','SIP/801,20',31,12,'ANSWERED',3,'','1509676274.4',NULL,''),(NULL,'2017-11-03 10:35:20','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000006','SIP/801-00000007','Dial','SIP/801,20',96,84,'ANSWERED',3,'','1509676520.6',NULL,''),(NULL,'2017-11-03 10:38:38','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000008','SIP/801-00000009','Dial','SIP/801,20',27,21,'ANSWERED',3,'','1509676718.8',NULL,''),(NULL,'2017-11-03 10:39:46','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-0000000a','SIP/801-0000000b','Dial','SIP/801,20',27,21,'ANSWERED',3,'','1509676786.10',NULL,''),(NULL,'2017-11-03 10:43:58','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-0000000c','SIP/801-0000000d','Dial','SIP/801,20',14,7,'ANSWERED',3,'','1509677038.12',NULL,''),(NULL,'2017-11-03 10:44:23','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-0000000e','SIP/801-0000000f','Dial','SIP/801,20',27,4,'ANSWERED',3,'','1509677063.14',NULL,''),(NULL,'2017-11-03 10:46:17','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000010','SIP/801-00000011','Dial','SIP/801,20',83,70,'ANSWERED',3,'','1509677177.16',NULL,''),(NULL,'2017-11-03 10:47:53','\"+8680001\" <80001>','80001','96000','callcenter','SIP/80001-00000012','SIP/801-00000013','Dial','SIP/801,20',12,4,'ANSWERED',3,'','1509677273.18',NULL,''),(NULL,'2017-11-03 10:48:23','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000014','SIP/801-00000015','Dial','SIP/801,20',17,10,'ANSWERED',3,'','1509677303.20',NULL,''),(NULL,'2017-11-03 10:50:06','\"+8680002\" <80002>','80002','96000','callcenter','SIP/80002-00000016','SIP/801-00000017','Dial','SIP/801,20',14,7,'ANSWERED',3,'','1509677406.22',NULL,''),(NULL,'2017-11-03 13:34:33','\"+867001\" <7001>','7001','7002','webtest','SIP/7001-00000018','SIP/7002-00000019','Dial','SIP/7002,45',132,122,'ANSWERED',3,'','1509687273.24',NULL,''),(NULL,'2017-11-03 13:36:54','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-0000001a','SIP/7001-0000001b','Dial','SIP/7001,45',158,149,'ANSWERED',3,'','1509687414.26',NULL,''),(NULL,'2017-11-03 13:39:37','\"+867002\" <7002>','7002','7001','webtest','SIP/7002-0000001c','SIP/7001-0000001d','Dial','SIP/7001,45',917,909,'ANSWERED',3,'','1509687577.28',NULL,'');
/*!40000 ALTER TABLE `cdr` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `groups_control`
--

DROP TABLE IF EXISTS `groups_control`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `groups_control` (
  `group_no` varchar(80) NOT NULL,
  `auth_type` int(10) DEFAULT NULL,
  `business_type` int(10) DEFAULT NULL,
  `ptt_addr` varchar(50) DEFAULT NULL,
  `ptt_port` varchar(10) DEFAULT NULL,
  `mixer_addr` varchar(50) DEFAULT NULL,
  `mixer_port` varchar(10) DEFAULT NULL,
  `data_addr` varchar(50) DEFAULT NULL,
  `vnc_addr` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`group_no`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `groups_control`
--

LOCK TABLES `groups_control` WRITE;
/*!40000 ALTER TABLE `groups_control` DISABLE KEYS */;
/*!40000 ALTER TABLE `groups_control` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `groups_whitelist`
--

DROP TABLE IF EXISTS `groups_whitelist`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `groups_whitelist` (
  `group_no` varchar(80) DEFAULT NULL,
  `id` varchar(80) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `groups_whitelist`
--

LOCK TABLES `groups_whitelist` WRITE;
/*!40000 ALTER TABLE `groups_whitelist` DISABLE KEYS */;
/*!40000 ALTER TABLE `groups_whitelist` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `iaxfriends`
--

DROP TABLE IF EXISTS `iaxfriends`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `iaxfriends` (
  `name` varchar(30) NOT NULL DEFAULT '',
  `username` varchar(30) DEFAULT NULL,
  `type` varchar(6) NOT NULL DEFAULT 'friend',
  `secret` varchar(50) DEFAULT NULL,
  `md5secret` varchar(32) DEFAULT NULL,
  `dbsecret` varchar(100) DEFAULT NULL,
  `notransfer` varchar(10) DEFAULT NULL,
  `transfer` varchar(10) DEFAULT NULL,
  `inkeys` varchar(100) DEFAULT NULL,
  `auth` varchar(100) DEFAULT NULL,
  `accountcode` varchar(100) DEFAULT NULL,
  `amaflags` varchar(100) DEFAULT NULL,
  `callerid` varchar(100) DEFAULT NULL,
  `context` varchar(100) DEFAULT 'ppcall-intercom',
  `defaultip` varchar(15) DEFAULT NULL,
  `host` varchar(31) NOT NULL DEFAULT 'dynamic',
  `language` varchar(5) DEFAULT NULL,
  `mailbox` varchar(50) DEFAULT NULL,
  `deny` varchar(95) DEFAULT NULL,
  `permit` varchar(95) DEFAULT NULL,
  `qualify` varchar(4) DEFAULT NULL,
  `disallow` varchar(100) DEFAULT NULL,
  `allow` varchar(100) DEFAULT NULL,
  `ipaddr` varchar(15) DEFAULT NULL,
  `port` int(11) DEFAULT '0',
  `regseconds` int(11) DEFAULT '0',
  `registrar` varchar(15) DEFAULT NULL,
  `forward` varchar(40) DEFAULT NULL,
  `last_clean` int(11) NOT NULL DEFAULT '0',
  `lastregtime` int(11) DEFAULT '0',
  `lastregaddr` varchar(15) DEFAULT NULL,
  `lastregport` int(11) DEFAULT '0',
  `mobile` varchar(30) DEFAULT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `iaxfriends`
--

LOCK TABLES `iaxfriends` WRITE;
/*!40000 ALTER TABLE `iaxfriends` DISABLE KEYS */;
INSERT INTO `iaxfriends` VALUES ('10000',NULL,'friend','111111',NULL,NULL,NULL,NULL,NULL,NULL,'10000',NULL,NULL,'normal-call',NULL,'dynamic',NULL,NULL,NULL,NULL,NULL,NULL,NULL,'192.168.77.203',12130,0,NULL,NULL,0,0,NULL,0,NULL),('10001',NULL,'friend','111111',NULL,NULL,NULL,NULL,NULL,NULL,'10001',NULL,NULL,'normal-call',NULL,'dynamic',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,NULL,NULL,0,0,NULL,0,NULL);
/*!40000 ALTER TABLE `iaxfriends` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sippeers`
--

DROP TABLE IF EXISTS `sippeers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sippeers` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(15) NOT NULL,
  `type` enum('friend','user','peer') DEFAULT NULL,
  `context` varchar(40) DEFAULT NULL,
  `secret` varchar(40) DEFAULT NULL,
  `host` varchar(10) DEFAULT NULL,
  `transport` varchar(10) DEFAULT NULL,
  `avpf` varchar(10) DEFAULT NULL,
  `force_avp` varchar(10) DEFAULT NULL,
  `encryption` varchar(10) DEFAULT NULL,
  `videosupport` varchar(10) DEFAULT NULL,
  `qualify` varchar(20) DEFAULT NULL,
  `callerid` varchar(40) DEFAULT NULL,
  `session-timers` enum('accept','refuse','originate') DEFAULT NULL,
  `dtlsenable` enum('yes','no') DEFAULT NULL,
  `dtlsverify` varchar(10) DEFAULT NULL,
  `dtlscertfile` varchar(45) DEFAULT NULL,
  `dtlsprivatekey` varchar(45) DEFAULT NULL,
  `dtlssetup` varchar(10) DEFAULT NULL,
  `fullcontact` varchar(128) DEFAULT NULL,
  `ipaddr` varchar(45) DEFAULT NULL,
  `useragent` varchar(45) DEFAULT NULL,
  `port` varchar(6) DEFAULT NULL,
  `regseconds` varchar(45) DEFAULT NULL,
  `defaultuser` varchar(15) DEFAULT NULL,
  `lastms` varchar(45) DEFAULT NULL,
  `regserver` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=16 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sippeers`
--

LOCK TABLES `sippeers` WRITE;
/*!40000 ALTER TABLE `sippeers` DISABLE KEYS */;
INSERT INTO `sippeers` VALUES (1,'801','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+86801\" <801>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','sip:tcdssknf@ee8un6in61ug.invalid^3Btransport=ws','123.139.21.246','JsSIP 3.0.15','39033','1509679088','gs7c08gg','55',''),(5,'80001','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+8680001\" <80001>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','3tv11j5d','0',''),(2,'802','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+86802\" <802>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','ie6mnoi5','0',''),(6,'80002','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+8680002\" <80002>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','80002','0',''),(3,'803','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+86803\" <803>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','370h55j9','0',''),(4,'804','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+86804\" <804>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','804','0',''),(7,'80003','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+8680003\" <80003>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),(8,'80004','friend','callcenter','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+8680004\" <80004>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),(9,'7001','friend','webtest','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+867001\" <7001>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','sip:mkos2rkm@eohmff72vtj7.invalid^3Btransport=ws','210.74.155.210','JsSIP 3.0.15','23859','1509690245','gktlrm6r','-1',''),(10,'7002','friend','webtest','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+867002\" <7002>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','sip:moevf190@55p24ov2jf3j.invalid^3Btransport=ws','210.74.155.210','JsSIP 3.0.15','22858','1509703326','6dm4n0bo','-1',''),(11,'7003','friend','webtest','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+867003\" <7003>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','k8fhaft7','0',''),(12,'7004','friend','webtest','11111111','dynamic','wss','yes','yes','yes','yes','yes','\"+867004\" <7004>','refuse','yes','no','/etc/asterisk/keys/server.pem','/etc/asterisk/keys/server.key','actpass','','','','','0','pslomsm2','0','');
/*!40000 ALTER TABLE `sippeers` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-05-27 12:20:48
