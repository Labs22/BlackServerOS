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
#include <assert.h>
#include <math.h>

#include "tablemodel.h"
//---------------------------------------------------------------------------
TableModel::TableModel(QObject *parent) : QStandardItemModel(0, 5, parent) {
  // Set the header.

  setHorizontalHeaderItem(TABLE_COL_IDX,    new QStandardItem("Table"));
  setHorizontalHeaderItem(DIR_COL_IDX,      new QStandardItem("Directory"));
  setHorizontalHeaderItem(STATUS_COL_IDX,   new QStandardItem("Status"));
  setHorizontalHeaderItem(PRELOAD_COL_IDX,  new QStandardItem("Preload"));
  setHorizontalHeaderItem(PROGRESS_COL_IDX, new QStandardItem("Progress"));

  for (int i=0; i<columnCount(); ++i) {
    QStandardItem *item = horizontalHeaderItem(i);
    item->setTextAlignment(Qt::AlignCenter);
  }
}
//---------------------------------------------------------------------------
void TableModel::install(list_t *tables) {
  list_nd_t *nd;
  
  for (nd = tables->head; nd != 0; nd = nd->next) {
    table_t *tbl = (table_t*)nd->data;
    insertTable(tbl);
  }
}
//---------------------------------------------------------------------------
void TableModel::uninstall(table_kind_t kind) {
  // If this kind of table is not installed, then we do not have
  // anything to do.

  if (!isInstalled(kind)) return;
    
  // Otherwise, we free the memory occupied by the tables and remove
  // the corresponding items.

  QStandardItem *root = getRoot(kind);

  int ntables = root->rowCount();
  int i;

  for (i=0; i<ntables; ++i) {
    QStandardItem *child = root->child(0, TABLE_COL_IDX);
    table_t *tbl = (table_t*)child->data(Qt::UserRole).value<void*>();

    table_free(tbl);
    root->removeRow(0);
  }

  // Set the user role to -1 to indicate that the table are not
  // installed.

  root->setData(int(-1), Qt::UserRole);
}
//---------------------------------------------------------------------------
QStandardItem *TableModel::insertRoot(table_kind_t kind) {
  QStandardItem *item = itemFromKind.value(kind);

  // If there are no items with the given name, then we create one.

  if (item == 0) {
    const char *name = table_string(kind);
    int nrows = rowCount();
    int ncols = columnCount();

    QStandardItemModel::insertRow(nrows);
    
    for (int i=ncols-1; i >= 0; --i) {
      item = new QStandardItem();
      setItem(nrows, i, item);
    }

    item->setText(name);

    itemFromKind.insert(kind, item);
    updateRootItem(item);
  }

  return item;
}
//---------------------------------------------------------------------------
QStandardItem *TableModel::insertTable(table_t *tbl) {
  assert(tbl != 0);

  QStandardItem *root = insertRoot(tbl->kind);
  QStandardItem *child = 0;

  QVariant data = QVariant::fromValue((void*)tbl);
  int ncols = columnCount();
  
  for (int i=ncols-1; i >= 0; --i) {
    child = new QStandardItem();
    root->setChild(tbl->idx, i, child);
  }
  
  child->setData(data, Qt::UserRole);
  updateTable(tbl);

  return child;
}
//---------------------------------------------------------------------------
void TableModel::enable(QStandardItem *item, bool value) {
  // If we deal with a top level item, then we enable all the children.

  if (item->parent() == 0) {
    int n = item->rowCount();

    for (int i=0; i<n; ++i)
      enable(item->child(i, TABLE_COL_IDX), value);
  }

  // Otherwise, we enable the corresponding table.

  else {
    table_t *tbl = tableFromItem(item);
    assert(tbl != 0);

    tbl->enabled = value;    
    tbl->active  = tbl->enabled == 0 ? 0 : tbl->active;

    updateTable(tbl);
 }
}
//---------------------------------------------------------------------------
void TableModel::updateTable(table_t *tbl) {
  int idx = tbl->idx;

  QStandardItem *root = itemFromKind.value(tbl->kind);
  assert(root != 0);
  QStandardItem *child = root->child(idx);

  updateChildItem(child);
  updateRootItem(root);
}
//---------------------------------------------------------------------------
void TableModel::updateItem(QStandardItem *item) {
  if (item->parent() == 0)
    updateRootItem(item);
  else
    updateChildItem(item);
}
//---------------------------------------------------------------------------
table_t *TableModel::tableFromItem(QStandardItem *item) {
  if (item->parent() == 0)
    return 0;
  else
    return (table_t*)item->data(Qt::UserRole).value<void*>();
}
//---------------------------------------------------------------------------
bool TableModel::isInstalled(QStandardItem *item) {
  QStandardItem *root = item->parent();

  if (root != 0) 
    return isInstalled(root);
  else {
    int nenabled = item->data(Qt::UserRole).toInt();
    if (nenabled < 0) return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool TableModel::isInstalled(table_kind_t kind) {
  QStandardItem *item = itemFromKind.value(kind);

  if (item == 0 || item->data(Qt::UserRole).toInt() == -1) 
    return false;
  else
    return true;
}
//---------------------------------------------------------------------------
QStandardItem *TableModel::getRoot(table_kind_t kind) {
  return itemFromKind.value(kind);
}
//---------------------------------------------------------------------------
void TableModel::insertRow(int row, const QList<QStandardItem*> &items) {
  QStandardItemModel::insertRow(row, items);

  QStandardItem *root = item(row, TABLE_COL_IDX);
  int nchildren = root->rowCount();

  for (int i=0; i<nchildren; ++i) {
    QStandardItem *child = root->child(i);
    emit itemChanged(child);
  }

  emit itemChanged(root);
}
//---------------------------------------------------------------------------
void TableModel::updateRootItem(QStandardItem *root) {
  assert(root->parent() == 0);

  int row = indexFromItem(root).row();
  int nchildren = root->rowCount();
  table_t *tbl = 0;

  if (nchildren > 0)
    tbl = tableFromItem(root->child(0, TABLE_COL_IDX));
  
  // Set the first column of the root item.

  if (tbl != 0) root->setText(QString(tbl->name));

  // Determine if this group of tables is enabled.

  uint64_t size  = 0;
  uint64_t psize = 0;
  int nenabled = 0;
  int nactive  = 0;

  for (int i=0; i<nchildren; ++i) {
    QStandardItem *child = root->child(i, TABLE_COL_IDX);
    table_t *tbl = tableFromItem(child);
    table_preload_t state = table_preload_state(tbl);

    if (tbl->enabled) ++nenabled;
    if (tbl->active)  ++nactive;

    size  += table_size(tbl) >> 20;
    psize += table_preload_size(tbl, state) >> 20;
  }

  // Store the number of children enabled into the root or -1 if the
  // item has no children, i.e., the table is not installed.

  if (nchildren > 0)
    root->setData(int(nenabled), Qt::UserRole); 
  else
    root->setData(int(-1), Qt::UserRole);
  
  if (nchildren == 0)
    root->setIcon(QIcon(":/icons/pixmaps/notinstalled.png"));
  else if (nenabled > 0)
    root->setIcon(QIcon(":/icons/pixmaps/enabled.png"));
  else
    root->setIcon(QIcon(":/icons/pixmaps/disabled.png"));
  
  // Set the path to the tables.

  root = item(row, DIR_COL_IDX);
  if (tbl != 0) root->setText(QString(tbl->path));

  // Set the status of the tables.

  root = item(row, STATUS_COL_IDX);
  root->setTextAlignment(Qt::AlignCenter);

  if (nchildren == 0)
    root->setText(QString("not installed"));

  else if (nenabled > 0)
    if (nactive > 0)
      root->setText("active");
    else
      root->setText("inactive");

  else
    root->setText("disabled");

  // Set the preload state.

  root = item(row, PRELOAD_COL_IDX);
  root->setTextAlignment(Qt::AlignCenter);

  if (psize > 0) {
    int ratio = (int)ceil(100.*psize / size);
    QString str = QString("%1% in RAM").arg(ratio);
    root->setText(str);
  }
  else
    root->setText("on disk");
}
//---------------------------------------------------------------------------
void TableModel::updateChildItem(QStandardItem *child) {
  QStandardItem *root = child->parent(); 
  assert(root != 0);
  table_t *tbl = tableFromItem(child);

  assert(tbl != 0);
  int idx = tbl->idx;

  // Set the table name and icon.

  child->setText(QString("table%1").arg(idx));

  if (tbl->enabled)
    child->setIcon(QIcon(":/icons/pixmaps/enabled.png"));
  else
    child->setIcon(QIcon(":/icons/pixmaps/disabled.png"));

  // Set the table status.

  child = root->child(idx, STATUS_COL_IDX);
  child->setTextAlignment(Qt::AlignCenter);

  if (tbl->active)
    child->setText("active");

  else if (tbl->enabled)
    child->setText("inactive");

  else
    child->setText("disabled");

  // Set the preload state.

  child = root->child(idx, PRELOAD_COL_IDX);
  child->setTextAlignment(Qt::AlignCenter);

  table_preload_t state = table_preload_state(tbl);

  uint64_t size  = table_size(tbl) >> 20;
  uint64_t psize = table_preload_size(tbl, state) >> 20;

  if (psize > 0) {
    int ratio = (int)ceil(100.*psize / size);
    QString str = QString("%1% in RAM").arg(ratio);
    child->setText(str);
  }
  else
    child->setText("on disk");
}
//---------------------------------------------------------------------------
void TableModel::updateProgress(void) {
  int nrows = rowCount();

  int count = 0;
  int total_cmin = 0;
  int total_cmax = 0;

  for (int i=0; i<nrows; ++i) {
    QStandardItem *root = item(i, TABLE_COL_IDX);

    int nenabled  = root->data(Qt::UserRole).toInt();
    int nchildren = root->rowCount();
    int mean_cmin = 0;
    int mean_cmax = 0;

    if (nenabled <= 0) continue;

    for (int j=0; j<nchildren; ++j) {
      QStandardItem *child = root->child(j, TABLE_COL_IDX);
      table_t *tbl = tableFromItem(child);

      assert(tbl != 0);
      if (!tbl->enabled) continue;

      uint64_t ncols = tbl->ncols;
      uint64_t cmin  = tbl->cmin;
      uint64_t cmax  = tbl->cmax;

      int ratio_cmin = 0;
      int ratio_cmax = 0;

      if (ncols > 0) {
        uint64_t n = ncols * ncols;
        uint64_t dcmin = ncols-cmin;
        uint64_t dcmax = ncols-cmax;

        ratio_cmin = (1000*dcmin*dcmin) / n;
        ratio_cmax = (1000*dcmax*dcmax) / n;
      }

      QPoint pt = QPoint(ratio_cmin, ratio_cmax);

      child = root->child(j, PROGRESS_COL_IDX);
      child->setData(pt, Qt::UserRole);

      mean_cmin += ratio_cmin;
      mean_cmax += ratio_cmax;
    }

    count += nenabled;

    total_cmin += mean_cmin;
    total_cmax += mean_cmax;

    mean_cmin /= nenabled;
    mean_cmax /= nenabled;

    QPoint pt = QPoint(mean_cmin, mean_cmax);

    root = item(i, PROGRESS_COL_IDX);
    root->setData(pt, Qt::UserRole);
  }

  if (count > 0) {
    total_cmin /= count;
    total_cmax /= count;
  }

  QPoint pt = QPoint(total_cmin, total_cmax);

  QStandardItem *root = invisibleRootItem();
  root->setData(pt, Qt::UserRole);
}
