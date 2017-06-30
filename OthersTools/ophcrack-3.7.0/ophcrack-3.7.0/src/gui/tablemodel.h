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
#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QMap>

#include "table.h"

class TableModel : public QStandardItemModel {
Q_OBJECT

public:
  TableModel(QObject *parent);

  static const int TABLE_COL_IDX    = 0;
  static const int DIR_COL_IDX      = 1;
  static const int STATUS_COL_IDX   = 2;
  static const int PRELOAD_COL_IDX  = 3;
  static const int PROGRESS_COL_IDX = 4;

  void install(list_t *tables);
  void uninstall(table_kind_t kind);
  void enable(QStandardItem *item, bool value);
  void updateTable(table_t *tbl);
  void updateItem(QStandardItem *item);
  void updateProgress(void);

  table_t *tableFromItem(QStandardItem *item);
  bool isInstalled(QStandardItem *item);
  bool isInstalled(table_kind_t kind);

  QStandardItem *insertRoot(table_kind_t kind);
  QStandardItem *insertTable(table_t *tbl);

  QStandardItem *getRoot(table_kind_t kind);
  void insertRow(int row, const QList<QStandardItem*> &items);

private:
  void updateRootItem(QStandardItem *item);
  void updateChildItem(QStandardItem *item);

  QMap<table_kind_t, QStandardItem*> itemFromKind;
};

#endif
