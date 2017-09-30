/*************************************************************************************
    garlic-player: SMIL Player for Digital Signage
    Copyright (C) 2016 Nikolaos Saghiadinos <ns@smil-control.com>
    This file is part of the garlic-player source code

    This program is free software: you can redistribute it and/or  modify
    it under the terms of the GNU Affero General Public License, version 3,
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************************/
#include <QString>
#include <QFileInfo>
#include <QIODevice>
#include <QtTest>
#include <stdio.h>
#include <stdlib.h>

#include <files/network.h>

void noMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg){Q_UNUSED(type); Q_UNUSED(context); Q_UNUSED(msg)}

class TestNetwork : public QObject
{
    Q_OBJECT
public:
    TestNetwork(){}


private Q_SLOTS:
    void cleanup();
    void initTestCase(){qInstallMessageHandler(noMessageOutput);}
    void testAgentString();
    void testDownloadSmil();
    void testDownloadMedia();
    void testDownloadMediaDirect();
    void testDownloadFailed();
    void testDownloadWebSite();
    void testDownloadWebSiteWith301Redirect();
};

void TestNetwork::cleanup()
{
    QFile file("./agent.txt");
    if (file.exists())
        file.remove();
    file.setFileName("./index.smil");
    if (file.exists())
        file.remove();
    file.setFileName("./server.jpg");
    if (file.exists())
        file.remove();
}

void TestNetwork::testAgentString()
{
    QByteArray agent("this is an agent string");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-admin.com/garlic/test.php?action=show_agent_string");
    QFileInfo fi("./agent.txt");
    QFile file(fi.absoluteFilePath());

    qRegisterMetaType<QObject *>();
    QSignalSpy spy(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy.isValid());
    MyNetwork->processFile(url, fi);

    int i = 0;
    while (spy.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }

    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<Network *>(spy.at(0).at(0)), MyNetwork);
    QVERIFY(file.exists());
    file.open(QIODevice::ReadOnly);
    QCOMPARE(file.readAll(), agent);
    file.close();
}

void TestNetwork::testDownloadSmil()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-admin.com/garlic/test.php?action=get_smil_new");
    QFileInfo fi("./index.smil");
    qRegisterMetaType<QObject *>();
    QSignalSpy spy1(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy1.isValid());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);
    QFile file(":/simple.smil");

    QFile data(fi.absoluteFilePath());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data.open(QIODevice::ReadOnly | QIODevice::Text);
    QCOMPARE(data.read(68), file.read(68)); // readAll() makes not equal \r\n when read from web
    QCOMPARE(MyNetwork, qvariant_cast<QObject *>(spy1.at(0).at(0))); // readAll() makes not equal \r\n when read from web
    QDateTime first_download = fi.lastModified();
    file.close();
    data.close();

    // second try with same file is already on disk downloaded previously
    QSignalSpy spy2(MyNetwork, SIGNAL(notmodified(QObject *)));
    QVERIFY(spy2.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy2.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy2.count(), 1);
    QVERIFY(first_download == fi.lastModified());

    // check to react on http 304
    url.setUrl("http://smil-admin.com/garlic/test.php?action=get_smil_cached");
    fi.setFile(":/notavaible.smil"); // to make sure file is not exist
    QSignalSpy spy3(MyNetwork, SIGNAL(notmodified(QObject *)));
    QVERIFY(spy3.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy3.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy3.count(), 1);

    url.setUrl("http://smil-admin.com/garlic/test.php?action=get_smil_updated");
    MyNetwork->processFile(url, fi);


    QSignalSpy spy4(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy4.isValid());
    i = 0;
    while (spy4.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy4.count(), 1);

    QFileInfo fi2(fi.absoluteFilePath()); // to make sure file exists
    QVERIFY(first_download > fi2.lastModified());
}

