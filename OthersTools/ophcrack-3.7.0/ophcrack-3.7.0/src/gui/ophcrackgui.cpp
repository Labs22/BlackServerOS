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
#include <QMessageBox>
#include <QTimer>
#include <QMenu>
#include <assert.h>
#include <math.h>
#include <config.h>

#include "ophcrackgui.h"
#include "hash.h"
#include "ophtask.h"
#include "list.h"
#include "message.h"
#include "original.h"
#include "singlehashdialog.h"
#include "aboutdialog.h"
//---------------------------------------------------------------------------
OphcrackGUI::OphcrackGUI(ophcrack_t *crack) : QMainWindow() {
  setupUi(this);

  this->crack = crack;
  this->arg = crack->arg;
  this->fsm = fsm_alloc(crack);

  tableDialog = 0;
  helpDialog = 0;
  exportDialog = 0;
#if HAVE_QWT
  graphDialog = 0;
#endif

  fsm_reset_preload(fsm);
  fsm_reset_bforce(fsm);
  resetTime();

  // Setup the model which will display the hashes.

  hashModel = new HashModel(hashView, arg->auditmode);
  hashView->setModel(hashModel);
  hashSelModel = hashView->selectionModel();

  connect(hashView, SIGNAL(delSelection()), this, SLOT(delSelection()));
  connect(hashModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), 
	  this, SLOT(addDelHash(const QModelIndex &, int, int)));

  connect(hashModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), 
	  this, SLOT(addDelHash(const QModelIndex &, int, int)));

  deleteButton->setEnabled(false);
  saveButton->setEnabled(false);

  // Insert into the model all the hashes which have been loaded
  // before we start the GUI.

  list_nd_t *nd;

  for (nd = crack->hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    hashModel->insertHash(hsh);
  }

  // Setup the model which we will use to display the tables.

  tableModel = new TableModel(tableView);
  tableView->setModel(tableModel);
  tableView->hideColumn(TableModel::DIR_COL_IDX);

  progDelegate = new ProgDelegate(tableView);
  tableView->setItemDelegateForColumn(TableModel::PROGRESS_COL_IDX, progDelegate);

  connect(tableModel, SIGNAL(itemChanged(QStandardItem*)),
	  this, SLOT(tableItemChanged(QStandardItem*)));

  // Install the tables which have been specified on the command line.

  list_t *table_str = crack->arg->table_str_cmd;
  
  for (nd = table_str->head; nd != 0; nd = nd->next) {
    char *tblstr = (char*)nd->data;
    installTables(tblstr, true, false);
  }

  bool replace = table_str->size > 0 ? false : true;

  // Install the tables given in the config file. Do not replace the
  // table if some have already been installed from the command line.

  table_str = crack->arg->table_str_conf;

  for (nd = table_str->head; nd != 0; nd = nd->next) {
    char *tblstr = (char*)nd->data;
    installTables(tblstr, replace, false);
  }

  // Mark as not installed the tables which have not been loaded.

  tableModel->insertRoot(lmalphanum5k);
  tableModel->insertRoot(lmalphanum10k);
  tableModel->insertRoot(lmextended);
  tableModel->insertRoot(lmgermanv1);
  tableModel->insertRoot(lmgermanv2);
  tableModel->insertRoot(ntextended);
  tableModel->insertRoot(ntdict);
  tableModel->insertRoot(ntnine);
  tableModel->insertRoot(nteight);
  tableModel->insertRoot(ntnum);
  tableModel->insertRoot(ntseven);
  tableModel->insertRoot(lmflash);
  tableModel->insertRoot(nteightxl);
  tableModel->insertRoot(ntspecialxl);
  tableModel->insertRoot(ntprobafree);
  tableModel->insertRoot(ntproba10g);
  tableModel->insertRoot(ntproba60g);

  // Associate a menu to the load button.

  loadMenu   = new QMenu();
  singleHash = loadMenu->addAction("Single hash");
  hashFile   = loadMenu->addAction("PWDUMP file");
  sessFile   = loadMenu->addAction("Session file");
  samFile    = loadMenu->addAction("Encrypted SAM");
#ifdef WIN32
  localSam2  = loadMenu->addAction("Local SAM with samdump2");
#endif

  connect(singleHash, SIGNAL(triggered()), this, SLOT(loadSingleHash()));
  connect(hashFile, SIGNAL(triggered()), this, SLOT(loadHashFile()));
  connect(samFile, SIGNAL(triggered()), this, SLOT(loadSamFile()));
  connect(sessFile, SIGNAL(triggered()), this, SLOT(loadHashFile()));
#ifdef WIN32
  connect(localSam2, SIGNAL(triggered()), this, SLOT(loadLocalSamSamdump()));
#endif

  loadButton->setMenu(loadMenu);

  // Associate a menu to the save button.

  saveMenu   = new QMenu();
  saveFile   = saveMenu->addAction("Save to file");
  exportFile = saveMenu->addAction("Export to CSV");

  connect(saveFile, SIGNAL(triggered()), this, SLOT(save()));
  connect(exportFile, SIGNAL(triggered()), this, SLOT(exportCSV()));

  saveButton->setMenu(saveMenu);

  // Connect the other buttons.

  connect(deleteButton, SIGNAL(clicked()), this, SLOT(delSelection()));
  connect(crackButton, SIGNAL(clicked()), this, SLOT(startStopCrack()));
  connect(tableButton, SIGNAL(clicked()), this, SLOT(selectTables()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetStat()));
