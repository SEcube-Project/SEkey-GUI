/**
  ******************************************************************************
  * File Name          : options_view.h
  * Description        :
  ******************************************************************************
  *
  * Copyright © 2016-present Blu5 Group <https://www.blu5group.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 3 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, see <https://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#ifndef OPTIONS_VIEW_H
#define OPTIONS_VIEW_H

#include <QDialog>
#include <QStringList>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>

namespace Ui {
class options_view;
}

class options_view : public QDialog
{
    Q_OBJECT

public:
    explicit options_view(QWidget *parent = nullptr, QString title = "default", QStringList* options = nullptr);
    ~options_view();
    QString option_value;

private:
    Ui::options_view *ui;

};

#endif // OPTIONS_VIEW_H
