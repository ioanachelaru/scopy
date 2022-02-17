/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sismograph.hpp"

#include <QPen>
#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>
#include "plot_utils.hpp"


#include <math.h>

using namespace adiscope;

Sismograph::Sismograph(QWidget *parent) : QwtPlot(parent)
  , curve("data"), sampleRate(10)
  , m_currentScale(-Q_INFINITY)
  , m_currentMaxValue(-Q_INFINITY)
{
	enableAxis(QwtPlot::xBottom, false);
	enableAxis(QwtPlot::xTop, true);

	setAxisTitle(QwtPlot::xTop, tr("Voltage (V)"));
	setAxisTitle(QwtPlot::yLeft, tr("Time (s)"));

	setAxisAutoScale(QwtPlot::yLeft, false);

	setAxisAutoScale(QwtPlot::xTop, false);
	setAxisScale(QwtPlot::xTop, -0.1, +0.1);


	QVector<QwtScaleDiv> divs;
	QwtScaleEngine *engine = axisScaleEngine(QwtPlot::xTop);
	divs.push_back(engine->divideScale(-0.1, +0.1, 5, 5));
    //divs.push_back(engine->divideScale(-1.0, +1.0, 5, 5));
    //divs.push_back(engine->divideScale(-5.0, +5.0, 10, 2));
    //divs.push_back(engine->divideScale(-25.0, +25.0, 10, 5));

	scaler = new AutoScaler(this, divs);

	connect(scaler, SIGNAL(updateScale(const QwtScaleDiv)),
			this, SLOT(updateScale(const QwtScaleDiv)));

	//setNumSamples(100);

	plotLayout()->setAlignCanvasToScales(true);
    scaleLabel = new CustomQwtScaleDraw();
    scaleLabel->setUnitOfMeasure(m_unitOfMeasureSymbol);
    this->setAxisScaleDraw(QwtPlot::xTop,scaleLabel);
	curve.attach(this);
	curve.setXAxis(QwtPlot::xTop);
}

Sismograph::~Sismograph()
{
	delete scaler;
}

void Sismograph::plot(double sample)
{
	xdata.push_back(sample);

	if (xdata.size() == numSamples + 2){
		double aux = xdata.first();

		if(aux == m_currentMaxValue){
			updateScale();
		}
			xdata.pop_front();
    }

	if(sample > m_currentMaxValue){
		updateScale();
	}

	scaler->setValue(sample);
	curve.setRawSamples(xdata.data(), ydata.data() + (ydata.size() -
				xdata.size()), xdata.size());
	replot();
}

double Sismograph::findMaxInFifo()
{
	double max = -Q_INFINITY;
	for(int i = 0 ; i < xdata.size(); i++){
		if(xdata.at(i) > max){
			max = xdata.at(i);
		}
	}
	return max;
}

void Sismograph::updateScale(){
	double sample = findMaxInFifo();
	/// compute scale
	int digits = 0;
	double num = sample;
	if(num != 0){
	if(int(num) == 0) {
		while((int)num*10 == 0){
			num *= 10;
			digits++;
		}
		digits = -1 * (digits - 1);
	}else{

		while((int)num){
			num /= 10;
			digits++;
		}
	}
	}
	double scale = pow(10 , digits);

	////update scale
	MetricPrefixFormatter m_prefixFormater;
	QString formatedPrefix = m_prefixFormater.getFormatedMeasureUnit(sample);
	setPlotAxisXTitle(formatedPrefix + m_unitOfMeasureName + "(" + formatedPrefix + m_unitOfMeasureSymbol + ")");
	scaleLabel->setUnitOfMeasure(m_unitOfMeasureSymbol);

	QwtScaleEngine *scaleEngine = axisScaleEngine(QwtPlot::xTop);
	updateScale(scaleEngine->divideScale((-1*scale),scale,5,10));
	m_currentScale = scale;
}

int Sismograph::getNumSamples() const
{
	return numSamples;
}

void Sismograph::updateYScale(double max, double min)
{

	reset();
	ydata.resize(numSamples + 1);
	xdata.reserve(numSamples + 1);
	setSampleRate(sampleRate);

	setAxisScale(QwtPlot::yLeft, min,max);
	replot();
}

void Sismograph::setHistoryDuration(double time)
{
	numSamples = (unsigned int) time * sampleRate;
	if(numSamples == 0)
		setNumSamples(time);
	else
		setNumSamples(numSamples);
}

void Sismograph::setNumSamples(int num)
{
	numSamples = (unsigned int) num;
	reset();
	ydata.resize(numSamples + 1);
	xdata.reserve(numSamples + 1);
	setAxisScale(QwtPlot::yLeft,numSamples / sampleRate, 0.0);
	setSampleRate(sampleRate);
	replot();
	scaler->setTimeout((double) numSamples * 1000.0 / sampleRate);
}

double Sismograph::getSampleRate() const
{
	return sampleRate;
}

void Sismograph::setSampleRate(double rate)
{
	sampleRate = rate;

	for (unsigned int i = 0; i <= numSamples; i++)
		ydata[i] = (double)(numSamples - i) / sampleRate;
}

void Sismograph::reset()
{
	xdata.clear();
	scaler->startTimer();
}

void Sismograph::setColor(const QColor& color)
{
	QPen pen(curve.pen());
	pen.setColor(color);
	curve.setPen(pen);
	replot();
}

void Sismograph::updateScale(const QwtScaleDiv div)
{
	setAxisScale(QwtPlot::xTop, div.lowerBound(), div.upperBound());
    setAxisScaleDraw(QwtPlot::xTop,scaleLabel);
}

void Sismograph::setLineWidth(qreal width)
{
        QPen pen(curve.pen());
        pen.setWidthF(width);
        curve.setPen(QPen(pen));
}

void Sismograph::setLineStyle(Qt::PenStyle lineStyle){
	QPen pen(curve.pen());
	pen.setStyle(lineStyle);
	curve.setPen(QPen(pen));
	replot();
}

void Sismograph::setUnitOfMeasure(QString unitOfMeasureName,QString unitOfMeasureSymbol){
    m_unitOfMeasureName = unitOfMeasureName;
    m_unitOfMeasureSymbol = unitOfMeasureSymbol;
}

void Sismograph::setPlotAxisXTitle(QString title){
    setAxisTitle(QwtPlot::xTop, title);
}
