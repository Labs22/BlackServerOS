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
#include "exportdialog.h"
//---------------------------------------------------------------------------
ExportDialog::ExportDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);
}
//---------------------------------------------------------------------------
void ExportDialog::getFields(int *fields) {

  fields[0] = idCheckBox->isChecked();
  fields[1] = uidCheckBox->isChecked();
  fields[2] = userCheckBox->isChecked();
  fields[3] = lmhashCheckBox->isChecked();
  fields[4] = nthashCheckBox->isChecked();
  fields[5] = lmpwd1CheckBox->isChecked();
  fields[6] = lmpwd2CheckBox->isChecked();
  fields[7] = ntpwdCheckBox->isChecked();
  fields[8] = crackedCheckBox->isChecked();
  fields[9] = timeCheckBox->isChecked();
  fields[10] = lengthCheckBox->isChecked();
  fields[11] = categoryCheckBox->isChecked();
  fields[12] = tableCheckBox->isChecked();

}
//---------------------------------------------------------------------------
char ExportDialog::getSeparator(void) {
  switch(separatorComboBox->currentIndex()) {
  case 0: return ';';
  case 1: return ',';
  case 2: return '\t';
  case 3: return ' ';
  }
  return ';';
}
//---------------------------------------------------------------------------
char ExportDialog::getQuotes(void) {
  switch(quotesComboBox->currentIndex()) {
  case 0: return 0;
  case 1: return '"';
  case 2: return '\'';
  }
  return 0;
}


