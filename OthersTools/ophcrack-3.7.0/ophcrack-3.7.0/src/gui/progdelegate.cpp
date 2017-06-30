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
#include <QModelIndex>
#include <QPainter>
#include <math.h>

#include "progdelegate.h"
#include "misc.h"
//---------------------------------------------------------------------------
ProgDelegate::ProgDelegate(QObject *parent) : QItemDelegate(parent) {
}
//---------------------------------------------------------------------------
void ProgDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
			 const QModelIndex &index) const {

  QVariant data = index.data(Qt::DisplayRole);

  if (data.isNull()) {
    static const QColor green = QColor(143, 196, 66);
    static const QColor red   = QColor(169, 52, 52);
    const QColor dark = option.palette.dark().color();
    
    // Store the current status of the painter.
    
    painter->save();
    
    // Compute the coordinates of the progress bar.
    
    QRect rect = option.rect;
    int width  = rect.width();
    int height = rect.height(); 
    
    int dx = MY_MAX(2, (int)ceil(0.02*width));
    int dy = MY_MAX(2, (int)ceil(0.05*height));
    
    rect.adjust(dx, dy, -dx, -dy);
    
    // Display the outline of the progress bar.
    
    painter->setPen(dark);
    painter->drawRect(rect);
    
    rect.adjust(1, 1, 0, 0);
    width = rect.width();
    
    // Display the two progress bars.
    
    QVariant data = index.data(Qt::UserRole);
    
    if (!data.isNull()) {
      QPoint pt = data.toPoint();
      
      int ratio_cmin = pt.x();
      int ratio_cmax = pt.y();
      
      QRect pbar1 = rect;
      int width1 = (ratio_cmin*width) / 1000;
      
      pbar1.setWidth(width1);
      painter->fillRect(pbar1, red);
      
      QRect pbar2 = rect;
      int width2 = (ratio_cmax*width) / 1000;
      
      pbar2.setWidth(width2);
      painter->fillRect(pbar2, green);
    }
    
    // Restore the old status of the painter.
    
    painter->restore();
  }

  else
    QItemDelegate::paint(painter, option, index);
}