void TestNetwork::testDownloadMedia()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-admin.com/garlic/test.php?action=get_media_new");
    QFileInfo fi("./server.jpg");
    QFileInfo fi_compare(fi.absoluteFilePath());
    qRegisterMetaType<QIODevice *>();
    QSignalSpy spy1(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy1.isValid());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);
    QDateTime first_download = fi_compare.lastModified();

    QFile data("./server.jpg");
    QFile file(":/server.jpg");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data.open(QIODevice::ReadOnly | QIODevice::Text);
    QCOMPARE(data.read(50), file.read(50)); // readAll() makes not equal \r\n when read from web
    file.close();
    data.close();

    // second try with already previous downloaded media
    QSignalSpy spy2(MyNetwork, SIGNAL(notmodified(QObject *)));
    QVERIFY(spy2.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy2.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy2.count(), 1);
    QVERIFY(fi.lastModified() == first_download);

    // check to react on http 304
    url.setUrl("http://smil-admin.com/garlic/test.php?action=get_media_cached");
    fi.setFile("./server_notavaible.jpg"); // to make sure file is not exist
    QSignalSpy spy3(MyNetwork, SIGNAL(notmodified(QObject *)));
    QVERIFY(spy3.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy3.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy3.count(), 1);
    QVERIFY(!fi.exists());
    QFileInfo fi1("./server.jpg");
    QVERIFY(first_download == fi1.lastModified());

    url.setUrl("http://smil-admin.com/garlic/test.php?action=get_media_updated");
    fi.setFile("./server.jpg"); // to make sure file exists
    QSignalSpy spy4(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy4.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy4.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy4.count(), 1);
    QFileInfo fi2(fi.absoluteFilePath()); // to make sure file exists
    QVERIFY(first_download < fi2.lastModified());
}

void TestNetwork::testDownloadMediaDirect()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-admin.com/garlic/server.jpg");
    QFileInfo fi("./server.jpg");
    QSignalSpy spy1(MyNetwork, SIGNAL(succeed(QObject *)));
    QVERIFY(spy1.isValid());
    QFile data("./server.jpg");
    QVERIFY(!data.exists());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);
    QFile file(":/server.jpg");
    QVERIFY(data.exists());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data.open(QIODevice::ReadOnly | QIODevice::Text);
    QCOMPARE(data.read(50), file.read(50)); // readAll() makes not equal \r\n when read from web
    file.close();
    data.close();

    // second try with already downloaded media
    QSignalSpy spy2(MyNetwork, SIGNAL(notmodified(QObject *)));
    QVERIFY(spy2.isValid());
    MyNetwork->processFile(url, fi);
    i = 0;
    while (spy2.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy2.count(), 1);
}

void TestNetwork::testDownloadFailed()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-admin.com/garlic/notexistingfile.jpg");
    QFileInfo fi("./notexisting");
    qRegisterMetaType<QIODevice *>();
    QSignalSpy spy1(MyNetwork, SIGNAL(failed(QObject *)));
    QVERIFY(spy1.isValid());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 2000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);

    // file exists but not valid mime/extension
    QUrl url2("http://smil-admin.com/garlic/no.bin");
    QSignalSpy spy2(MyNetwork, SIGNAL(failed(QObject *)));
    QVERIFY(spy2.isValid());
    MyNetwork->processFile(url2, fi);
    i = 0;
    while (spy2.count() == 0 && i < 2000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy2.count(), 1);
}

void TestNetwork::testDownloadWebSite()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://smil-control.com");
    QFileInfo fi;
    qRegisterMetaType<QObject *>();
    QSignalSpy spy1(MyNetwork, SIGNAL(notcacheable(QObject *)));
    QVERIFY(spy1.isValid());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(qvariant_cast<QObject *>(spy1.at(0).at(0)), MyNetwork);
    QCOMPARE(MyNetwork->getRemoteFileUrl(), url);
}

void TestNetwork::testDownloadWebSiteWith301Redirect()
{
    QByteArray agent("GAPI/1.0 (UUID:f9d65c88-e4cd-43b4-89eb-5c338e54bcae; NAME:TestTDownload) xxxxxx-xx/x.x.x (MODEL:GARLIC)");
    Network *MyNetwork = new Network(agent);
    QUrl url("http://heise.de");
    QFileInfo fi;
    qRegisterMetaType<QObject *>();
    QSignalSpy spy1(MyNetwork, SIGNAL(notcacheable(QObject *)));
    QVERIFY(spy1.isValid());
    MyNetwork->processFile(url, fi);
    int i = 0;
    while (spy1.count() == 0 && i < 4000)
    {
        QTest::qWait(200);
        i +=200;
    }
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(qvariant_cast<QObject *>(spy1.at(0).at(0)), MyNetwork);
    QCOMPARE(MyNetwork->getRemoteFileUrl(), url); // should not be something like https://heise.de
}




QTEST_MAIN(TestNetwork)

#include "tst_network.moc"
