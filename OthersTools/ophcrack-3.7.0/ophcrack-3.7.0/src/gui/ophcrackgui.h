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
#ifndef OPHCRACKGUI_H
#define OPHCRACKGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <config.h>

#include "ui_ophcrackgui.h"
#include "tabledialog.h"
#include "hashmodel.h"
#include "tablemodel.h"
#include "progdelegate.h"
#include "ophcrack.h"
#include "arg.h"
#include "message.h"
#include "fsm.h"
#include "list.h"
#include "helpdialog.h"
#include "exportdialog.h"

#if HAVE_QWT
#include "graphdialog.h"
#endif

class OphcrackGUI : public QMainWindow, public Ui::OphcrackGUI {
Q_OBJECT
  
public:
  OphcrackGUI(ophcrack_t *crack);
  
private slots:
  void tableItemChanged(QStandardItem *item);
  void selectTables(void);
  void startStopCrack(void);
  void handleMessages(void);
  void loadSingleHash(void);
  void loadHashFile(void);
  void loadSamFile(void);
  void loadLocalSamSamdump(void);
  void save(void);
  void exportCSV(void);
  void delSelection(void);
  void resetStat(void);
  void displayGraph(void);
  void updateConfig(void);
  void defaultConfig(void);
  void threadChanged(int x);
  void hreduxChanged(int x);
  void queueChanged(int x);
  void bforceChanged(int x);
  void hideUnameChanged(int x);
  void auditModeChanged(int x);
  void close(void);
  void addDelHash(const QModelIndex &parent, int start, int end);
  void tablesChanged(void);
  void hashesAdded(list_t *hashes);
  void displayAbout(void);
  void displayHelp(void);
  void installTables(QString str, bool replace, bool warn);
  void sessOnOff(int state);
  void chooseSessFile(void);

private:
  void startCrack(void);
  void stopCrack(void);
  void updateStat(void);
  void updateProgress(void);
  void updateStatus(void);
  void resetTime(void);
  void saveConfig(void);
  void closeEvent(QCloseEvent *event);
  void showNotFound(bool flag);

private:
  fsm_t *fsm;
  arg_t *arg;

  struct timeval tm_start;
  struct timeval tm_total;
  struct timeval tm_sess;

  ophcrack_t *crack;
  HashModel *hashModel;
  TableModel *tableModel;
  TableDialog *tableDialog;
  ProgDelegate *progDelegate;
  QItemSelectionModel *hashSelModel;
  HelpDialog *helpDialog;
  ExportDialog *exportDialog;
#if HAVE_QWT
  GraphDialog *graphDialog;
#endif

  QMenu *loadMenu;
  QAction *singleHash;
  QAction *hashFile;
  QAction *samFile;
  QAction *sessFile;
  QAction *localSam2;

  QMenu *saveMenu;
  QAction *saveFile;
  QAction *exportFile;

};

#endif
