#include "datalogger.hpp"


using namespace adiscope;

DataLogger::DataLogger(bool lastValue, bool average, bool all)
{
	dataLoggerAPI = new DataLoggerAPI();
	dataLoggerUI = new DataLoggerUI(lastValue,average,all);

	connect(dataLoggerUI, &DataLoggerUI::pathChanged, dataLoggerAPI, &DataLoggerAPI::setPath);
	connect(dataLoggerUI, &DataLoggerUI::timeIntervalChanged, dataLoggerAPI, &DataLoggerAPI::setTimerInterval);
	connect(dataLoggerUI, &DataLoggerUI::toggleDataLogger, this, &DataLogger::toggleDataLogger);

	dataLoggerAPI->setTimerInterval(5000);
}

void DataLogger::startLogger()
{
  dataLoggerAPI->startLogger(dataLoggerUI->isOverwrite());
  dataLoggerUI->isDataLoggerRunning(true);
}

void DataLogger::stopLogger()
{
	dataLoggerAPI->stopLogger();
	dataLoggerUI->isDataLoggerRunning(false);
}

void DataLogger::createChannel(QString name, Type type)
{
	ch_Filter filter =ch_Filter::LAST_VALUE ;
	QString uiFilter = dataLoggerUI->getFilter();
	if(uiFilter == "Average"){
		filter = ch_Filter::AVERAGE;
	}
	if(uiFilter == "All"){
		filter = ch_Filter::ALL;
	}
	dataLoggerAPI->createChannel(name,type,filter);
}

void DataLogger::distroyChannel(QString name)
{
	dataLoggerAPI->distroyChannel(name);
}

void DataLogger::resetChannel(QString name)
{
	dataLoggerAPI->resetChannel(name);
}


void DataLogger::receiveValue(QString name, QString value)
{
	dataLoggerAPI->receiveValue(name,value);
}

QWidget* DataLogger::getWidget()
{
	return dataLoggerUI->getDataLoggerUIWidget();
}

bool DataLogger::isDataLoggerOn()
{
	return dataLoggerUI->isDataLoggingOn();
}

void DataLogger::setWarningMessage(QString message)
{
	dataLoggerUI->setWarningMessage(message);
}

void DataLogger::dataLoggerToggled(bool toggled)
{
	dataLoggerUI->toggleDataLoggerSwitch(toggled);
	Q_EMIT toggleDataLogger(toggled);
}

void DataLogger::setPath(QString path){
	dataLoggerUI->setDataLoggerPath(path);
	dataLoggerAPI->setPath(path);
}

QString DataLogger::getPath(){
	return dataLoggerUI->getDataLoggerPath();
}

bool DataLogger::isOverwrite()
{
	return dataLoggerUI->isOverwrite();
}

void DataLogger::setOverwrite(bool en)
{
	dataLoggerUI->setOverwrite(en);
}

int DataLogger::getRecordingTimeInterval()
{
	return dataLoggerUI->getTimerInterval();
}

void DataLogger::setRecodingTimeInterval(int interval)
{
	dataLoggerUI->setTimerInterval(interval);
	dataLoggerAPI->setTimerInterval(interval * 1000);
}

DataLogger::~DataLogger()
{
	delete dataLoggerAPI;
	delete dataLoggerUI;
}

