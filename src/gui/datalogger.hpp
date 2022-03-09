#ifndef DATALOGGER_HPP
#define DATALOGGER_HPP

#include "dataloggerui.hpp"
#include "dataloggerapi.h"


namespace adiscope {

class DataLogger: public QObject
{
	Q_OBJECT
public:
	DataLogger(bool lastValue, bool average, bool all);
	~DataLogger();

	void createChannel(QString name, Type type);
	void distroyChannel(QString name);
	void resetChannel(QString name);
	void logData();
	QWidget *getWidget();
	bool isDataLoggerOn();
	void setWarningMessage(QString message);

public Q_SLOTS:
	void receiveValue(QString name, QString value);
	void stopLogger();
	void startLogger();
	void dataLoggerToggled(bool toggled);
	void setPath(QString path);
	QString getPath();
	bool isOverwrite();
	void setOverwrite(bool en);
	int getRecordingTimeInterval();
	void setRecodingTimeInterval(int interval);

Q_SIGNALS:
	void toggleDataLogger(bool toggled);

private:
	DataLoggerAPI *dataLoggerAPI;
	DataLoggerUI *dataLoggerUI;

};
}
#endif // DATALOGGER_HPP
