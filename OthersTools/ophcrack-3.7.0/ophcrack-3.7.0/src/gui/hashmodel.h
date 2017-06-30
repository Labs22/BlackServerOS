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
#ifndef HASHMODEL_H
#define HASHMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QMap>

#include "hash.h"

class HashModel : public QStandardItemModel {
Q_OBJECT

public:
  HashModel(QObject *parent, bool x);

  void insertHash(hash_t *hash);
  void insertHashes(list_t *hashes);
  void updateHash(hash_t *hsh, bool notFound = false);
  void updateItem(QStandardItem *item);
  bool removeRow(int row);
  void setUnameVisible(bool x);
  void setAuditMode(bool x);

private:
  QMap<int, QStandardItem*> itemFromId;
  bool unameVisible;
  bool auditMode;

};

#endif
