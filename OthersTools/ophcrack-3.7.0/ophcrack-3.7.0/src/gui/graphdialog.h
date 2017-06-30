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
#ifndef GRAPHDIALOG_H
#define GRAPHDIALOG_H

#include <QDialog>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_interval_data.h>

#include "histogram_item.h"
#include "ophstat.h"
#include "list.h"
#include "ui_graphdialog.h"

class GraphDialog : public QDialog, public Ui::GraphDialog {
Q_OBJECT

public:
  GraphDialog(QWidget *parent = 0);

  void plot(ophstat_t *stats, int crack_time);

private:
  void plotHistoLength(uint32_t *val, QwtPlot *plot);
  void plotHistoCategory(uint32_t *val, QwtPlot *plot);
  void plotCurve(list_t *list, int crack_time, QwtPlot *plot);

private:

  QwtArray<QwtDoubleInterval> lengthX;
  QwtArray<double> lengthY;
  HistogramItem *lengthHistogram;
  QwtPlotGrid *lengthGrid;

  QwtArray<QwtDoubleInterval> categoryX;
  QwtArray<double> categoryY;
  HistogramItem *categoryHistogram;
  QwtPlotGrid *categoryGrid;
  
  QwtArray<double> timeX;
  QwtArray<double> timeY;
  QwtPlotCurve *timeCurve;
  QwtPlotGrid *timeGrid;

};

#endif
