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
#include <QString>

#include "hashmodel.h"
#include "list.h"
#include "hash.h"
#include "misc.h"
//---------------------------------------------------------------------------
HashModel::HashModel(QObject *parent, bool x) : QStandardItemModel(0, 6, parent) {
  // Set the header.

  auditMode = x;
  if (auditMode) {
    setHorizontalHeaderItem(0, new QStandardItem("ID"));
    setHorizontalHeaderItem(1, new QStandardItem("Cracked"));
    setHorizontalHeaderItem(2, new QStandardItem("Time"));
    setHorizontalHeaderItem(3, new QStandardItem("Table"));
    setHorizontalHeaderItem(4, new QStandardItem("Length"));
    setHorizontalHeaderItem(5, new QStandardItem("Category"));
  } else {
    setHorizontalHeaderItem(0, new QStandardItem("User"));
    setHorizontalHeaderItem(1, new QStandardItem("LM Hash"));
    setHorizontalHeaderItem(2, new QStandardItem("NT Hash"));
    setHorizontalHeaderItem(3, new QStandardItem("LM Pwd 1"));
    setHorizontalHeaderItem(4, new QStandardItem("LM Pwd 2"));
    setHorizontalHeaderItem(5, new QStandardItem("NT Pwd"));
  }    

  for (int i=0; i<columnCount(); ++i) {
    QStandardItem *item = horizontalHeaderItem(i);
    item->setTextAlignment(Qt::AlignCenter);
  }
  
  unameVisible = 0;
}
//---------------------------------------------------------------------------
void HashModel::insertHash(hash_t *hsh) {
  assert(hsh != 0);

  int nrows = rowCount();
  int ncols = columnCount();
  int id = hsh->id;

  QStandardItem *item = itemFromId.value(id);

  if (item == 0) {
    for (int i=ncols-1; i>=0; --i) {
      item = new QStandardItem();
      item->setData(int(id), Qt::UserRole);
      setItem(nrows, i, item);
    }

    itemFromId.insert(id, item);
  }

  updateHash(hsh);
}
//---------------------------------------------------------------------------
void HashModel::insertHashes(list_t *hashes) {
  list_nd_t *nd;

  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    insertHash(hsh);
  }
}
//---------------------------------------------------------------------------
void HashModel::updateHash(hash_t *hsh, bool notFound) {
  int id = hsh->id;
  QStandardItem *root = itemFromId.value(id);

  if (root == 0) return;

  int row = indexFromItem(root).row();
  
  if (!auditMode) {
    
    // Set the user name.
    
    if (unameVisible)
      item(row, 0)->setText("*****");
    else
      item(row, 0)->setText(QString::fromUtf8(hsh->info));
    
    // Set the hash and password.

    int idx_txt = 0, idx_pwd = 0;
    
    switch (hsh->kind) {
    case lm1:
      idx_txt = 1;
      idx_pwd = 3;
      break;
      
    case lm2:
      idx_txt = 1;
      idx_pwd = 4;
      break;
      
    case nt:
      idx_txt = 2;
      idx_pwd = 5;
      break;
    }
    
    item(row, idx_txt)->setText(hsh->str);
    
    if (hsh->pwd[0] != 0) {
      item(row, idx_pwd)->setText(hsh->pwd);
      item(row, idx_pwd)->setForeground(QColor(0, 0, 0));
    }
    
    else if (hsh->done > 0) {
      item(row, idx_pwd)->setText("empty");
      item(row, idx_pwd)->setForeground(QColor(0, 0, 255));
    }
    
    else if (hsh->done <= 0) {
      if (notFound) {
	item(row, idx_pwd)->setText("not found");
	item(row, idx_pwd)->setForeground(QColor(255, 0, 0));
      } else {
	item(row, idx_pwd)->setText("");
	item(row, idx_pwd)->setForeground(QColor(0, 0, 0));
      }
    }
    
    // If the 1st LM hash is not connected to a NT hash, then we
    // display the NT password as empty.
    
    if (hsh->kind == lm1 && hsh->lmhsh1 == 0) {
      item(row, 5)->setText("empty");
      item(row, 5)->setForeground(QColor(0, 0, 255));
    }
  } else {

    //audit mode
    // Set the ID
    char tmp[64] = {0};
    
    sprintf(tmp, "%d", hsh->id);
    item(row, 0)->setText(tmp);

    item(row, 3)->setForeground(QColor(0, 0, 0));
    item(row, 4)->setForeground(QColor(0, 0, 0));
    item(row, 5)->setForeground(QColor(0, 0, 0));

    // Only LM1
    if (hsh->kind == lm1 && hsh->lmhsh1 == 0 && hsh->lmhsh2 == 0) {

      if (hsh->done > 0) {
	item(row, 1)->setText("Yes");
	
	sprintf(tmp, "%d s", hsh->time);
	item(row, 2)->setText(tmp);

	item(row, 3)->setText(hsh->table);

	sprintf(tmp, "%d", hsh->length);
	item(row, 4)->setText(tmp);

	item(row, 5)->setText(category_string(hsh->category));
      } else {
	item(row, 1)->setText("No");
	item(row, 2)->setText("");
	item(row, 3)->setText("");
	item(row, 4)->setText("");
	item(row, 5)->setText("");
      }
    } 
    // LM1 and LM2 with no NT
    else if (hsh->kind == lm1 && hsh->lmhsh1 == 0 && hsh->lmhsh2 != 0) {

      if (hsh->done > 0 && hsh->lmhsh2->done >0) {
	item(row, 1)->setText("Yes");
	sprintf(tmp, "%d s", (hsh->time > hsh->lmhsh2->time) ? hsh->time : hsh->lmhsh2->time);
	item(row,2)->setText(tmp);
	snprintf(tmp, 64, "%s / %s", hsh->table, hsh->lmhsh2->table);
	item(row,3)->setText(tmp);
	sprintf(tmp, "%d", hsh->length+hsh->lmhsh2->length);
	item(row,4)->setText(tmp);
	item(row,5)->setText(category_string(hsh->category|hsh->lmhsh2->category));
      } else {
	item(row, 1)->setText("No");
	item(row,2)->setText("");
	item(row,3)->setText("");
	item(row,4)->setText("");
	item(row,5)->setText("");
      }
    }
    // NT with LM1 and LM2
    else if (hsh->kind == nt && hsh->lmhsh1 != 0 && hsh->lmhsh2 != 0) {
      if (hsh->done > 0) {
	item(row, 1)->setText("Yes");
	sprintf(tmp, "%d s", hsh->time);
	item(row,2)->setText(tmp);
	if (hsh->lmhsh1->table != 0 && hsh->lmhsh2->table != 0)
	  snprintf(tmp, 64, "%s / %s", hsh->lmhsh1->table, hsh->lmhsh2->table);
	else 
	  snprintf(tmp, 64, "%s", hsh->table);
	item(row,3)->setText(tmp);
	sprintf(tmp, "%d", hsh->length);
	item(row,4)->setText(tmp);
	item(row,5)->setText(category_string(hsh->category));
      } else {
	item(row, 1)->setText("No");
	item(row,2)->setText("");
	item(row,3)->setText("");
	item(row,4)->setText("");
	item(row,5)->setText("");
      }
    }
    // NT only
    else if (hsh->kind == nt && hsh->lmhsh1 == 0 && hsh->lmhsh2 == 0) {
      if (hsh->done > 0) {
	item(row, 1)->setText("Yes");
	sprintf(tmp, "%d s", hsh->time);
	item(row,2)->setText(tmp);
	item(row,3)->setText(hsh->table);
	sprintf(tmp, "%d", hsh->length);
	item(row,4)->setText(tmp);
	item(row,5)->setText(category_string(hsh->category));
      } else {
	item(row, 1)->setText("No");
	item(row,2)->setText("");
	item(row,3)->setText("");
	item(row,4)->setText("");
	item(row,5)->setText("");
      }
    } 
  }  
}
//---------------------------------------------------------------------------
bool HashModel::removeRow(int row) {
  QStandardItem *root = item(row, 0);
  int id = root->data(Qt::UserRole).toInt();
  
  itemFromId.remove(id);

  return QStandardItemModel::removeRow(row);
}
//---------------------------------------------------------------------------
void HashModel::setUnameVisible(bool x) {
  unameVisible = x;
}

//---------------------------------------------------------------------------
void HashModel::setAuditMode(bool x) {
  auditMode = x;

  for (int i=0; i<rowCount(); ++i) 
    for (int j=0; j<columnCount(); ++j) 
      item(i, j)->setText("");


  if (auditMode) {
    setHorizontalHeaderItem(0, new QStandardItem("ID"));
    setHorizontalHeaderItem(1, new QStandardItem("Cracked"));
    setHorizontalHeaderItem(2, new QStandardItem("Time"));
    setHorizontalHeaderItem(3, new QStandardItem("Table"));
    setHorizontalHeaderItem(4, new QStandardItem("Length"));
    setHorizontalHeaderItem(5, new QStandardItem("Category"));
  } else {
    setHorizontalHeaderItem(0, new QStandardItem("User"));
    setHorizontalHeaderItem(1, new QStandardItem("LM Hash"));
    setHorizontalHeaderItem(2, new QStandardItem("NT Hash"));
    setHorizontalHeaderItem(3, new QStandardItem("LM Pwd 1"));
    setHorizontalHeaderItem(4, new QStandardItem("LM Pwd 2"));
    setHorizontalHeaderItem(5, new QStandardItem("NT Pwd"));
  }    
}
