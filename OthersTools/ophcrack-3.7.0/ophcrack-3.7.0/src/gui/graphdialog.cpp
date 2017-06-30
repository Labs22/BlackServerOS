/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2009 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
 *   
 *   Ophcrack is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   Ophcrack is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Ophcrack; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *   
 *   This program is released under the GPL with the additional exemption 
 *   that compiling, linking, and/or using OpenSSL is allowed.
 *   
 *   
 *   
 *   
*/
#include "graphdialog.h"
//---------------------------------------------------------------------------
GraphDialog::GraphDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);

  lengthHistogram = 0;
  categoryHistogram = 0;
  timeCurve = 0;
}
//---------------------------------------------------------------------------
void GraphDialog::plot(ophstat_t *stats, int crack_time) {
  
  this->plotHistoLength(stats->length, lengthPlot);
  this->plotHistoCategory(stats->category, categoryPlot);
  this->plotCurve(stats->time, crack_time, timePlot);
}
//---------------------------------------------------------------------------
void GraphDialog::plotHistoLength(uint32_t *val, QwtPlot *plot) {

  int i;
  const int numValues = 16;
  double pos = -0.5;
  const int width = 1;
  
  if (lengthHistogram == 0) {
    lengthX.resize(numValues);
    lengthY.resize(numValues);

    for (i=0; i<numValues; i++) {
      lengthX[i] = QwtDoubleInterval(pos, pos + double(width));
      pos += width;
    }
  }

  for ( i = 0; i < numValues; i++ )
    lengthY[i] = (double)val[i]; 
  
  if (lengthHistogram == 0) {
    
    plot->clear();
    plot->setCanvasBackground(QColor(Qt::white));

    lengthGrid = new QwtPlotGrid;
    lengthGrid->enableXMin(false);
    lengthGrid->enableX(false);
    lengthGrid->enableYMin(false);
    lengthGrid->enableY(true);
    lengthGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    lengthGrid->attach(plot);
 
    lengthHistogram = new HistogramItem();
    static const QColor green = QColor(143, 196, 66);
    lengthHistogram->setColor(green);

    lengthHistogram->attach(plot);

    plot->setAxisScale(QwtPlot::xBottom, -0.5, 15.5, 1);
    plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 0);
    
    plot->setAxisTitle(QwtPlot::xBottom, QString("Length"));
    plot->setAxisTitle(QwtPlot::yLeft, QString("Number of passwords"));
  }

  // finally, refresh the plot
  lengthHistogram->setData(QwtIntervalData(lengthX, lengthY));
  plot->replot();

  
}
//---------------------------------------------------------------------------
void GraphDialog::plotHistoCategory(uint32_t *val, QwtPlot *plot) {

  int i;
  const int numValues = 6;
  double pos = -0.5;
  const int width = 1;

  if (categoryHistogram == 0) {
    categoryX.resize(numValues);
    categoryY.resize(numValues);

    for (i=0; i<numValues; i++) {
      categoryX[i] = QwtDoubleInterval(pos, pos + double(width));
      pos += width;
    }
  }

  categoryY[0] = double(val[0]);
  categoryY[1] = double(val[1]+val[2]);
  categoryY[2] = double(val[5]+val[6]+val[4]);
  categoryY[3] = double(val[3]);
  categoryY[4] = double(val[7]);
  categoryY[5] = double(val[8]+val[9]+val[10]+val[11]+val[12]+val[13]+val[14]+val[15]);
  
  if (categoryHistogram == 0) {
    plot->clear();
    plot->setCanvasBackground(QColor(Qt::white));

    categoryGrid = new QwtPlotGrid;
    categoryGrid->enableXMin(false);
    categoryGrid->enableX(false);
    categoryGrid->enableYMin(false);
    categoryGrid->enableY(true);
    categoryGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    categoryGrid->attach(plot);
 
    categoryHistogram = new HistogramItem();
    static const QColor red   = QColor(169, 52, 52);
    categoryHistogram->setColor(red);
  
    categoryHistogram->attach(plot);

    plot->setAxisScale(QwtPlot::xBottom, -0.5, 5.5, 1);
    plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 0);

    plot->setAxisTitle(QwtPlot::xBottom, QString("Complexity category"));
    plot->setAxisTitle(QwtPlot::yLeft, QString("Number of passwords"));
  }

  // finally, refresh the plot
  categoryHistogram->setData(QwtIntervalData(categoryX, categoryY));
  plot->replot();

  
}
//---------------------------------------------------------------------------
void GraphDialog::plotCurve(list_t *list, int crack_time, QwtPlot *plot) {
  int i;
  list_nd_t *lnd;
  int hsh_time;

  int numValues = (128 > list->size) ? 128 : list->size;

  timeX.resize(numValues);
  timeY.resize(numValues);

  for (i=0; i<numValues; i++ ) {
    timeX[i] = i * crack_time / numValues;
    timeY[i] = 0.0;
  }
  
  if (crack_time == 0) crack_time = 1;

  for (lnd = list->head; lnd != 0; lnd = lnd->next) {
    hsh_time = *(int*)(lnd->data);
    i = (int)floor(hsh_time * numValues / crack_time);
    timeY[(i >= numValues) ? numValues-1 : i]+= 1.0;
  }  
  
  for (i=1; i<numValues; i++) 
    timeY[i] += timeY[i-1];

  if (timeCurve == 0) {
    plot->clear();
    plot->setCanvasBackground(QColor(Qt::white));

    lengthGrid = new QwtPlotGrid;
    lengthGrid->enableXMin(true);
    lengthGrid->enableX(true);
    lengthGrid->enableYMin(false);
    lengthGrid->enableY(true);
    lengthGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    lengthGrid->setMinPen(QPen(Qt::gray, 0, Qt::DotLine));
    lengthGrid->attach(plot);

    timeCurve = new QwtPlotCurve("Password found"); 
    timeCurve->setStyle(QwtPlotCurve::Steps);
    static const QColor red   = QColor(169, 52, 52);
    timeCurve->setPen(red);
    timeCurve->attach(plot);
  
    plot->setAxisTitle(QwtPlot::xBottom, QString("Crack time (s)"));
    plot->setAxisTitle(QwtPlot::yLeft, QString("Number of passwords"));
  }

  // finally, refresh the plot
  plot->setAxisScale(QwtPlot::yLeft, 0, timeY[numValues-1]+1.0);
  timeCurve->setData(timeX, timeY);
  plot->replot();

  
}
