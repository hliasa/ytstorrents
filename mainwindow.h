#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_resultsTable_clicked(const QModelIndex &index);
    void on_torrentsTable_clicked(const QModelIndex &index);
    void on_btnDownload_clicked();

    void on_search_term_returnPressed();

    void on_resultsTable_doubleClicked(const QModelIndex &index);

    void on_btnCopy_clicked();

private:
    Ui::MainWindow *ui;
    void SetTorrentsTable(int);
    QJsonArray SendRequest(QString);
};

#endif // MAINWINDOW_H
