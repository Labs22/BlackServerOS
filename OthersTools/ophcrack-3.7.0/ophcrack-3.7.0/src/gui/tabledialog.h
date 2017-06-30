/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2008 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
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
#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>
#include <QItemSelectionModel>

#include "ui_tabledialog.h"
#include "tablemodel.h"
#include "ophcrack.h"

class TableDialog : public QDialog, public Ui::TableDialog {
Q_OBJECT

public:
  TableDialog(TableModel *tableModel, QWidget *parent = 0);

signals:
  void install(QString path, bool replace, bool warn);

private:
  TableModel *tableModel;
  QItemSelectionModel *selectModel;

private slots:
  void swap(int row1, int row2);
  void moveUp(void);
  void moveDown(void);
  void enable(void);
  void disable(void);
  void chooseDir(void);
};

#endif
