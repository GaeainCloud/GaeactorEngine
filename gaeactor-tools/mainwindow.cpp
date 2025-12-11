#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamanager.hpp"
#include <QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this,&MainWindow::setprocess_sig, this, &MainWindow::setprocess_slot);
    DataManager::getInstance().set_deal_data_process_func_callback(std::bind(&MainWindow::deal_data_process_func_callback,this,std::placeholders::_1,std::placeholders::_2));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::deal_data_process_func_callback(int total, int cur)
{
    if(cur == -1)
    {
        m_total = total;
        ui->horizontalSlider->setRange(0,m_total);
        ui->label_5->setText(QString("0 / %1").arg(total));
    }
    else
    {
        emit setprocess_sig(cur);
    }
}

void MainWindow::on_pushButton_clicked()
{
    QString path = ui->lineEdit_3->text();
    double diff = ui->lineEdit->text().toDouble();
    double sapce = ui->lineEdit_2->text().toDouble();
    DataManager::getInstance().dealpath(path,diff,sapce);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString _dir_name = QFileDialog::getExistingDirectory(this, "choose dir",QDir::currentPath(), QFileDialog::ShowDirsOnly);
    ui->lineEdit_3->setText(_dir_name);

}

void MainWindow::setprocess_slot(int cur)
{
    ui->horizontalSlider->setValue(cur);

    QString tracestr = QString("%1 deal path process:%2 / %3 %4%").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                           .arg(cur).arg(m_total)
                           .arg((double)(cur) / (double)(m_total) * 100);
    ui->label_5->setText(tracestr);

}