#if HAVE_QWT
  connect(graphButton, SIGNAL(clicked()), this, SLOT(displayGraph()));
#else
  graphButton->setEnabled(false);
#endif
  connect(threadSlider, SIGNAL(valueChanged(int)), this, SLOT(threadChanged(int)));
  connect(hreduxSlider, SIGNAL(valueChanged(int)), this, SLOT(hreduxChanged(int)));
  connect(queueSlider, SIGNAL(valueChanged(int)), this, SLOT(queueChanged(int)));
  connect(defaultButton, SIGNAL(clicked()), this, SLOT(defaultConfig()));
  connect(bforceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(bforceChanged(int)));
  connect(hideUnameBox, SIGNAL(currentIndexChanged(int)), this, SLOT(hideUnameChanged(int)));
  connect(auditModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(auditModeChanged(int)));
  connect(aboutButton, SIGNAL(clicked()), this, SLOT(displayAbout()));
  connect(helpButton, SIGNAL(clicked()), this, SLOT(displayHelp()));
  connect(sessBox, SIGNAL(stateChanged(int)), this, SLOT(sessOnOff(int)));
  connect(sessButton, SIGNAL(clicked()), this, SLOT(chooseSessFile()));

  // Setup a timer which will be used to pool the message queue at
  // regular intervals.

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(handleMessages()));
  timer->start(500);

  // Associtate the tables to the hashes and update the statistics,
  // status bar and the preferences.

  tablesChanged();

  updateStat();
  updateStatus();
  updateConfig();

  // Start cracking if the user asked it.

  if (arg->run)
    emit startStopCrack();
}
//---------------------------------------------------------------------------
void OphcrackGUI::tableItemChanged(QStandardItem *item) {
  QStandardItem *parent = item->parent();
  QModelIndex index = tableModel->indexFromItem(item);

  int row = index.row();
  int col = index.column();

  if (col > 0) return;

  // If it is a top level item, then we refresh all the root items.

  if (parent == 0) {
    int nrows = tableModel->rowCount();

    for (int i=0; i<nrows; ++i) {
      QStandardItem *root = tableModel->item(i, 0);
      int nenabled = root->data(Qt::UserRole).toInt();

      if (nenabled > 0)
	tableView->setRowHidden(i, QModelIndex(), false);
      else
	tableView->setRowHidden(i, QModelIndex(), true); 
    }
  }

  // If is is a child item, then we refresh only the concerned item.

  else {
    QModelIndex index = tableModel->indexFromItem(parent);
    table_t *tbl = tableModel->tableFromItem(item);
    
    if (tbl && tbl->enabled)
      tableView->setRowHidden(row, index, false);
    else
      tableView->setRowHidden(row, index, true);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::selectTables(void) {
  if (tableDialog == 0) {
    tableDialog = new TableDialog(tableModel, this);

    connect(tableDialog, SIGNAL(install(QString, bool, bool)), 
	    this, SLOT(installTables(QString, bool, bool)));
  }

  if (tableDialog->exec() == QDialog::Accepted) {
    fsm_reset_preload(fsm);
    tablesChanged();
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::tablesChanged(void) {
  // Return to the 'start' state and ignore any pending message.

  fsm->oldstate = st_start;
  list_clear(fsm->pending_msg);

  // Save the table configuration and update the status bar.

  saveConfig();
  updateStatus();

  // Build a list of the enabled tables following the order specified
  // by the user.

  list_t *enabled = crack->enabled;
  list_clear(enabled);

  int nrows = tableModel->rowCount();
  int i, j;

  for (i=0; i<nrows; ++i) {
    QStandardItem *item = tableModel->item(i, 0);
    int ntables = item->rowCount();

    for (j=0; j<ntables; ++j) {
      QStandardItem *child = item->child(j, 0);
      table_t *tbl = (table_t*)child->data(Qt::UserRole).value<void*>();

      if (tbl->enabled)
	list_add_tail(enabled, tbl);
    }
  }

  // Associate the enabled tables to the hashes. The tables are
  // added in the order specified by the user. Since, for a given
  // hash, some tables might have been searched in already, we have to
  // be careful to retain the last visited column so that we do not
  // need to go through the whole table again.

  list_t *hashes = crack->hashes;
  int maxtid = crack->maxtid;

  list_nd_t *hnd;
  list_nd_t *tnd;

  htbl_t **id_to_htbl = (htbl_t**)malloc(maxtid*sizeof(htbl_t*)); 

  for (hnd = hashes->head; hnd != 0; hnd = hnd->next) {
    hash_t *hsh = (hash_t*)hnd->data;

    // We store the tables associated with the current hash into the
    // id_to_htbl array so that we can retrieve them easily later. The
    // column index is reset to its 'covered' (minus one) value so
    // that we are sure we do not miss a column.

    memset(id_to_htbl, 0, maxtid*sizeof(htbl_t*));

    while (hsh->tables->size > 0) {
      htbl_t *htbl = (htbl_t*)list_rem_head(hsh->tables);
      table_t *tbl = htbl->tbl;

      assert(id_to_htbl[tbl->id] == 0);
      
      id_to_htbl[tbl->id] = htbl;
      htbl->col = htbl->covered-1;
    }

    // We associate the tables to the current hash following the order
    // specified by the user.

    list_t *enabled = crack->enabled;

    for (tnd = enabled->head; tnd != 0; tnd = tnd->next) {
      table_t *tbl = (table_t*)tnd->data;
      htbl_t *htbl = id_to_htbl[tbl->id];

      if (htbl != 0) {
	list_add_tail(hsh->tables, htbl);
	id_to_htbl[tbl->id] = 0;
      } else
	ophcrack_associate(crack, hsh, tbl);
    }

    // Some tables which were previously associated to the hash might
    // not have been enabled this time. We store them anyway because
    // we want to keep track of the last column we have been searching
    // from.

    int i;

    for (i=0; i<maxtid; ++i) {
      htbl_t *htbl = id_to_htbl[i];

      if (htbl != 0)
	list_add_tail(hsh->tables, htbl);
    }

    // Since we changed the list of tables, we should reset the table
    // list node pointer.

    hsh->tnd = 0;
  }

  free(id_to_htbl);

  // Update the status of the search in the tables.

  ophcrack_update(crack);
  updateProgress();
}
//---------------------------------------------------------------------------
void OphcrackGUI::hashesAdded(list_t *hashes) {
  list_t *enabled = crack->enabled;
  list_nd_t *hnd, *tnd;

  for (hnd = hashes->head; hnd != 0; hnd = hnd->next) {
    hash_t *hsh = (hash_t*)hnd->data;

    assert(hsh->tables->size == 0);

    for (tnd = enabled->head; tnd != 0; tnd = tnd->next) {
      table_t *tbl = (table_t*)tnd->data;
      ophcrack_associate(crack, hsh, tbl);
    }
  }

  fsm_reset_preload(fsm);
  fsm_reset_bforce(fsm);

  ophcrack_update(crack);
  updateProgress();
}
//---------------------------------------------------------------------------
void OphcrackGUI::startStopCrack(void) {
  int npwds_total = crack->npwds_total;
  int npwds_found = crack->npwds_found;

  if (fsm->state == st_wait && npwds_found < npwds_total) {
    startCrack();
    fsm_reset(fsm);

    fsm->state = st_start;
    fsm_handle_start(fsm);
  } 

  else if (fsm->state == st_pause1) {
    if (npwds_found < npwds_total) {
      startCrack();

      fsm->state = st_pause2;
      fsm_handle_pause2(fsm, 0);
    }
  }

  else if (fsm->state == st_pause2) {
    stopCrack();
    fsm->state = st_pause1;
  }

  else if (fsm->state != st_wait) {
    stopCrack();
    fsm_pause(fsm);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::loadSingleHash(void) {
  SingleHashDialog singleHashDialog;

  if (singleHashDialog.exec() == QDialog::Accepted) {
    char *str = strdup(singleHashDialog.getHash());

    list_t *hashes = list_alloc();
    list_nd_t *nd;

    int maxhid = crack->maxhid;
    int ret = hash_extract_lmnt(str, hashes, maxhid);
 
    // If not hashes could be loaded, then we inform the user.

    if (ret == 0) {
      QMessageBox msgBox(QMessageBox::Warning,
			 "Warning",
			 QString("No hashes could be extracted." ) +
			 QString("Check that you provided valid hashes ") +
			 QString("in one of the three formats allowed."),
			 QMessageBox::Ok);
      msgBox.exec();
    } 

    // Otherwise, we add the hashes to the list of hashes we must
    // potentially crack.

    else {
      for (nd = hashes->head; nd != 0; nd = nd->next) {
	hash_t *hsh = (hash_t*)nd->data;

	ophcrack_add_hash(crack, hsh);
	hashModel->insertHash(hsh);
      }

      hashesAdded(hashes);
    }

    free(str);
    list_free(hashes);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::loadHashFile(void) {
  
  QString fileName = QFileDialog::getOpenFileName(this, 
						  tr("Open PWDUMP file"), 
						  QString(), 
						  QString()); 

  if (!fileName.isNull()) {
    const char *fname = fileName.toLatin1().constData();
    
    // If the file cannot be read, then we warn the user.

    FILE *file = fopen(fname, "r");

    if (file == 0) {
      QMessageBox msgBox(QMessageBox::Warning,
			 "Warning",
			 QString("Cannot open file %1 for reading.").arg(fname),
			 QMessageBox::Ok);
      msgBox.exec();
    }

    // Otherwise, we load all the hashes we find.

    else {
      list_t *hashes = list_alloc();
      list_nd_t *nd;

      int maxhid = crack->maxhid;
      hash_load_pwdump(hashes, file, maxhid);

      for (nd = hashes->head; nd != 0; nd = nd->next) {
	hash_t *hsh = (hash_t*)nd->data;
	
	ophcrack_add_hash(crack, hsh);
	hashModel->insertHash(hsh);
      }

      hashesAdded(hashes);

      list_free(hashes);
      fclose(file);
    }
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::loadSamFile(void) {

  QString dir = QFileDialog::getExistingDirectory(this, 
						  tr("Select the directory containing the encrypted SAM and SYSTEM files.\n(Usually under C:\\WINDOWS\\system32\\config\\)"), 
						  QString(), 
						  QFileDialog::ShowDirsOnly);
  
  if (!dir.isNull()) {
    list_t *hashes = list_alloc();
    list_nd_t *nd;
    
    // Retrieve the hashes from the encrypted SAM.
    
    int maxhid = crack->maxhid;
    int npwds = hash_load_sam(hashes, dir.toLatin1().constData(), maxhid);
    QString buff = 0;

    // Check that something has actually been loaded.
    
    if (npwds == 0) 
      buff = QString("No proper hashes have been found in the encrypted SAM file in %1.").arg(arg->samdir);
    
    // Check if the SYSTEM or SAM file could not be found.
    
    else if (npwds == -1) 
      buff = QString("No SYSTEM file has been found in %1.").arg(arg->samdir);
    
    else if (npwds == -2) 
      buff = QString("No SAM file has been found in %1.").arg(arg->samdir);
    
    // Check if there was a problem while reading the SYSTEM or SAM file.
    
    else if (npwds == -3)
      buff = QString("A problem occured while reading the SYSTEM file found in %1.").arg(arg->samdir);
    
    else if (npwds == -4) 
      buff = QString("A problem occured while reading the SAM file found in %1.").arg(arg->samdir);

    if (npwds <= 0) {
      QMessageBox msgBox(QMessageBox::Warning,
			 "Warning",
			 buff,
			 QMessageBox::Ok);
      msgBox.exec();
    }
    
    
    // Add the hashes.
    
    for (nd = hashes->head; nd != 0; nd = nd->next) {
      hash_t *hsh = (hash_t*)nd->data;
      
      ophcrack_add_hash(crack, hsh);
      hashModel->insertHash(hsh);
    }

    hashesAdded(hashes);    
    list_free(hashes);    
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::loadLocalSamSamdump(void) {  
  // Retrieve the hashes from the local SAM with samdump2.
  
  list_t *hashes = list_alloc();
  list_nd_t *nd;
  
  // Retrieve the hashes from the encrypted SAM.
  
  int maxhid = crack->maxhid;
  int npwds = hash_dump_sam(hashes, maxhid);
  QString buff = 0;

  // Check that something has actually been loaded.
  
  if (npwds == 0) 
    buff = QString("No proper hashes have been dumped with samdump2");
  
  else if (npwds == -1) 
    buff = QString("Error in samdump2: get_live_syskey function");
  
  else if (npwds == -2) 
    buff = QString("Error in samdump2: get_sam function\n\nMake sure ophcrack is running with administrator rights to perform this action. On Windows Vista and later, right-click on ophcrack icon and select run as administrator.\n\nThis function is not supported on Windows 10 Anniversary update. Use mimikatz (https://github.com/gentilkiwi/mimikatz/releases/) to dump local hashes: mimikatz privilege::debug  token::elevate lsadump::sam");
    
  else if (npwds == -3)
    buff = QString("Error in samdump2: samdump2 function");

  if (npwds <= 0) {
    QMessageBox msgBox(QMessageBox::Warning,
		       "Warning",
		       buff,
		       QMessageBox::Ok);
    msgBox.exec();
  }
    
    
  // Add the hashes.
  
  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    
    ophcrack_add_hash(crack, hsh);
    hashModel->insertHash(hsh);
  }
  
  hashesAdded(hashes);    
  list_free(hashes);    
}
//---------------------------------------------------------------------------
void OphcrackGUI::save(void) {
  QString fileName;

  // If an output file has already been specified, then we preselect it.

  if (arg->ofname != 0)
    fileName = QFileDialog::getSaveFileName(this, 
					    tr("Save File"),
					    arg->ofname, 
					    QString());

  // Otherwise, we preselect the current directory.

  else
    fileName = QFileDialog::getSaveFileName(this, 
					    tr("Save File"),
					    QDir::currentPath(), 
					    QString());

  // If the user chose a file, then we use it to store all the hashes
  // which are currently loaded.
  
  if (!fileName.isNull()) {
    const char *fname = fileName.toLatin1().constData();
    
    // Store the selected file so that we can preselect it later.

    if (arg->ofname != 0) free(arg->ofname);
    arg->ofname = strdup(fname);

    FILE *file = fopen(fname, "w");

    if (file == 0) {
      QMessageBox msgBox(QMessageBox::Warning,
			 "Warning",
			 QString("Cannot open file %1 for writing.").arg(fname),
			 QMessageBox::Ok);
      msgBox.exec();
    } 

    else {
      scheduler_t *sched = crack->sched;
      pthread_mutex_t *mutex = sched->mutex;

      pthread_mutex_lock(mutex);
      ophcrack_save(crack, file, 0, 0);
      pthread_mutex_unlock(mutex);

      fclose(file);
    }
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::exportCSV(void) {
  QString fileName;
  int *fields;

  fields = (int*)malloc(13*sizeof(int));
  if (exportDialog == 0)
    exportDialog = new ExportDialog(this);
  
  if (exportDialog->exec() == QDialog::Accepted) {
    exportDialog->getFields(fields);
    fileName = QFileDialog::getSaveFileName(this, 
					    tr("Export to CSV"),
					    QDir::currentPath(), 
					    QString());

    if (!fileName.isNull()) {
      const char *fname = fileName.toLatin1().constData();
      
      FILE *file = fopen(fname, "w");
      
      if (file == 0) {
	QMessageBox msgBox(QMessageBox::Warning,
			   "Warning",
			 QString("Cannot open file %1 for writing.").arg(fname),
			   QMessageBox::Ok);
	msgBox.exec();
      } 
      
      else {
	scheduler_t *sched = crack->sched;
	pthread_mutex_t *mutex = sched->mutex;
	
	pthread_mutex_lock(mutex);
	ophcrack_export_csv(crack, file, fields, 
			    exportDialog->getSeparator(), 
			    exportDialog->getQuotes());
	pthread_mutex_unlock(mutex);
	
	fclose(file);
      }
    }
    
  }
  free(fields);
}
//---------------------------------------------------------------------------
void OphcrackGUI::resetStat(void) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  pthread_mutex_lock(mutex);
  ophstat_reset(crack->stat);
  pthread_mutex_unlock(mutex);

  resetTime();
  updateStat();
  updateStatus();
}
//---------------------------------------------------------------------------
void OphcrackGUI::displayGraph(void) {
#if HAVE_QWT
  if (graphDialog == 0) {
    graphDialog = new GraphDialog(this);
  }

  int tv_sec = (int) tm_total.tv_sec;
  
  graphDialog->plot(crack->stat, tv_sec);
  graphDialog->show();
#endif
}
//---------------------------------------------------------------------------
void OphcrackGUI::handleMessages(void) {
  message_t *msg = 0;

  int count = 1000;
  bool flag = false;

  // Handle at most 'count' messages.

  state_t fsm_state = fsm->state;

  while (count-- && (msg = message_tryget())) {
    // A password has been found.
    
    if (msg->kind == msg_found) {
      msg_found_t *found = (msg_found_t*)msg->data;
      hash_t *hsh = found->hsh;

#if HAVE_QWT
      int tv_sec = (int) tm_total.tv_sec;

      if (graphDialog != 0 && graphDialog->isVisible()) 
	graphDialog->plot(crack->stat, tv_sec);
#endif
      hashModel->updateHash(hsh);
    }

    // The preloading of a single file is done.

    else if (msg->kind == msg_preload) {
      msg_load_t *load = (msg_load_t*)msg->data;
      table_t *tbl = load->tbl;

      if (tbl && load->done)
        tableModel->updateTable(tbl);

      else if (tbl == 0 && load->size == 0) {
        for (list_nd_t *nd = crack->enabled->head; nd != 0; nd = nd->next)
          tableModel->updateTable((table_t*)nd->data);
      }
    }

    // A table has been unloaded.

    else if (msg->kind == msg_unload) {
      msg_load_t *load = (msg_load_t*)msg->data;
      table_t *tbl = load->tbl;

      if (tbl)
	tableModel->updateTable(tbl);
    }

    // Ask the finite state machine to take the appropriate steps
    // given the received message.

    msg = fsm_next(fsm, msg);

    // Delete the message.

    if (msg)
      message_free(msg);

    flag = true;
  }

  if (fsm->state & (~st_wait & ~st_pause1 & ~st_pause2)) {
    updateStat();
    updateProgress();
    updateStatus();
  }

  else if (flag)
    updateStatus();

  // Check if it is time to save the session.

  if (arg->ssave && fsm->state & (st_work1 + st_work2)) {
    struct timeval tm_curr;
    gettimeofday(&tm_curr, 0);

    time_t tv_sec = tm_curr.tv_sec - tm_sess.tv_sec;

    if (tv_sec >= 30) {
      tm_sess = tm_curr;
      fsm_ssave(fsm);
    }
  }

  // Check if the job is finished.

  if (fsm->state == st_wait && fsm_state != st_wait) stopCrack();
}
//---------------------------------------------------------------------------
void OphcrackGUI::startCrack(void) {
  crackButton->setIcon(QIcon(":/icons/pixmaps/stop.png"));
  crackButton->setText("Stop");

  loadButton->setEnabled(false);
  deleteButton->setEnabled(false);
  tableButton->setEnabled(false);
  tabWidget->setTabEnabled(2, false);

  gettimeofday(&tm_start, 0);
  tm_sess = tm_start;
  extern struct timeval tm_main_start;
  tm_main_start = tm_start;

  showNotFound(false);
}
//---------------------------------------------------------------------------
void OphcrackGUI::stopCrack(void) {
  crackButton->setIcon(QIcon(":/icons/pixmaps/crack.png"));
  crackButton->setText("Crack");

  loadButton->setEnabled(true);
  deleteButton->setEnabled(crack->hashes->size > 0 ? true : false);
  tableButton->setEnabled(true);
  tabWidget->setTabEnabled(2, true);

  updateStat();
  updateStatus();
  updateProgress();

  if (fsm->state == st_wait) showNotFound(true);
}
//---------------------------------------------------------------------------
void OphcrackGUI::delSelection(void) {
  // Check that we are in a state which allow deleting hashes.

  if (fsm->state & (~st_wait & ~st_pause1)) return;

  // Check that some hashes have actually been selected.

  QList<QModelIndex> indexes = hashSelModel->selectedRows();
  
  if (indexes.isEmpty()) return;

  // Create a map which gives, for a given hash id, the list of hashes
  // with that id in the order they are stored in the list of hashes
  // in the ophrack_t data structure.

  QMultiMap<int, hash_t*>hashFromId;
  list_t *hashes = crack->hashes;

  while (hashes->size > 0) {
    hash_t *hsh = (hash_t*)list_rem_tail(hashes);
    hashFromId.insert(hsh->id, hsh);
  }

  // Delete the selected hashes. If we are in the 1st pause state, it
  // might not be appropriate to free the memory occupied by a hash
  // because it might still be used by a running task. In that case,
  // we store the hashes and they will be removed later, when it will
  // be safer.

  int maxhid = crack->maxhid;

  while (!indexes.isEmpty()) {
    QModelIndex index = indexes.takeFirst();
    int id = index.data(Qt::UserRole).toInt();

    assert(id >= 0 && id < maxhid);
    QList<hash_t*> hashes = hashFromId.values(id);
    assert(hashes.size() > 0);

    while (!hashes.isEmpty()) {
      hash_t *hsh = hashes.takeFirst();
      hsh->id = -1;

      if (fsm->state == st_pause1)
	list_add_tail(fsm->htoremove, hsh);
      else
	hash_free(hsh);
    }

    hashFromId.remove(id);
  }

  // Manually reset the hash related stuff in ophcrack.

  crack->hnd = 0;
  crack->maxhid = 0;
  crack->npwds_total = 0;
  crack->npwds_found = 0;
  for (int i=0; i<16; i++) {
    crack->stat->length[i]=0;
    crack->stat->category[i]=0;
  }
  list_clear(crack->stat->time);

  // Remove all the lines in the hash view. Since the number of rows
  // changes, we always remove the first one.

  int nrows = hashModel->rowCount();

  for (int i=0; i<nrows; ++i)
    hashModel->removeRow(0);

  // Adjust the id of the remaining hashes and insert them into the
  // ophcrack_t data structure.

  for (int i=0; i<maxhid; ++i) {
    QList<hash_t*> hashes = hashFromId.values(i);
    int j = i;

    while (hashes.isEmpty() && ++j < maxhid)
      hashes = hashFromId.values(j);

    if (j == maxhid) break;
    hashFromId.remove(j);

    while (!hashes.isEmpty()) {
      hash_t *hsh = hashes.takeFirst();

      hsh->id = i;
      ophcrack_add_hash(crack, hsh);
      hashModel->insertHash(hsh);
    }
  }

  assert(crack->maxhid == crack->npwds_total);

  updateStatus();
  updateProgress();
}
//---------------------------------------------------------------------------
void OphcrackGUI::updateStat(void) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;
  ophstat_t *stat = crack->stat;

  pthread_mutex_lock(mutex);
  uint64_t hredux = stat->hredux;
  uint64_t prefix = stat->prefix;
  uint64_t postfix = stat->postfix;
  uint64_t start = stat->start;
  uint64_t fseek_idx = stat->fseek_idx;
  uint64_t fseek_end = stat->fseek_end;
  uint64_t fseek_srt = stat->fseek_srt;
  uint64_t falarm = stat->falarm;
  uint64_t falarm_hredux = stat->falarm_hredux;
  uint64_t match_table = stat->match_table;
  uint64_t match_bforce = stat->match_bforce;
  pthread_mutex_unlock(mutex);

  // Hash/Redux and false alarms.

  hreduxField->clear();
  hreduxField->insert(QString("%1").arg(hredux));

  falarmField->clear();
  falarmField->insert(QString("%1").arg(falarm));

  falarm2Field->clear();

  if (falarm > 0)
    falarm2Field->insert(QString("%1").arg(falarm_hredux / falarm));
  else
    falarm2Field->insert(QString("0"));

  // Passwords.

  pwdField->clear();
  pwdField->insert(QString("%1").arg(match_table));

  bforceField->clear();
  bforceField->insert(QString("%1").arg(match_bforce));

  // Fseek.

  uint64_t fseek = fseek_idx + fseek_end + fseek_srt;

  fseekField->clear();
  fseekField->insert(QString("%1").arg(fseek));

  idxField->clear();
  idxField->insert(QString("%1").arg(fseek_idx));

  endField->clear();
  endField->insert(QString("%1").arg(fseek_end));

  srtField->clear();
  srtField->insert(QString("%1").arg(fseek_srt));

  // Prefix, postfix and start.

  prefixField->clear();
  prefixField->insert(QString("%1").arg(prefix));

  postfixField->clear();
  postfixField->insert(QString("%1").arg(postfix));

  startField->clear();
  startField->insert(QString("%1").arg(start));

}
//---------------------------------------------------------------------------
void OphcrackGUI::updateProgress(void) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophcrack_update(crack);

  pthread_mutex_lock(mutex);
  tableModel->updateProgress();
  pthread_mutex_unlock(mutex);
}
//---------------------------------------------------------------------------
void OphcrackGUI::updateStatus(void) {
  int npwds_total = crack->npwds_total;
  int npwds_found = crack->npwds_found;

  // Preload progress.

  uint64_t psize_curr  = fsm->psize_curr;
  uint64_t psize_total = fsm->psize_total;
  int psize_ratio = 0;

  if (psize_total > 0)
    psize_ratio = (100*psize_curr) / psize_total;

  QString preloadStr = "waiting";

  if (psize_ratio == 100)
    preloadStr = "done";

  else if (psize_total > 0)
    preloadStr = QString("%1%").arg(psize_ratio);

  preloadEdit->setText(preloadStr);

  // Brute force.

  if (!arg->bforce)
    bforceEdit->setText("disabled");

  else {
    uint64_t bforce_curr  = fsm->bforce_curr;
    uint64_t bforce_total = fsm->bforce_total;
    int bforce_ratio = 0;

    if (bforce_total > 0)
      bforce_ratio = (100*bforce_curr) / bforce_total;

    QString bforceStr = "waiting";

    if (bforce_ratio == 100)
      bforceStr = "done";

    else if (bforce_total > 0)
      bforceStr = QString("%1%").arg(bforce_ratio);

    bforceEdit->setText(bforceStr);
  }

  // Number of passwords found.

  QString pwdStr = QString("%1/%2").arg(npwds_found)
    .arg(npwds_total);

  pwdEdit->setText(pwdStr);

  // Elapsed time.

  if (fsm->state & (~st_wait & ~st_pause1 & ~st_pause2)) {
    struct timeval tm_stop;
    extern struct timeval tm_main_total;
    gettimeofday(&tm_stop, 0);
  
    tm_total.tv_sec += tm_stop.tv_sec - tm_start.tv_sec;
    tm_main_total.tv_sec = tm_total.tv_sec;
    tm_start = tm_stop;
  }

  long tv_sec = tm_total.tv_sec;
  long hour   = tv_sec / 3600;
  long sec    = tv_sec - 3600 * hour;
  long min    = sec / 60;
  
  sec %= 60;

  QString tmStr = QString("%1h %2m %3s").arg(hour).arg(min)
    .arg(sec);
  
  timeEdit->setText(tmStr);  
}
//---------------------------------------------------------------------------
void OphcrackGUI::resetTime(void) {
  tm_start.tv_sec  = 0;
  tm_start.tv_usec = 0;
  tm_total.tv_sec  = 0;
  tm_total.tv_usec = 0;

  if (fsm->state & (~st_wait & ~st_pause1 & ~st_pause2))
    gettimeofday(&tm_start, 0);
}
//---------------------------------------------------------------------------
void OphcrackGUI::defaultConfig(void) {
  arg_default(arg);
  updateConfig();
}
//---------------------------------------------------------------------------
void OphcrackGUI::updateConfig(void) {
  int nthreads  = arg->nthreads;
  int nhredux   = arg->nhredux;
  int mdqueue   = arg->mdqueue;
  int bforce    = arg->bforce;
  int hideuname = arg->hideuname;
  int auditmode = arg->auditmode;

  threadSlider->setSliderPosition(nthreads);
  threadChanged(nthreads);

  hreduxSlider->setSliderPosition(nhredux / 5000);
  hreduxChanged(nhredux / 5000);

  queueSlider->setSliderPosition(mdqueue / 50);
  queueChanged(mdqueue / 50);

  bforceBox->setCurrentIndex(bforce > 0 ? 1 : 0);

  hideUnameBox->setCurrentIndex(hideuname > 0 ? 1 : 0);

  auditModeBox->setCurrentIndex(auditmode > 0 ? 1 : 0);

  if (arg->sfname != 0) {
    sessEdit->setText(arg->sfname);
    sessBox->setCheckState(arg->ssave ? Qt::Checked : Qt::Unchecked);
    sessBox->setEnabled(true);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::threadChanged(int x) {
  if (x != crack->arg->nthreads) {
    arg->nthreads = x;
    threadLabel->setText(QString("%1 (restart required)").arg(x));
  }
  else
    threadLabel->setText(QString("%1").arg(x));
}
//---------------------------------------------------------------------------
void OphcrackGUI::hreduxChanged(int x) {
  arg->nhredux = 5000*x;
  hreduxLabel->setText(QString("%1").arg(arg->nhredux));
}
//---------------------------------------------------------------------------
void OphcrackGUI::queueChanged(int x) {
  arg->mdqueue = 50*x;
  queueLabel->setText(QString("%1").arg(arg->mdqueue));
}
//---------------------------------------------------------------------------
void OphcrackGUI::bforceChanged(int x) {
  arg->bforce = x;

  fsm_reset_bforce(fsm);
  updateStatus();
}
//---------------------------------------------------------------------------
void OphcrackGUI::hideUnameChanged(int x) {
  arg->hideuname = x;

  hashModel->setUnameVisible(x);

  list_nd_t *nd;
  for (nd = crack->hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    hashModel->updateHash(hsh);
  }

}
//---------------------------------------------------------------------------
void OphcrackGUI::auditModeChanged(int x) {
  arg->auditmode = x;

  hashModel->setAuditMode(x);

  list_nd_t *nd;
  for (nd = crack->hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    hashModel->updateHash(hsh);
  }
  
  if (x)
    hideUnameBox->setEnabled(false);
  else
    hideUnameBox->setEnabled(true);
}
//---------------------------------------------------------------------------
void OphcrackGUI::saveConfig(void) {
  FILE *file = fopen(arg->cfname, "w");

  if (file == 0) return;

  // Save the order and state (enabled or not) of the tables.

  int nrows = tableModel->rowCount();
  
  for (int i=0; i<nrows; ++i) {
    QStandardItem *root = tableModel->item(i, 0);
    int nenabled  = root->data(Qt::UserRole).toInt();
    int nchildren = root->rowCount(); 
    
    if (nenabled == -1) continue;
    
    QStandardItem *child = root->child(0, 0);
    table_t *tbl = tableModel->tableFromItem(child);
    const char *path = tbl->path;
    
    fprintf(file, "table %s", path);

    if (nenabled == 0) {
      fprintf(file, ",-\n");
      continue;
    }
    
    for (int j=0; j<nchildren; ++j) {
      child = root->child(j, 0);
      tbl = tableModel->tableFromItem(child);
      
      if (tbl->enabled)
	fprintf(file, ",%d", tbl->idx);
    }
    
    fprintf(file, "\n");
  }

  // Save the preferences.

  fprintf(file, "nthreads %d\n", arg->nthreads);
  fprintf(file, "nhredux %d\n", arg->nhredux);
  fprintf(file, "maxdqueue %d\n", arg->mdqueue);
  fprintf(file, "bforce %d\n", arg->bforce);
  fprintf(file, "hideuname %d\n", arg->hideuname);
  fprintf(file, "auditmode %d\n", arg->auditmode);

  if (arg->sfname != 0) {
    fprintf(file, "session_file %s\n", arg->sfname);
    fprintf(file, "session_save %d\n", arg->ssave);
  }

  fclose(file);
}
//---------------------------------------------------------------------------
void OphcrackGUI::close(void) {
  saveConfig();
  QMainWindow::close();
}
//---------------------------------------------------------------------------
void OphcrackGUI::closeEvent(QCloseEvent *event) {
  saveConfig();
  QMainWindow::closeEvent(event);
}
//---------------------------------------------------------------------------
void OphcrackGUI::addDelHash(const QModelIndex &, int, int) {
  // Disable/enable the save and delete button.

  saveButton->setEnabled(crack->hashes->size ? true : false);
  deleteButton->setEnabled(crack->hashes->size > 0 ? true : false);  
}
//---------------------------------------------------------------------------
void OphcrackGUI::displayAbout(void) {
  AboutDialog about;
  about.exec();
}
//---------------------------------------------------------------------------
void OphcrackGUI::displayHelp(void) {
  if (helpDialog == 0) 
    helpDialog = new HelpDialog(this);

  helpDialog->show();
}
//---------------------------------------------------------------------------
void OphcrackGUI::installTables(QString str, bool replace, bool warn) {
  const char *tblstr = str.toLatin1().constData();

  // Search for the tables in the various directories specified.

  list_t *table_path = crack->arg->table_path;
  list_t *tables = list_alloc();
  list_nd_t *dnd;

  for (dnd = table_path->head; dnd != 0; dnd = dnd->next) {
    char *dir = (char*)dnd->data;
    int ret = table_open(tables, dir, tblstr);

    if (ret >= 0) break;
  }

  // If the requested tables were not found, then we notify the user.

  if (warn && dnd == 0) {
    QMessageBox msgBox(QMessageBox::Warning,
		       "Warning",
		       QString("You requested the table(s) %1, ").arg(str) +
		       QString("but some or all tables could not ") +
		       QString("be found or loaded."),
		       QMessageBox::Ok);
    msgBox.exec();
  }

  // If some tables have been loaded, then we install them.

  else if (dnd != 0) {
    assert(tables->size > 0);

    table_t *tbl = (table_t*)tables->head->data;
    table_kind_t kind = tbl->kind;

    // Check the size of the tables. 

    int size_invalid = 0;
    list_nd_t *nd;

    for (nd = tables->head; nd != 0; nd = nd->next) {
      table_t *tbl = (table_t*)nd->data;
      ophcrack_setup_table(tbl);
      if (table_verify(tbl) < 0) {
	size_invalid = 1;
	QMessageBox msgBox(QMessageBox::Warning,
			   "Warning",
			   QString("Size of table %1 is invalid.").arg(str) +
			   QString("Please download it again."),
			   QMessageBox::Ok);
	msgBox.exec();
      }
    }

    // Check if we must replace existing tables by the newly loaded
    // ones.

    bool installed = tableModel->isInstalled(kind);

    if (replace && installed) {
      int ntables = crack->tables->size;
      int i;
      
      for (i=0; i<ntables; ++i) {
	table_t *tbl = (table_t*)list_rem_head(crack->tables);
	
	if (tbl->kind != kind)
	  list_add_tail(crack->tables, tbl);
      }
     
      tableModel->uninstall(kind);
 
      // Install the newly loaded tables.
      if (!size_invalid) {
	for (nd = tables->head; nd != 0; nd = nd->next) {
	  table_t *tbl = (table_t*)nd->data;
	  ophcrack_add_table(crack, tbl);
	}
	
	tableModel->install(tables);
      }
    }

    // If the tables are not installed yet, then we install them. If
    // 'replace' is false then we disable all tables.

    else if (!installed && !size_invalid) {
      for (nd = tables->head; nd != 0; nd = nd->next) {
	table_t *tbl = (table_t*)nd->data;

	if (!replace) tbl->enabled = 0;
	ophcrack_add_table(crack, tbl);
      }
      
      tableModel->install(tables);      
    }
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::sessOnOff(int state) {
  if (state == Qt::Unchecked) {
    arg->ssave = 0;
    sessEdit->setEnabled(false);
  }

  else if (state == Qt::Checked) {
    arg->ssave = 1;
    fsm->ssave = 1;

    sessBox->setEnabled(true);
    sessEdit->setEnabled(true);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::chooseSessFile(void) {
  QString fileName;

  // If a session file has already been specified, then we preselect it.
  
  if (arg->sfname != 0)
    fileName = QFileDialog::getSaveFileName(this, 
					    tr("Choose Session File"),
					    arg->sfname);
  
  // Otherwise, we preselect the current directory.
  
  else
    fileName = QFileDialog::getSaveFileName(this, 
					    tr("Choose Session File"),
					    QDir::currentPath());

  // If the user chose a file, then we use it to store all the hashes
  // which are currently loaded.
  
  if (!fileName.isNull()) {
    const char *fname = fileName.toLatin1().constData();
    
    if (arg->sfname != 0) free(arg->sfname);
    arg->sfname = strdup(fname);

    sessEdit->setText(fileName);

    // Uncheck and recheck the checkbox in order to trigger a
    // stateChanged signal.

    sessBox->setCheckState(Qt::Unchecked);
    sessBox->setCheckState(Qt::Checked);
  }
}
//---------------------------------------------------------------------------
void OphcrackGUI::showNotFound(bool flag) {
  list_t *hashes = crack->hashes;
  list_nd_t *nd;

  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;

    if (hsh->done <= 0 && hsh->id >= 0)
      hashModel->updateHash(hsh, flag);
  }
}
