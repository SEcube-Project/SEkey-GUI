/**
  ******************************************************************************
  * File Name          : options_view.cpp
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

#include "options_view.h"
#include "ui_options_view.h"

#include <QPushButton>

options_view::options_view(QWidget *parent, QString title, QStringList* options) :
    QDialog(parent),
    ui(new Ui::options_view)
{
    ui->setupUi(this);

    ui->comboBox->addItems(*options);

    this->setWindowTitle(title);

    connect(ui->ok_pb, &QPushButton::clicked, this, [this](){
       this->option_value = ui->comboBox->currentText();
        this->done(0);
    });
    connect(ui->cancel_pb, &QPushButton::clicked, this, [this](){
        this->done(1);
    });
}

options_view::~options_view()
{
    delete ui;
}
