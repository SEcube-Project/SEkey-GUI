/**
  ******************************************************************************
  * File Name          : viewwindow.h
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

#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H
#include <QDialog>
#include <QGridLayout>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include "mainwindow.h"
#include "sekeyclient.h"

typedef enum class _loginMode{
    Admin = 1,
    User = 2
} loginMode;

class viewwindow : public QDialog
{
    Q_OBJECT
public:
    viewwindow(MainWindow *parent = 0, sekeyClient* client = nullptr, loginMode mode = loginMode::User);
    ~viewwindow();
private:

    sekeyClient* client;
    loginMode mode;
    QGridLayout* grid;
    QMenuBar* menubar;
    QMenu* secubeMenu, *aboutMenu;
    QAction *quitAction, *logoutAction, *aboutAction, *detailsAction;
    QTreeView* result_view;
    QGroupBox* command_gb;
    QVBoxLayout* command_gb_layout;
    QComboBox* commands_cb;
    QPushButton* run_pb;
    QStandardItemModel* view_model;

    void creatActions();
    void creatMenus();
    void on_run_pb_press();

 private slots:
    void on_run_pb_press_wrapper();
};

#endif // VIEWWINDOW_H
