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
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <assert.h>

#include "tabledialog.h"
#include "table.h"
//---------------------------------------------------------------------------
TableDialog::TableDialog(TableModel *tableModel, QWidget *parent) : 
  QDialog(parent) {
  
  // Setup the dialog.

  setupUi(this);
  this->tableModel = tableModel;

  // Setup the table view.

  tableView->setModel(tableModel);
  tableView->hideColumn(TableModel::PROGRESS_COL_IDX);
  selectModel = tableView->selectionModel();

  QHeaderView *header = tableView->header();
  int len = header->length();

  header->resizeSection(0, 4*len/10);
  header->resizeSection(1, 6*len/10);

  // ...

  connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));
  connect(enableButton, SIGNAL(clicked()), this, SLOT(enable()));
  connect(disableButton, SIGNAL(clicked()), this, SLOT(disable()));
  connect(installButton, SIGNAL(clicked()), this, SLOT(chooseDir()));
}
//---------------------------------------------------------------------------
void TableDialog::swap(int row1, int row2) {
  if (row1 == row2) return;

  int max_row = row1 > row2 ? row1 : row2;
  int min_row = row1 < row2 ? row1 : row2;

  // Take the elements at max_row first because otherwise the row
  // index changes.

  QList<QStandardItem*> max_items = tableModel->takeRow(max_row);
  QList<QStandardItem*> min_items = tableModel->takeRow(min_row);

  // Insert the elements at min_row first because otherwise the index
  // is not correct.

  tableModel->insertRow(min_row, max_items);
  tableModel->insertRow(max_row, min_items);
}
//---------------------------------------------------------------------------
void TableDialog::moveUp(void) {
  QList<QModelIndex> indexes = selectModel->selectedRows();
  
  if (indexes.isEmpty()) return;

  // Retrieve the row of the last selected item.

  QModelIndex index = indexes.last();
  QStandardItem *item = tableModel->itemFromIndex(index);
  int row = index.row();

  if (row == 0 || item->parent() != 0) return;
  selectModel->clearSelection();

  // Record whether the items where expanded or not.

  QModelIndex index_prev = tableModel->index(row-1, TableModel::TABLE_COL_IDX);
  QModelIndex index_curr = tableModel->index(row, TableModel::TABLE_COL_IDX);
  
  bool expand_prev = tableView->isExpanded(index_prev);
  bool expand_curr = tableView->isExpanded(index_curr);

  tableView->setExpanded(index_prev, false);
  tableView->setExpanded(index_curr, false);

  swap(row-1, row);

  selectModel->select(index_prev, 
		      QItemSelectionModel::Select | 
		      QItemSelectionModel::Rows);

  tableView->setExpanded(index_curr, expand_prev);
  tableView->setExpanded(index_prev, expand_curr);
}
//---------------------------------------------------------------------------
void TableDialog::moveDown(void) {
  QList<QModelIndex> indexes = selectModel->selectedRows();
  
  if (indexes.isEmpty()) return;

  // Retrieve the row of the last selected item.

  QModelIndex index = indexes.last();
  QStandardItem *item = tableModel->itemFromIndex(index);
  int row = index.row();

  if (row == tableModel->rowCount()-1 || item->parent() != 0) return;
  selectModel->clearSelection();

  // Record whether the items where expanded or not.

  QModelIndex index_curr = tableModel->index(row, 0);
  QModelIndex index_next = tableModel->index(row+1, 0); 
  
  bool expand_curr = tableView->isExpanded(index_curr);
  bool expand_next = tableView->isExpanded(index_next);

  tableView->setExpanded(index_next, false);
  tableView->setExpanded(index_curr, false);

  swap(row, row+1);

  selectModel->select(index_next, 
		      QItemSelectionModel::Select | 
		      QItemSelectionModel::Rows);

  tableView->setExpanded(index_curr, expand_next);
  tableView->setExpanded(index_next, expand_curr);
}
//---------------------------------------------------------------------------
void TableDialog::enable(void) {
  QList<QModelIndex> indexes = selectModel->selectedRows();

  if (indexes.isEmpty()) return;

  while (!indexes.isEmpty()) {
    QModelIndex index = indexes.takeFirst();
    QStandardItem *item = tableModel->itemFromIndex(index);
 
    tableModel->enable(item, true);
  }
}
//---------------------------------------------------------------------------
void TableDialog::disable(void) {
  QList<QModelIndex> indexes = selectModel->selectedRows();

  if (indexes.isEmpty()) return;

  while (!indexes.isEmpty()) {
    QModelIndex index = indexes.takeFirst();
    QStandardItem *item = tableModel->itemFromIndex(index);
    
    tableModel->enable(item, false);
  }  
}
//---------------------------------------------------------------------------
void TableDialog::chooseDir(void) {
  QString path = QFileDialog::getExistingDirectory(this, 
						   tr("Select the directory which contains the tables."), 
						   QString(), 
						   QFileDialog::ShowDirsOnly);
  
  if (!path.isNull()) emit install(path, true, true);
}
