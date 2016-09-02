#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QString>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QClipboard>
#include "infowindow.h"
#include "ui_infowindow.h"


QJsonArray g_movies,g_torrents;
int g_total,g_minutes;
QString g_hash,g_name,g_link,g_name_full;
double g_rate;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //set Table titles
    ui->resultsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Movie"));
    ui->resultsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Year"));
    ui->resultsTable->horizontalHeader()->setFixedHeight(30);
    ui->resultsTable->horizontalHeader()->setStretchLastSection(true);
    ui->torrentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Quality"));
    ui->torrentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Size"));
    ui->torrentsTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Peers"));
    ui->torrentsTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Seeds"));
    ui->torrentsTable->horizontalHeader()->setFixedHeight(30);
    ui->torrentsTable->horizontalHeader()->setStretchLastSection(true);
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->btnDownload->setEnabled(false);                 //clear lists from previous results
    ui->btnCopy->setEnabled(false);                     //
    ui->lblSelection->setText("Selected movie: none");  //
    ui->resultsTable->clearContents();                  //
    ui->resultsTable->setRowCount(0);                   //
    ui->torrentsTable->clearContents();                 //
    ui->torrentsTable->setRowCount(0);                  //

    QString URL = ui->search_term->text();
    QJsonArray movies = SendRequest(URL);
    g_movies = movies;
    QString total = QString("Found %1 results").arg(g_total);
    ui->lblTotal->setText(total);
    ui->resultsTable->setRowCount(g_movies.count());
    int i = 0;
    foreach (const QJsonValue & value, g_movies)
    {
        QJsonObject mov = value.toObject();
        QString year;
        //qDebug() << mov.find("title").value().toString();
        ui->resultsTable->setItem(i,0, new QTableWidgetItem(mov.find("title").value().toString()));
        ui->resultsTable->setItem(i,1, new QTableWidgetItem(year.setNum(mov.find("year").value().toInt())));
        i++;
    }
}
QJsonArray MainWindow::SendRequest(QString txtTerms)
{

    //qDebug() << txtTerms;
    // create custom temporary event loop on stack
    QEventLoop eventLoop;
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QNetworkRequest req( QUrl( QString("https://yts.ag/api/v2/list_movies.json?limit=50&query_term=" + txtTerms) ) );
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called
    QJsonArray ret;
    if (reply->error() == QNetworkReply::NoError) {
        //success
        //qDebug() << "Success";
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonObject object = document.object();
        QJsonObject data = object.value("data").toObject();
        QJsonArray movies = data.find("movies").value().toArray();
        g_total = data.find("movie_count").value().toInt();
        //qDebug() << movie_count<< "movies found";
        delete reply;
        ret = movies;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
    return ret;
}

void MainWindow::SetTorrentsTable(int index){
    ui->torrentsTable->setRowCount(g_movies.at(index).toObject().find("torrents").value().toArray().count());
    g_torrents = g_movies.at(index).toObject().find("torrents").value().toArray();
    //qDebug() << torrents.at(0).toObject().find("hash").value().toString();
    for(int i=0;i<g_torrents.count();i++)
    {
        QString peers,seeds;
        peers.setNum(g_torrents.at(i).toObject().find("peers").value().toInt());
        seeds.setNum(g_torrents.at(i).toObject().find("seeds").value().toInt());
        //qDebug() << "peers"<< peers;
        ui->torrentsTable->setItem(i,0,new QTableWidgetItem(g_torrents.at(i).toObject().find("quality").value().toString()));
        ui->torrentsTable->setItem(i,1,new QTableWidgetItem(g_torrents.at(i).toObject().find("size").value().toString()));
        ui->torrentsTable->setItem(i,2,new QTableWidgetItem(peers));
        ui->torrentsTable->setItem(i,3,new QTableWidgetItem(seeds));
    }
}

void MainWindow::on_resultsTable_clicked(const QModelIndex &index)
{
    //qDebug() << index.row();
    g_name = g_movies.at(index.row()).toObject().find("slug").value().toString();
    g_name_full = g_movies.at(index.row()).toObject().find("title_long").value().toString();
    g_rate = g_movies.at(index.row()).toObject().find("rating").value().toDouble();
    g_minutes = g_movies.at(index.row()).toObject().find("runtime").value().toInt();
    SetTorrentsTable(index.row());
    ui->btnDownload->setEnabled(false);
    ui->btnCopy->setEnabled(false);
    ui->lblSelection->setText("Selected movie: none");
    ui->torrentsTable->clearSelection();
}

void MainWindow::on_torrentsTable_clicked(const QModelIndex &index)
{
    g_hash = g_torrents.at(index.row()).toObject().find("hash").value().toString();
    g_link = "magnet:?xt=urn:btih:"+g_hash+"&dn="+g_name+"&tr=udp://tracker.leechers-paradise.org:6969&tr=udp://open.demonii.com:1337/announce&tr=udp://tracker.openbittorrent.com:80&tr=udp://tracker.coppersurfer.tk:6969&tr=udp://glotorrents.pw:6969/announce&tr=udp://tracker.opentrackr.org:1337/announce&tr=udp://torrent.gresille.org:80/announce&tr=udp://p4p.arenabg.com:1337";
    ui->btnDownload->setEnabled(true);
    ui->btnCopy->setEnabled(true);
    ui->lblSelection->setText("Selected movie: " + g_name_full);
}

void MainWindow::on_btnDownload_clicked()
{
    QDesktopServices::openUrl(g_link);
}

void MainWindow::on_search_term_returnPressed()
{
    on_pushButton_clicked();
}

void MainWindow::on_resultsTable_doubleClicked(const QModelIndex &index)
{
    QPixmap pixmap;
    // create custom temporary event loop on stack
    QEventLoop eventLoop;
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    // the HTTP request
    QNetworkRequest req( QUrl( QString(g_movies.at(index.row()).toObject().find("large_cover_image").value().toString()) ) );
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called
    QJsonArray ret;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray jpegData = reply->readAll();
        pixmap.loadFromData(jpegData);
        delete reply;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
    InfoWindow *winfo = new InfoWindow(g_rate, g_minutes, g_name_full, pixmap,g_movies.at(index.row()).toObject().find("summary").value().toString());
    winfo->show();
    /*QMessageBox info;
    info.setWindowTitle(g_movies.at(index.row()).toObject().find("title_long").value().toString());
    info.setText(g_movies.at(index.row()).toObject().find("summary").value().toString());
    info.exec();*/
}
void MainWindow::on_btnCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(g_link);
}
