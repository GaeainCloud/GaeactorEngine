#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void deal_data_process_func_callback(int total,int cur);
signals:
    void setprocess_sig(int cur);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void setprocess_slot(int cur);

private:
    Ui::MainWindow *ui;
    int m_total;
};
#endif // MAINWINDOW_H
