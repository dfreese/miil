#include <miil/TimeGraph.h>
#include <miil/util.h>

QColor TimeGraph::GraphColors[8] = {
    QColor::fromHsv(0,255,255),
    QColor::fromHsv(36,255,255),
    QColor::fromHsv(72,255,255),
    QColor::fromHsv(144,255,255),
    QColor::fromHsv(180,255,255),
    QColor::fromHsv(225,255,255),
    QColor::fromHsv(270,255,255),
    QColor::fromHsv(315,255,255)
};

TimeGraph::TimeGraph(
        int number_of_channels,
        int max_number_graph_points,
        QWidget * parent) :
    QCustomPlot(parent),
    no_channels(number_of_channels),
    max_no_points(max_number_graph_points)
{
    for (int i = 0; i < no_channels; i++) {
        this->addGraph();
        this->graph(i)->setName("Channel " + QString::number(i));
        this->graph(i)->setPen(QPen(QBrush(GraphColors[i]),3));
        x.push_back(QVector<double>());
        y.push_back(QVector<double>());
    }

    this->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    this->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    this->xAxis->setDateTimeFormat("hh:mm:ss");
    this->xAxis->setLabel("Time");
    this->yAxis->setLabel("Current (A)");
    this->legend->setVisible(true);
    this->setBackground(QBrush(QColor(Qt::lightGray)));
    this->axisRect()->insetLayout()->setInsetAlignment(
            0, Qt::AlignTop|Qt::AlignLeft);
}

TimeGraph::~TimeGraph() {
}

void TimeGraph::refresh() {
    if (this->isVisible()) {
        for (int i=0; i<no_channels; i++) {
            this->graph(i)->setData(x[i],y[i]);
        }
        scaleGraphWithPad(this,0.0,0.1);
        this->replot();
    }
}

void TimeGraph::setPoint(double x_pt, double y_pt, int channelNumber) {
    x[channelNumber].push_back(x_pt);
    y[channelNumber].push_back(y_pt);
    while(x[channelNumber].size() > max_no_points) {
        x[channelNumber].pop_front();
    }
    while(y[channelNumber].size() > max_no_points) {
        y[channelNumber].pop_front();
    }
}

void TimeGraph::setPoint(double y_pt, int channelNumber) {
    setPoint(Util::GetTimeOfDay(),y_pt,channelNumber);
}

void TimeGraph::scaleGraphWithPad(QCustomPlot * plot, float xpad, float ypad) {
    if (plot->graphCount() > 0) {
        // Store the range to see if it changes during the rescale.
        // If not, don't pad it, to keep it from growing
        QCPRange oldxrange = plot->xAxis->range();
        QCPRange oldyrange = plot->yAxis->range();

        for(int i=0; i<plot->graphCount(); i++) {
            plot->graph(i)->rescaleAxes(false);
        }
        for(int i=0; i<plot->graphCount(); i++) {
            plot->graph(i)->rescaleAxes(true);
        }

        QCPRange xrange = plot->xAxis->range();
        if ((oldxrange.lower != xrange.lower) &&
            (oldxrange.upper != xrange.upper))
        {
            xrange.lower -= xpad*(xrange.upper-xrange.lower);
            xrange.upper += xpad*(xrange.upper-xrange.lower);
            plot->xAxis->setRange(xrange);
            oldxrange = xrange;
        }

        QCPRange yrange = plot->yAxis->range();
        if ((oldyrange.lower != yrange.lower) &&
            (oldyrange.upper != yrange.upper))
        {
            yrange.lower -= ypad*(yrange.upper-yrange.lower);
            yrange.upper += ypad*(yrange.upper-yrange.lower);
            plot->yAxis->setRange(yrange);
            oldyrange = yrange;
        }
    }
}

void TimeGraph::clearPlot() {
    for (int i = 0; i < no_channels; i++) {
        x[i].clear();
        y[i].clear();
    }
    refresh();
}

void TimeGraph::setMaxPoints(int max_number_graph_points) {
    max_no_points = max_number_graph_points;
}
