#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QDebug>
class QVBoxLayout;
class QToolButton;
class QCheckBox;
class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
	SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget() override;
    void updateRecordstatus(bool bRedord);
private:
	QToolButton * creatToolButton(const QString& icon, const QString& context = QString());
private slots:
	void clickedslot(bool checked = false);
private:
	void updateSettings();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
private:
	QVBoxLayout *m_pLayout;

	QCheckBox* m_record_chkbtn;
};

#endif // MAINWIDGET_H
