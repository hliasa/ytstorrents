#include "infowindow.h"
#include "ui_infowindow.h"

InfoWindow::InfoWindow(double rate, int minutes, QString title,QPixmap imgPoster, QString txtSum,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(title);
    ui->txtSummary->setText(txtSum);
    ui->txtSummary->setReadOnly(true);
    ui->imgFrame->setPixmap(imgPoster.scaled(this->size(),Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->imgFrame->setScaledContents(true);

    ui->lblRate->setText(QString("Rating: %1/10").arg(rate));
    ui->lblTime->setText(QString("Duration: %1 minutes").arg(minutes));
}

InfoWindow::~InfoWindow()
{
    delete ui;
}
