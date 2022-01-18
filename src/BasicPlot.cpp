#include "BasicPlot.h"
#include <QwtPlotOpenGLCanvas>
#include <QwtPainter>
#include <QDebug>

namespace adiscope {

static int staticPlotId = 0;

#define replotFrameDuration 1000.0/replotFrameRate

bool useOpenGlCanvas = 0;
BasicPlot::BasicPlot(QWidget* parent) : QwtPlot(parent), started(false), replotFrameRate(60)
{
	auto openGLEnvVar = qgetenv("SCOPY_USE_OPENGL");
	useOpenGlCanvas = (bool)openGLEnvVar.toInt();

	connect(&replotTimer,SIGNAL(timeout()),this,SLOT(replotNow()));

	if(useOpenGlCanvas) {
		QwtPlotOpenGLCanvas* plotCanvas = qobject_cast< QwtPlotOpenGLCanvas* >( canvas() );
		if ( plotCanvas == NULL )
		{
			plotCanvas = new QwtPlotOpenGLCanvas(this);
			plotCanvas->setPaintAttribute(QwtPlotAbstractGLCanvas::BackingStore );
#ifdef IMMEDIATE_PAINT
			plotCanvas->setPaintAttribute(QwtPlotAbstractGLCanvas::ImmediatePaint, true);
#endif
			setCanvas( plotCanvas );
		} else {
			;
		}
	} else {
		QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( canvas() );
#ifdef IMMEDIATE_PAINT
		plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
#endif
	}

	qDebug(CAT_PLOT)<<QString::number(id)<<"Created plot";
	id = staticPlotId;
	staticPlotId++; // for debug
	pfps.setCapacity(fpsHistoryCount);
	ifps.setCapacity(fpsHistoryCount);
	fpsLabel.attach(this);
	QFont font;
	font.setBold( true );
	fpsTxt.setFont( font );
	fpsTxt.setRenderFlags( Qt::AlignRight | Qt::AlignTop );
	fpsTxt.setColor(QColor("red"));
	replotNow();
}

void BasicPlot::startStop(bool en) {
	if(en) {
		start();
	} else  {
		stop();
	}
}

void BasicPlot::start() {
	if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"Starting freerunning plot - framerate - " << replotFrameRate;
	pfps.clearHistory();
	ifps.clearHistory();
	ims.clearHistory();
	pms.clearHistory();

	started = true;
}

bool BasicPlot::isStarted() {
	return started;
}
void BasicPlot::stop() {
	if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"FreeRunningPlot - Stopping freerunning plot - will force one replot";
	started = false;
	replotTimer.stop();
	replotNow();
}
void BasicPlot::setRefreshRate(double hz) {
	replotFrameRate = hz;
	if(replotTimer.isActive())	{
		replotTimer.start(replotFrameRate);
	}
}

void BasicPlot::setVisibleFpsLabel(bool vis) {
	fpsLabel.setVisible(vis);
}

void BasicPlot::hideFpsLabel() {
	fpsLabel.hide();
	debug = false;
}
void BasicPlot::showFpsLabel() {
	fpsLabel.show();
	debug = true;
}

double BasicPlot::getRefreshRate() {
	return replotFrameRate;

}
void BasicPlot::replotNow() {
	auto instrumentCycle = fpsTimer.nsecsElapsed() / 1e+06;	;
	fpsTimer.start();

	QwtPlot::replot();

	auto replotDuration = fpsTimer.nsecsElapsed() / 1e+06;
	fpsTimer.restart();

	// -------- COMPUTE TIMINGS --------
	instrumentCycle = instrumentCycle + replotDuration;
	auto instrumentFps = 1000.0/(double)(instrumentCycle);

	auto instrumentCycleAvg = ims.pushValueReturnAverage(instrumentCycle);
	auto instrumentFpsAvg = ifps.pushValueReturnAverage(instrumentFps);
	auto replotTheoreticalFpsAvg = pfps.pushValueReturnAverage(1000.0/replotDuration);
	auto replotDurationAvg = pms.pushValueReturnAverage(replotDuration);

	if(debug) qDebug(CAT_PLOT)	<< QString::number(id) << " - FreeRunningPlot - drawing plot - "
				<< instrumentFpsAvg <<" fps - "
				<< instrumentCycleAvg << " ms"
#ifdef IMMEDIATE_PAINT
				<< replotTheoreticalFpsAvg << " fps "
				<< replotDurationAvg << "ms avg frame time"
#endif
				;


	QString rendering = (useOpenGlCanvas) ? "OpenGl rendering" : "Software rendering";
	fpsTxt.setText(rendering + "\n" +
					  "instrument: " + QString::number(instrumentFpsAvg) + " fps / " + QString::number(instrumentCycleAvg) + " ms" + "\n"
#ifdef IMMEDIATE_PAINT
					+ "plot: " + QString::number(replotTheoreticalFpsAvg) + "fps / " + QString::number(replotDurationAvg) + " ms"
#endif
					);
	fpsLabel.setText(fpsTxt);

}


void BasicPlot::replot() {
#ifdef IMMEDIATE_PAINT
		if(!replotTimer.isActive()) {
			//qDebug(CAT_PLOT)<<QString::number(id)<<"FreeRunningPlot - freerunning replot - schedule next frame";
			replotTimer.setSingleShot(true);
			replotTimer.start(1000.0/replotFrameRate);
		}
		else
		{
			if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"FreeRunningPlot - freerunning replot - already scheduled";
		}
#else
	replotNow();
#endif
}

} // adiscope
