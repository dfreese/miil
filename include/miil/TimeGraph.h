#ifndef TIMEGRAPH_H
#define TIMEGRAPH_H

#include <qcustomplot.h>
#include <QVector>

class TimeGraph: public QCustomPlot {
    Q_OBJECT
public:
    TimeGraph(int number_of_channels = 1,
              int max_number_graph_points = 10000,
              QWidget * parent = 0);
    ~TimeGraph();

private:
    void scaleGraphWithPad(
            QCustomPlot * plot,
            float xpad=0.0,
            float ypad=0.0);

    int no_channels;
    int max_no_points;
    QVector<QVector<double> > x;
    QVector<QVector<double> > y;
    static QColor GraphColors[8];

public slots:
    void refresh();
    void setPoint(double x_pt, double y_pt, int channelNumber);
    void setPoint(double y_pt, int channelNumber);
    void clearPlot();
    void setMaxPoints(int max_number_graph_points);
};

#endif /* TIMEGRAPH_H */
