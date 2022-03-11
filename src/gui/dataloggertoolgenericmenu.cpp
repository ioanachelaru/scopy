#include "dataloggertoolgenericmenu.hpp"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "customSwitch.hpp"

using namespace adiscope::gui;

DataLoggerToolGenericMenu::DataLoggerToolGenericMenu(QWidget *parent):
	GenericMenu(parent)
{}

DataLoggerToolGenericMenu::~DataLoggerToolGenericMenu()
{}

void DataLoggerToolGenericMenu::init(QString title, QColor* color, bool showAll){
	this->initInteractiveMenu();
	this->setMenuHeader(title,color,false);

	auto *scaleSection = new adiscope::gui::SubsectionSeparator("Settings", false,this);

	//show all
	if(showAll){
		QWidget *showAllWidget = new QWidget(this);
		auto showAllLayout = new QHBoxLayout(showAllWidget);
		auto showAllSWitch = new CustomSwitch(showAllWidget);

		showAllLayout->addWidget(new QLabel("Show all",showAllWidget));
		showAllLayout->addWidget(showAllSWitch);

		scaleSection->setContent(showAllWidget);
		connect(showAllSWitch, &CustomSwitch::toggled, this, [=](bool toggled){
			Q_EMIT DataLoggerToolGenericMenu::toggleAll(toggled);
		});
	}

	//scale
	QWidget *scaleWidget = new QWidget(this);
	auto scaleLayout = new QHBoxLayout(scaleWidget);
	scaleSWitch = new CustomSwitch(scaleWidget);
	scaleSWitch->setChecked(true);

	scaleLayout->addWidget(new QLabel("Scale",scaleWidget));
	scaleLayout->addWidget(scaleSWitch);

	scaleSection->setContent(scaleWidget);

	////history
	auto *historyWidget = new QWidget(this);
	auto *historyLayout = new QVBoxLayout(historyWidget);
	auto *h1layout = new QHBoxLayout(historyWidget);

	historySwitch = new CustomSwitch(historyWidget);
	historySwitch->setChecked(true);
	historyLayout->addWidget(historySwitch);
	historyLayout->setAlignment(historySwitch,Qt::AlignRight);

	QComboBox *historySize = new QComboBox(historyWidget);
	historySize->addItem(QString("1s"));
	historySize->addItem(QString("10s"));
	historySize->addItem(QString("60s"));
	historySize->setCurrentIndex(1);

	auto *historySizeLabel = new QLabel("History Size",historyWidget);
	h1layout->addWidget(historySizeLabel);
	h1layout->addWidget(historySize);

	historyLayout->addItem(h1layout);

	auto *h2layout = new QHBoxLayout(historyWidget);
	QComboBox *historyStyle = new QComboBox(historyWidget);
	historyStyle->addItem(QString("Solid Line"));
	historyStyle->addItem(QString("Dot Line"));
	historyStyle->addItem(QString("Dash Line"));
	historyStyle->addItem(QString("Dash-Dot Line"));
	historyStyle->addItem(QString("Dash-Dot-Dot Line"));
	historyStyle->setCurrentIndex(0);

	auto *historyStyleLabel = new QLabel("History Style",historyWidget);

	h2layout->addWidget(historyStyleLabel);
	h2layout->addWidget(historyStyle);
	historyLayout->addItem(h2layout);

	auto *historySection = new adiscope::gui::SubsectionSeparator("History", true,this);
	historySection->setContent(historyWidget);
	////end history

	////Peak holder
	auto *peakHolderWidget = new QWidget(this);
	auto *peakHolderLayout = new QHBoxLayout(peakHolderWidget);
	peakHolderSwitch = new CustomSwitch(peakHolderWidget);
	peakHolderSwitch->setChecked(true);
	peakHolderReset = new QPushButton("Reset",peakHolderWidget);
	peakHolderReset->setStyleSheet("QPushButton{"
										"height:30px;"
										"background-color: #4A64FF;"
										"border-radius: 4px;"
										"font-size: 12px;"
										"line-height: 14px;"
										"color: #FFFFFF;}"
										"QPushButton:hover{"
										"background-color: #4a34ff;"
										"}");

	peakHolderLayout->addWidget(peakHolderReset);
	peakHolderLayout->addWidget(peakHolderSwitch);



	auto *peakHolderSection = new adiscope::gui::SubsectionSeparator("Peak Holder", true,this);
	peakHolderSection->setContent(peakHolderWidget);
	///end peak holder


	scaleSection->getButton()->click();
	historySection->getButton()->click();
	peakHolderSection->getButton()->click();

	this->insertSection(scaleSection);
	this->insertSection(historySection);
	this->insertSection(peakHolderSection);

	this->layout()->addItem( new QSpacerItem(10,10, QSizePolicy::Fixed, QSizePolicy::Expanding));
	connect(scaleSWitch, &CustomSwitch::toggled, this, [=](bool toggled){
		Q_EMIT DataLoggerToolGenericMenu::toggleScale(toggled);
	});

	connect(historySwitch,  &CustomSwitch::toggled, this, [=](bool toggled){
		Q_EMIT DataLoggerToolGenericMenu::toggleHistory(toggled);
	});
	connect(historySize, QOverload<int>::of(&QComboBox::currentIndexChanged), this,[=](int index){
				setHistorySize(index);
			});
	connect(historyStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
		Q_EMIT lineStyleChanged(lineStyleFromIdx(index));
	});
	connect(peakHolderReset, &QPushButton::clicked,this, [=](){
		Q_EMIT DataLoggerToolGenericMenu::resetPeakHolder();
	});
	connect(peakHolderSwitch, &CustomSwitch::toggled, this, [=](bool toggled){
		Q_EMIT DataLoggerToolGenericMenu::togglePeakHolder(toggled);
	});
}

void DataLoggerToolGenericMenu::scaleToggle(bool toggled)
{
	scaleSWitch->setChecked(toggled);
}

void DataLoggerToolGenericMenu::peakHolderToggle(bool toggled)
{
	peakHolderSwitch->setChecked(toggled);
}

void DataLoggerToolGenericMenu::historyToggle(bool toggled)
{
	historySwitch->setChecked(toggled);
}

void DataLoggerToolGenericMenu::peakHolderResetClicked()
{
	peakHolderReset->click();
}

void DataLoggerToolGenericMenu::setHistorySize(int idx)
{
	Q_EMIT changeHistorySize(numSamplesFromIdx(idx));
}

double DataLoggerToolGenericMenu::numSamplesFromIdx(int idx)
{
	switch(idx) {
	case 0:	return 1.0;
	case 1:	return 10.0;
	case 2:	return 60.0;
	default:throw std::runtime_error("Invalid INDEX");
	}
}

Qt::PenStyle DataLoggerToolGenericMenu::lineStyleFromIdx(int idx)
{
	switch (idx) {
	case 0:	return Qt::PenStyle::SolidLine;
	case 1:	return Qt::PenStyle::DotLine;
	case 2:	return Qt::PenStyle::DashLine;
	case 3:	return Qt::PenStyle::DashDotLine;
	case 4: return Qt::PenStyle::DashDotDotLine;
	}
	return Qt::PenStyle::SolidLine;
}

